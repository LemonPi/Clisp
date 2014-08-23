#include "parser_impl.h"
#include "environment.h"
#include "error.h"

using namespace std;
using namespace Lexer;
using namespace Environment;
using Error::error;

template <typename T, typename Iter>
const T& get(Iter p) {
    return boost::get<T>(p->data);
}

List Parser::expr() {   // returns an unevaluated expression from stream
    List res;
    // expr ... (expr) ...) starts with first lp eaten
    while (true) {
        cs.get();
        switch (cs.current().kind) {
            case Kind::lp: {    // start of another expression
                res.push_back(expr());  // construct with List, kind is expr and data stored in lstval
                // after geting in an ( expression ' ' <-- expecting rp
                if (cs.current().kind != Kind::rp) return {{error("')' expected")}};
                break;
            }
            case Kind::rp: return res;  // for initial expr call, all nested expr calls will exit through first case
            default: res.push_back(cs.current()); break;   // anything else just push back as is
        }
    }
}

Cell Parser::eval(const List& expr, Env* env) {
    for (auto p = expr.begin(); p != expr.end(); ++p) {
        switch (p->kind) {
            case Kind::number: return *p;
            // return next expression unevaluated, (quote expr)
            case Kind::quote: return *++p;  
            case Kind::lambda: {    // (lambda (params) (body))
                assert(p + 2 != expr.end()); 
                auto params = get<List>(++p);
                auto body = get<List>(++p);
                procs.push_back({params, body, env});    // introduce onto heap
                return {&procs.back()};
            }
            // introduce cell to environment (define name expr)
            case Kind::define: {
                string name = get<string>(++p);
                return (*env)[name] = eval({++p, expr.end()}, env); 
            }
            // (... (expr) ...) parentheses encloses expression (as parsed by expr())
            case Kind::expr: { 
                auto res = evlist(get<List>(p), env); 
                if (res.size() == 1) return {res[0]}; // single element
                return {res};
            }
            // (cond ((pred) (expr)) ((pred) (expr)) ...(else expr)) expect list of pred-expr pairs
            case Kind::cond: {
                while (++p != expr.end()) {
                    const List& clause = get<List>(p);
                    if (clause[0].kind == Kind::els) {
                        if (p + 1 == expr.end()) return eval({clause[1]}, env);
                        else return {error("Else clause not at end of condition")};
                    }
                    if (eval({clause[0]}, env)) return eval({clause[1]}, env);
                }
            }
            // primitive procedures
            case Kind::add: case Kind::sub: case Kind::mul: case Kind::div: case Kind::less: case Kind::greater: case Kind::equal: 
            case Kind::cat: case Kind::cons: case Kind::car: case Kind::cdr: case Kind::list: {
                auto prim = *p;
                return apply_prim(prim, evlist({++p, expr.end()}, env));
            }
            case Kind::name: {  // lexer cannot distinguish between varname and procname, have to evaluate against environment
                Cell x = env->lookup(get<string>(p));
                if (x.kind != Kind::proc) return x;
                return apply(x, evlist({++p, expr.end()}, env));    // user defined proc
            }
            default: return {error("Unmatched cell in eval")};
        }
    }
    return {};
}

List Parser::evlist(const List& expr, Env* env) {
    List res;   // instead of returning right away, push back into res then return res
    for (auto p = expr.begin(); p != expr.end(); ++p) {
        switch (p->kind) {
            case Kind::number: res.push_back(*p); break;
            // return next expression unevaluated, (quote expr)
            case Kind::quote: res.push_back(*++p); break;  
            case Kind::lambda: {    // (lambda (params) (body))
                assert(p + 2 != expr.end()); 
                auto params = get<List>(++p);
                auto body = get<List>(++p);
                procs.push_back({params, body, env});    // introduce onto heap
                res.push_back({&procs.back()});
                break;
            }
            // introduce cell to environment (define name expr)
            case Kind::define: {
                string name = get<string>(++p);
                res.push_back((*env)[name] = eval({++p, expr.end()}, env)); 
                return res;
            }
            // (... (expr) ...) parentheses encloses expression (as parsed by expr())
            case Kind::expr: {
                auto r = evlist(get<List>(p), env); 
                if (r.size() == 1) res.push_back({r[0]}); // single element result
                else res.push_back({r});
                break;
            }
            // (cond ((pred) (expr)) ((pred) (expr)) ...) expect list of pred-expr pairs
            case Kind::cond: {
                while (++p != expr.end()) {
                    const List& clause = get<List>(p);
                    if (clause[0].kind == Kind::els) {
                        if (p + 1 == expr.end()) { res.push_back(eval({clause[1]}, env)); return res; }
                        else return {error("Else clause not at end of condition")};
                    }
                    if (eval({clause[0]}, env)) { res.push_back(eval({clause[1]}, env)); break; }
                }
                break;
            }
            // primitive procedures
            case Kind::add: case Kind::sub: case Kind::mul: case Kind::div: case Kind::less: case Kind::greater: case Kind::equal: 
            case Kind::cons: case Kind::car: case Kind::cdr: case Kind::list: {
                auto prim = *p;
                res.push_back(apply_prim(prim, evlist({++p, expr.end()}, env)));
                return res; // finished reading entire expression
            }
            case Kind::name: {  // lexer cannot distinguish between varname and procname, have to evaluate against environment
                Cell x = env->lookup(get<string>(p));
                if (x.kind != Kind::proc) { res.push_back(x); break; }
                else { res.push_back(apply(x, evlist({++p, expr.end()}, env))); return res; }   // user defined proc
            }
            default: error("Unmatched in evlist"); break;
        }
    }
    return res;
}

Cell Parser::apply(const Cell& c, const List& args) {  // expect fully evaluated args
    const Proc& proc = *boost::get<Proc*>(c.data);
    Env* newenv = Parser::bind(proc.params, args, proc.env);
    return eval(proc.body, newenv);
}

Env* Parser::bind(const List& params, const List& args, Env* env) {
    Env newenv {env};
    assert(params.size() == args.size());
    auto q = args.begin();
    for (auto p = params.begin(); p != params.end(); ++p, ++q)
        newenv[get<string>(p)] = *q;

    envs.push_back(newenv);  // store on the heap to allow reference and pointer
    return &envs.back();
}

// primitive procedures
Cell Parser::apply_prim(const Cell& prim, const List& args) {
    switch (prim.kind) {
        case Kind::add: {   // more efficient to separate addition and concatenation
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res += get<double>(p);
            return {res};
        }
        case Kind::cat: {   // (cat 'str 'str ...)
            string res {get<string>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res += get<string>(p);
            return {res};
        }
        case Kind::sub: {
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res -= get<double>(p);
            return {res};
        }
        case Kind::mul: {
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res *= get<double>(p);
            return {res};
        }
        case Kind::div: {
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res /= get<double>(p);  // uncheckd divide by 0
            return {res};
        }
        case Kind::less: {
            if (args[0].kind == Kind::number)
                return Cell{boost::apply_visitor(less_visitor(get<double>(args.begin())), args[1].data)};
            return Cell{boost::apply_visitor(less_visitor(get<string>(args.begin())), args[1].data)};
        }
        case Kind::equal: {
            if (args[0].kind == Kind::number)
                return Cell{boost::apply_visitor(equal_visitor(get<double>(args.begin())), args[1].data)};
            return Cell{boost::apply_visitor(equal_visitor(get<string>(args.begin())), args[1].data)};
        }
        case Kind::greater: {   // for the sake of efficiency not implemented using !< && !=
            if (args[1].kind == Kind::number)   // a > b == b < a, just use less
                return Cell{boost::apply_visitor(less_visitor(get<double>(args.begin() + 1)), args[0].data)};
            return Cell{boost::apply_visitor(less_visitor(get<string>(args.begin() + 1)), args[0].data)};
        }
        case Kind::list:              // same as cons in this implementation, just that cons conventionally expects only 2 args
        case Kind::cons: return args; // return List of the args
        case Kind::car: return boost::get<List>(args[0].data)[0]; // args is a list of one cell which holds a list itself
        case Kind::cdr: return boost::get<List>(args[0].data)[1];
        default: return error("Mismatch in apply_prim");
    }
}
