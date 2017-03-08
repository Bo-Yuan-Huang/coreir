#ifndef PASSES_HPP_
#define PASSES_HPP_

#include "common.hpp"
#include "context.hpp"

bool typecheck(Context* c, Module* m);
bool rungenerators(Context* c, Module* m);
Module* loadModule(Context* c, string filename);


#endif //PASSES_HPP_
