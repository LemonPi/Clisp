#include "environment.h"

Environment::Env Environment::e0;
std::vector<Environment::Env> Environment::envs {}; 
std::vector<Lexer::Proc> Environment::procs {};
