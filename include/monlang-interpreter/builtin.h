/*
    standalone header, no .cpp

    individual declarations in builtin/
    operators declarations in builtin/operators.h
*/

#ifndef BUILTIN_H
#define BUILTIN_H

#include <monlang-interpreter/types.h>

/* builtins */
#include <monlang-interpreter/builtin/print.h>
#include <monlang-interpreter/builtin/operators.h>

#include <map>

using builtin_fn = value_t(*)(const std::vector<value_t>&);

static const std::map<std::string, builtin_fn>
BUILTIN_TABLE = {
    {"print", builtin::print},

    /* operators */
    {"+", builtin::op::plus},
};

#endif // BUILTIN_H
