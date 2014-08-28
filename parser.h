#ifndef lispcpp_parser
#define lispcpp_parser
#include "lexer.h"
#include "environment.h"
namespace Parser {
    using namespace Lexer;
    using namespace Environment;

    List expr(bool getfirst);    // parses an expression without evaluating it, returning it as the lstval inside a cell
    Cell eval(const List& expr, Env* env);     // delayed evaluation of expression given back by expr()
    Cell apply(const Cell& proc, const List& args);           // applies a procedure to return a value
}
#endif
