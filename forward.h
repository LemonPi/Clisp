#ifndef clispp_forward
#define clispp_forward
#include <vector>
namespace Lexer {
    struct Cell;
    using List = std::vector<Cell>;
}
namespace Environment {
    class Env;
}
#endif
