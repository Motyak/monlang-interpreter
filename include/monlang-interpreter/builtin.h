/*
    standalone header, no .cpp

    individual declarations in builtin/
    operators' declarations in builtin/operators.h
*/

#ifndef BUILTIN_H
#define BUILTIN_H

#include <monlang-interpreter/types.h>

/* builtins */
#include <monlang-interpreter/builtin/print.h>
#include <monlang-interpreter/builtin/operators.h>

#include <map>

static const std::map<std::string, prim_value_t::Lambda>
BUILTIN_TABLE __attribute__((init_priority(6000))) = {
    {"print", builtin::print},

    /* operators */
    {"+", builtin::op::plus},
    {"&&", builtin::op::logical_and},
    {"||", builtin::op::logical_or},
};

#endif // BUILTIN_H
