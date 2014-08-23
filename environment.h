#ifndef clispp_environment
#define clispp_environment
#include <memory>
#include <map>
#include "forward.h"
#include "lexer.h"
#include "error.h"

namespace Environment {
    using namespace std;
    using Lexer::Cell;
    using Lexer::Proc;
    using Lexer::List;

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
            Error::error("Unbound variable");
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

    extern Env e0;
    constexpr int max_capacity = 10000; // max of 10000 variables and procedures
    extern vector<Env> envs; 
    extern vector<Proc> procs;
}
#endif
