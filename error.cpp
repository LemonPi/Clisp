#include <iostream>
#include "error.h"

int Error::no_of_errors = 0;
double Error::error(const string& s) {
    no_of_errors++;
    cerr << "error: " << s << '\n';
    return 1.0;
}
