#include <sstream>
#include "parser.h"
#include "lexer.h"
#include "environment.h"
#include "error.h"

using namespace Lexer;
using namespace Parser;
using namespace Environment;

namespace Driver {
    void start() {
        envs.reserve(max_capacity * 4); // reserve to preserve pointers
        procs.reserve(max_capacity);
        envs.push_back(e0);

        while (true) {
            cout << "> ";
            cs.get();   // eat up first '('
            cout << eval(expr(), &e0) << '\n';    
        }
    }
}

int main(int argc, char* argv[]) {
    switch (argc) {
        case 1:
            break;
        case 2:
            cs.set_input(new istringstream{argv[1]});
            break;
        default:
            Error::error("too many arguments");
            return 1;
    }

    Driver::start();
    return Error::no_of_errors;
}
