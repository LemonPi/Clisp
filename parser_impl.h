#ifndef bc_parser_impl
#define bc_parser_impl
#include "parser.h"

namespace Parser {  // implementation interface
    List evlist(const List& expr, Env* env);
    Env* bind(const List& params, const List& args, Env* env);
    Cell apply_prim(const Cell& prim, const List& args);
}
#endif
