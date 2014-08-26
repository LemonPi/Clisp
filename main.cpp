#include <fstream>
#include "parser.h"
#include "lexer.h"
#include "environment.h"
#include "error.h"

using namespace Lexer;
using namespace Parser;
using namespace Environment;

namespace Driver {
    void start(bool print_res) {
        envs.reserve(max_capacity * 4); // reserve to preserve pointers
        procs.reserve(max_capacity);
        envs.push_back(e0);

        while (true) {
            if (print_res) cout << "> ";
            cs.get();   // eat up first '('
            try {
                auto res = eval(expr(), &e0);
                if (print_res)
                    cout << res << '\n';    
            }
            catch (exception& e) {
                cout << "Bad expression: " << e.what() << endl;    // continue loop
            }
            if (cs.eof()) { cs.reset(); print_res = true; }
        }
    }
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
            cs.set_input(new ifstream{argv[1]});
            string option {argv[2]};
            if (option == "-p" || option == "-print") print_res = true;
            break;
        }
        default:
            throw runtime_error("too many arguments");
            return 1;
    }
    Driver::start(print_res);

    return 0;
}
