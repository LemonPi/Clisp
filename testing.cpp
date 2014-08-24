#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include "boost/variant.hpp"

using namespace std;
using boost::variant;

// -------------------- data structures
enum class Kind : char {
    Cat, Cons, Car, Cdr, List,  // primitive procs
    Define = 'd', Lambda = 'l', Number = '#', Name = 'n', Expr = 'e', Proc = 'p', False = 'f', True = 't', Cond = 'c', Else = ',', End = '.',    // special cases
    Quote = '\'', Lp = '(', Rp = ')', And = '&', Not = '!', Or = '|',
    Mul = '*', Add = '+', Sub = '-', Div = '/', Less = '<', Equal = '=', Greater = '>'  // primitive operators
};
class Cell;
class Env;
int no_of_errors = 0;
double error(const string&);

using List = vector<Cell>;

struct Proc {
    List params;    
    List body;
    Env* env;
};

using Data = variant<string, double, Proc*, List>;  // could make List into List*, but then introduce more management issues and indirection

struct Cell {
    Kind kind;
    Data data;

    // constructors
    Cell() : kind{Kind::End} {} // need default for vector storage
    Cell(Kind k) : kind{k} {}
    Cell(const double n) : kind{Kind::Number}, data{n} {}
    Cell(const string& s) : kind{Kind::Name}, data{s} {}
    Cell(const char* s) : kind{Kind::Name}, data{s} {}
    Cell(Proc* p) : kind{Kind::Proc}, data{p} {}
    Cell(List l) : kind{Kind::Expr}, data{l} {}
    explicit Cell(bool b) : kind{b? Kind::True : Kind::False} {}

    // copy and move constructors
    Cell(const Cell&) = default;
    Cell& operator=(const Cell&) = default;
    Cell(Cell&&) = default;
    Cell& operator=(Cell&&) = default;

    ~Cell() = default;

    // conversion operators
    operator bool() { return kind != Kind::False; }
};

class Env {
private:
    using Env_map = map<string, Cell>;
    Env_map env;
    Env* outer;
public:
    // constructors
    Env() : outer{nullptr} {}
    Env(Env* o) : outer{o} {}
    Env(const List& params, const List& args, Env* o) : outer{o} {
        auto a = args.begin();
        for (auto p = params.begin(); p != params.end(); ++p, ++a)
            env[boost::get<string>(p->data)] = *a++;    
    }

    Env_map& findframe(string n) {
        if (env.find(n) != env.end())
            return env;
        else if (outer != nullptr) 
            return outer->findframe(n);
        error("Unbound variable");
        exit(1);
    }

    Cell& lookup(string n) {
        return findframe(n)[n];
    }

    Cell& operator[](string n) { // access for assignment
        return env[n];
    }

    // copying and moving
    Env(const Env&) = default;
    Env& operator=(const Env&) = default;

    Env(Env&&) = default;
    Env& operator=(Env&&) = default;
    ~Env() = default;
};

constexpr int max_capacity = 10000; // max of 10000 variables and procedures
Env e0;
vector<Env> envs; 
vector<Proc> procs;

// ------------------- stream
class Cell_stream {
public:
    Cell_stream(istream& instream_ref) : owns{false}, ip{&instream_ref} {}
    Cell_stream(istream* instream_pt)  : owns{true}, ip{instream_pt} {}

    Cell get();    // get and return next cell
    const Cell& current() { return ct; } // most recently get cell
    bool eof() { return ip->eof(); }
    void sync() { ip->sync(); }
    void reset() { set_input(cin); }

    void set_input(istream& instream_ref) { close(); ip = &instream_ref; owns = false; }
    void set_input(istream* instream_pt) { close(); ip = instream_pt; owns = true; }

private:
    void close() { if (owns) delete ip; }
    bool owns;
    istream* ip;    // input stream pointer
    Cell ct {Kind::End};   // current token, default value in case of misuse
};

Cell_stream cs {cin};

Cell Cell_stream::get() {
    // get 1 char, decide what kind of cell is incoming,
    // appropriately get more char then return Cell
    char c = 0;

    do {  // skip all whitespace including newline
        if(!ip->get(c)) return ct = {Kind::End};  // no char can be get from ip
    } while (isspace(c));

    switch (c) {
        case '!':
        case '&':
        case '*':
        case '/':
        case '+':
        case '-':
        case '(':
        case ')':
        case '<':
        case '>':
        case '=':
        case '\'':
        case '|':
            return ct = {static_cast<Kind>(c)}; // primitive operators
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.': {  // possibly make . list shorthand
            ip->putback(c);
            double temp;
            *ip >> temp;
            return ct = {temp};
        }
        case 'a':
        case 'c':
        case 'd':
        case 'e':
        case 'l':
        case 'n':
        case 'o':{  // potential primitives
            ip->putback(c);
            string temp;
            *ip >> temp;
            while (temp.back() == ')') {    // greedy reading of string adjacent to )
                temp.pop_back();
                ip->putback(')');
            }
            if (temp == "define") ct.kind = Kind::Define;
            else if (temp == "lambda") ct.kind = Kind::Lambda;
            else if (temp == "cond") ct.kind = Kind::Cond;
            else if (temp == "cons") ct.kind = Kind::Cons;
            else if (temp == "car") ct.kind = Kind::Car;
            else if (temp == "cdr") ct.kind = Kind::Cdr;
            else if (temp == "list") ct.kind = Kind::List;
            else if (temp == "else") ct.kind = Kind::Else;
            else if (temp == "and") ct.kind = Kind::And;
            else if (temp == "or") ct.kind = Kind::Or;
            else if (temp == "not") ct.kind = Kind::Not;
            else if (temp == "cat") ct.kind = Kind::Cat;
            else { ct.kind = Kind::Name; ct.data = temp; }
            return ct;
        }
        default: {    // name
            ip->putback(c);
            string temp;
            *ip >> temp;
            while (temp.back() == ')') {    // greedy reading of string adjacent to )
                temp.pop_back();
                ip->putback(')');
            }
            ct.data = temp;
            ct.kind = Kind::Name;
            return ct;
        }
    }
}

// ------------------- operators
template <typename T, typename Iter>
const T& get(Iter p) {
    return boost::get<T>(p->data);
}

class print_visitor : public boost::static_visitor<> {
    string end {" "};
public:
    print_visitor() {}
    print_visitor(string e) : end{e} {}
    void operator()(const string& str) const {
        cout << str << end;
    }
    void operator()(const double num) const {
        cout << num << end;
    }
    void operator()(const Proc* proc) const {
        cout << "proc" << end;
    }
    void operator()(const List list) const {
        cout << '(';
        auto p = list.begin();
        if(p->kind != Kind::Number && p->kind != Kind::Name && p->kind != Kind::Expr) cout << static_cast<char>(p->kind);    // primitive
        for (;p + 1 != list.end(); ++p) 
            boost::apply_visitor(print_visitor(), p->data);
        boost::apply_visitor(print_visitor(""), p->data);
        
        cout << ')' << end;
    }
};

void print(const Cell& cell) {
    if(cell.kind != Kind::Number && cell.kind != Kind::Name && cell.kind != Kind::Expr) cout << static_cast<char>(cell.kind);    // primitive
    boost::apply_visitor(print_visitor(), cell.data);
}

ostream& operator<<(ostream& os, const Cell& c) {
    print(c);
    return os;
}


// ---------------- parser
namespace Parser {
    Env* bind(const List& params, const List& args, Env* env);
}
List expr();    // parses an expression without evaluating it, returning it as the lstval inside a cell
Cell eval(const List& expr, Env* env);     // delayed evaluation of expression given back by expr()
Cell apply(const Cell& c, const List& args);           // applies a procedure to return a value
List evlist(const List& expr, Env* env);
Cell apply_prim(const Cell& prim, const List& args);

List expr() {   // returns an unevaluated expression from stream
    List res;
    // expr ... (expr) ...) starts with first lp eaten
    while (true) {
        cs.get();
        switch (cs.current().kind) {
            case Kind::Lp: {    // start of another expression
                res.push_back(expr());  // construct with List, kind is expr and data stored in lstval
                // after geting in an ( expression ' ' <-- expecting rp
                if (cs.current().kind != Kind::Rp) return {{error("')' expected")}};
                break;
            }
            case Kind::End:
            case Kind::Rp: return res;  // for initial expr call, all nested expr calls will exit through first case
            default: res.push_back(cs.current()); break;   // anything else just push back as is
        }
    }
}

Cell eval(const List& expr, Env* env) {
    for (auto& cell : expr)
        print(cell);
    cout << endl;
    for (auto p = expr.begin(); p != expr.end(); ++p) {
        switch (p->kind) {
            case Kind::Number: { cout << "number encountered: " << get<double>(p) << endl; return *p; }
            // return next expression unevaluated, (quote expr)
            case Kind::Quote: return *++p;  
            case Kind::Lambda: {    // (lambda (params) (body))
                assert(p + 2 != expr.end()); 
                auto params = get<List>(++p);
                auto body = get<List>(++p);
                procs.push_back({params, body, env});    // introduce onto heap
                return {&procs.back()};
            }
            // introduce cell to environment (define name expr)
            case Kind::Define: {
                auto np = ++p;    // cell to be defined
                if (np->kind == Kind::Name) 
                    return (*env)[get<string>(np)] = eval({++p, expr.end()}, env); 
                else if (np->kind == Kind::Expr) {   // (syntactic sugar for defining functions (define (func args) (body))
                    auto declaration = get<List>(np);
                    string name = get<string>(declaration.begin());
                    auto params = List{declaration.begin() + 1, declaration.end()};
                    auto body = get<List>(++p);
                    procs.push_back({params, body, env});
                    return (*env)[name] = {&procs.back()};
                }
                else return {error("Unfamiliar form to define")};
            }
            // (... (expr) ...) parentheses encloses expression (as parsed by expr())
            case Kind::Expr: { 
                auto res = evlist(get<List>(p), env); 
                if (res.size() == 1) return {res[0]}; // single element
                return {res};
            }
            // (cond ((pred) (expr)) ((pred) (expr)) ...(else expr)) expect list of pred-expr pairs
            case Kind::Cond: {
                while (++p != expr.end()) {
                    const List& clause = get<List>(p);
                    if (clause[0].kind == Kind::Else) {
                        if (p + 1 == expr.end()) return eval({clause[1]}, env);
                        else return {error("Else clause not at end of condition")};
                    }
                    if (eval({clause[0]}, env)) return eval({clause[1]}, env);
                }
            }
            // primitive procedures
            case Kind::Add: case Kind::Sub: case Kind::Mul: case Kind::Div: case Kind::Less: case Kind::Greater: case Kind::Equal: 
            case Kind::Cat: case Kind::Cons: case Kind::Car: case Kind::Cdr: case Kind::List: case Kind::And: case Kind::Or: case Kind::Not:
                             { cout << "calling apply prim, proc: " << static_cast<char>(p->kind);
                                 cout << endl;
                                 auto prim = *p;
                                 auto argstart = ++p;
                                 cout << "primitive access successful\n";
                                 List arg_unevaled = {argstart, expr.end()};
                                 cout << "argument length: " << arg_unevaled.size() << endl;
                                 cout << "------------ unevaledargs -------------\n";
                                 for (auto& cell : arg_unevaled)
                                     print(cell);
                                 cout << endl;
                                 cout << "----------- endunevaledargs -----------\n";
                                 auto args = evlist({argstart, expr.end()}, env);
                                 cout << "------------ args -------------\n";
                                 for (auto& cell : args)
                                     print(cell);
                                 cout << endl;
                                 cout << "----------- endargs -----------\n";
                                 cout << "evlist successful\n";
                                 return apply_prim(prim, args); }
            case Kind::Name: {  // lexer cannot distinguish between varname and procname, have to evaluate against environment
                Cell x = env->lookup(get<string>(p));
                if (x.kind != Kind::Proc) return x;
                return apply(x, evlist({++p, expr.end()}, env));    // user defined proc
            }
            default: return {error("Unmatched cell in eval")};
        }
    }
    return {};
}

List evlist(const List& expr, Env* env) {
    cout << "In evlist with expr size: " << expr.size() << endl;
    for (auto& cell : expr)
        print(cell);
    cout << endl;
    List res;   // instead of returning right away, push back into res then return res
    for (auto p = expr.begin(); p != expr.end(); ++p) {
        switch (p->kind) {
            case Kind::Number: res.push_back(*p); break;
            // return next expression unevaluated, (quote expr)
            case Kind::Quote: res.push_back(*++p); break;  
            case Kind::Lambda: {    // (lambda (params) (body))
                assert(p + 2 != expr.end()); 
                auto params = get<List>(++p);
                auto body = get<List>(++p);
                procs.push_back({params, body, env});    // introduce onto heap
                res.push_back({&procs.back()});
                break;
            }
            // introduce cell to environment (define name expr)
            case Kind::Define: {
                auto np = ++p;    // cell to be defined
                if (np->kind == Kind::Name) {
                    res.push_back((*env)[get<string>(np)] = eval({++p, expr.end()}, env)); 
                    return res;
                }
                else if (np->kind == Kind::Expr) {   // (syntactic sugar for defining functions (define (func args) (body))
                    auto declaration = get<List>(np);
                    string name = get<string>(declaration.begin());
                    auto params = List{declaration.begin() + 1, declaration.end()};
                    auto body = get<List>(++p);
                    procs.push_back({params, body, env});
                    res.push_back((*env)[name] = {&procs.back()});
                    return res;
                }
                else return {error("Unfamiliar form to define")};
            }
            // (... (expr) ...) parentheses encloses expression (as parsed by expr())
            case Kind::Expr: {
                auto r = evlist(get<List>(p), env); 
                if (r.size() == 1) res.push_back({r[0]}); // single element result
                else res.push_back({r});
                break;
            }
            // (cond ((pred) (expr)) ((pred) (expr)) ...) expect list of pred-expr pairs
            case Kind::Cond: {
                while (++p != expr.end()) {
                    const List& clause = get<List>(p);
                    if (clause[0].kind == Kind::Else) {
                        if (p + 1 == expr.end()) { res.push_back(eval({clause[1]}, env)); return res; }
                        else return {error("Else clause not at end of condition")};
                    }
                    if (eval({clause[0]}, env)) { res.push_back(eval({clause[1]}, env)); break; }
                }
                break;
            }
            // primitive procedures
            case Kind::Add: case Kind::Sub: case Kind::Mul: case Kind::Div: case Kind::Less: case Kind::Greater: case Kind::Equal: 
            case Kind::Cat: case Kind::Cons: case Kind::Car: case Kind::Cdr: case Kind::List: case Kind::And: case Kind::Or: case Kind::Not: {
                auto prim = *p;
                auto argstart = ++p;
                                 cout << "primitive access successful\n";
                                 List arg_unevaled = {argstart, expr.end()};
                                 cout << "argument length: " << arg_unevaled.size() << endl;
                                 cout << "------------ unevaledargs (evlist) -------------\n";
                                 for (auto& cell : arg_unevaled)
                                     print(cell);
                                 cout << endl;
                                 cout << "----------- endunevaledargs (evlist) -----------\n";
                auto args = evlist({argstart, expr.end()}, env);
                                 cout << "------------ args (evlist) -------------\n";
                                 for (auto& cell : args)
                                     print(cell);
                                 cout << endl;
                                 cout << "----------- endargs (evlist) -----------\n";
                res.push_back(apply_prim(prim, args));
                return res; // finished reading entire expression
            }
            case Kind::Name: {  // lexer cannot distinguish between varname and procname, have to evaluate against environment
                Cell x = env->lookup(get<string>(p));
                if (x.kind != Kind::Proc) { res.push_back(x); break; }
                else { res.push_back(apply(x, evlist({++p, expr.end()}, env))); return res; }   // user defined proc
            }
            default: error("Unmatched in evlist"); break;
        }
    }
    return res;
}

Cell apply(const Cell& c, const List& args) {  // expect fully evaluated args
    cout << "Inside apply with arg length: " << args.size() << endl;
                                 cout << "------------ args (apply) -------------\n";
                                 for (auto& cell : args)
                                     print(cell);
                                 cout << endl;
                                 cout << "----------- endargs (apply) -----------\n";

    const Proc& proc = *boost::get<Proc*>(c.data);
    Env* newenv = Parser::bind(proc.params, args, proc.env);
    cout << "Arguments bound to new environment\n";
    for (auto& cell : proc.params)
        cout << get<string>(cell.data) << ": " << newenv->lookup(get<string>(cell.data)) << endl;
    return eval(proc.body, newenv);
}

namespace Parser {
    Env* bind(const List& params, const List& args, Env* env) {
        Env newenv {env};
        assert(params.size() == args.size());
        auto q = args.begin();
        for (auto p = params.begin(); p != params.end(); ++p, ++q) {
            cout << "Binding " << get<string>(p) << " to " << *q << endl;
            newenv[get<string>(p)] = *q;
        }

        envs.push_back(newenv);  // store on the heap to allow reference and pointer
        return &envs.back();
    }
}

// primitive procedures
class less_visitor : public boost::static_visitor<bool> {
    // first elements stored, second elements taken as operand
    string str;
    double num;
    List list;
    Proc* proc;
public:
    less_visitor(const string& s) : str{s} {}
    less_visitor(const double d) : num{d} {}
    less_visitor(Proc* const p) : proc(p) {}
    less_visitor(const List& l) : list{l} {}
    bool operator()(const string& s) const { return str < s; }
    bool operator()(const double n) const { return num < n; }
    bool operator()(Proc* const p) const { return (*proc).body < (*p).body; }
    bool operator()(const List& l) const { return list < l; }
};

class equal_visitor : public boost::static_visitor<bool> {
    string str;
    double num;
    List list;
    Proc* proc;
public:
    equal_visitor(const string& s) : str{s} {}
    equal_visitor(const double d) : num{d} {}
    equal_visitor(Proc* const p) : proc(p) {}
    equal_visitor(const List& l) : list{l} {}
    bool operator()(const string& s) const { return str == s; }
    bool operator()(const double n) const { return num == n; }
    bool operator()(Proc* const p) const { return proc == p; }
    bool operator()(const List& l) const { return list == l; }
};

bool operator<(const Cell& a, const Cell& b) {
    if (a.kind == Kind::Number)
        return boost::apply_visitor(less_visitor(boost::get<double>(a.data)), b.data);
    return boost::apply_visitor(less_visitor(boost::get<string>(a.data)), b.data);
}
bool operator==(const Cell& a, const Cell& b) {
     if (a.kind == Kind::Number)
        return boost::apply_visitor(equal_visitor(boost::get<double>(a.data)), b.data);
    return boost::apply_visitor(equal_visitor(boost::get<string>(a.data)), b.data);
}

Cell apply_prim(const Cell& prim, const List& args) {
    cout << "inside apply prim: " << static_cast<char>(prim.kind) << endl;
                                 cout << "------------ args (apply_prim) -------------\n";
                                 for (auto& cell : args)
                                     print(cell);
                                 cout << endl;
                                 cout << "----------- endargs (apply_prim) -----------\n";
    switch (prim.kind) {
        case Kind::Add: {   // more efficient to separate addition and concatenation
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res += get<double>(p);
            return {res};
        }
        case Kind::Cat: {   // (cat 'str 'str ...)
            string res {get<string>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res += get<string>(p);
            return {res};
        }
        case Kind::Sub: {
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res -= get<double>(p);
            return {res};
        }
        case Kind::Mul: {
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res *= get<double>(p);
            return {res};
        }
        case Kind::Div: {
            double res {get<double>(args.begin())};
            for (auto p = args.begin() + 1; p != args.end(); ++p)
                res /= get<double>(p);  // uncheckd divide by 0
            return {res};
        }
        case Kind::Less: {
            if (args[0].kind == Kind::Number)
                return Cell{boost::apply_visitor(less_visitor(get<double>(args.begin())), args[1].data)};
            return Cell{boost::apply_visitor(less_visitor(get<string>(args.begin())), args[1].data)};
        }
        case Kind::Equal: {
            if (args[0].kind == Kind::Number)
                return Cell{boost::apply_visitor(equal_visitor(get<double>(args.begin())), args[1].data)};
            return Cell{boost::apply_visitor(equal_visitor(get<string>(args.begin())), args[1].data)};
        }
        case Kind::Greater: {   // for the sake of efficiency not implemented using !< && !=
            if (args[1].kind == Kind::Number)   // a > b == b < a, just use less
                return Cell{boost::apply_visitor(less_visitor(get<double>(args.begin() + 1)), args[0].data)};
            return Cell{boost::apply_visitor(less_visitor(get<string>(args.begin() + 1)), args[0].data)};
        }
        case Kind::And: {
            for (auto& clause : args)
                if(clause.kind == Kind::False) return clause;
            return Cell{Kind::True};
        }
        case Kind::Or: {
            for (auto& clause : args)
                if(clause.kind == Kind::True) return clause;
            return Cell{Kind::False};
        }
        case Kind::Not: return Cell{args[0].kind == Kind::False? Kind::True : Kind::False};  // only expect 1 argument
        case Kind::List:              // same as cons in this implementation, just that cons conventionally expects only 2 args
        case Kind::Cons: return args; // return List of the args
        case Kind::Car: return boost::get<List>(args[0].data)[0]; // args is a list of one cell which holds a list itself
        case Kind::Cdr: return boost::get<List>(args[0].data)[1];
        default: return error("Mismatch in apply_prim");
    }
}


void start(bool print_res) {
    envs.reserve(max_capacity * 4); // reserve to preserve pointers
    procs.reserve(max_capacity);
    envs.push_back(e0);

    while (true) {
        if (print_res) cout << "> ";
        cs.get();   // eat up first '('
        auto res = eval(expr(), &e0);
        if (print_res)
            cout << res << '\n';    
        if (cs.eof()) { cs.reset(); print_res = true; }
    }
}

// ---------------- error
double error(const string& s) {
    no_of_errors++;
    cerr << "error: " << s << '\n';
    return 1.0;
}

int main(int argc, char* argv[]) {
    bool print_res {false};
    switch (argc) {
        case 1:
            print_res = true;
            break;
        case 2:
            cs.set_input(new ifstream{argv[1]});
            break;
        case 3: {
            string option {argv[2]};
            if (option == "-p" || option == "-print") print_res = true;
            break;
        }
        default:
            error("too many arguments");
            return 1;
    }
    start(print_res);
    /* expr testing
    cs.get();   // eat first '('
    List l = expr();
    print({l});
    cout << endl;
    */

    /* eval testing
    List v {Kind::Define, "x", List{Kind::Lambda, List{"x", "y"}, List{Kind::Add, "x", "y"}}};
    Cell c = eval(v, &e0);
    cout << static_cast<char>(c.kind) << c << endl;

    List v2 {"x", 50, 60};
    cout << "------------- second expr ---------------\n";
    Cell c2 = eval(v2, &e0);
    cout << c2 << endl;
    */


    /* print and Cell testing
    List x {"inner1", "inner2", 6.9};
    List v {"first", "second", "third", "fourth", 5.5, x}; // need brace for char* because 2 conversions required to construct from string
    List z {"this", "list", "has", "only", "strings"};
    cout << v;

    Cell justdouble {0.5};
    Cell justname {"name"};
    
    cout << "sizes: \n";
    cout << "Cell: " << sizeof(Cell) << endl;   // 32 with union, 48 without
    cout << "List: " << sizeof(List) << endl;
    cout << "List*: " << sizeof(List*) << endl;
    cout << "Data: " << sizeof(Data) << endl;
    cout << "Proc: " << sizeof(Proc) << endl;
    cout << "just double: " << sizeof(justdouble) << endl;
    cout << "just name: " << sizeof(justname) << endl;
    */

}
