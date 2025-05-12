/*
    standalone header, no .cpp

    individual declarations in builtin/
    operators' declarations in builtin/operators.h
    primitive constructors' declarations in builtin/prim_ctors.h
*/

#ifndef BUILTIN_H
#define BUILTIN_H

#include <monlang-interpreter/types.h>

/* builtins */
#include <monlang-interpreter/builtin/print.h>
#include <monlang-interpreter/builtin/operators.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <map>

static const std::map<std::string, prim_value_t::Lambda>
BUILTIN_TABLE __attribute__((init_priority(6000))) = {
    {"print", builtin::print},

    /* operators */
    {"+", builtin::op::plus},
    {"&&", builtin::op::logical_and},
    {"||", builtin::op::logical_or},

    /* primitive constructors */
    {"Byte", builtin::prim_ctor::Byte},
    {"Bool", builtin::prim_ctor::Bool},
    {"Int", builtin::prim_ctor::Int},
    {"Float", builtin::prim_ctor::Float},
    {"Str", builtin::prim_ctor::Str},
    // {"List", builtin::prim_ctor::List},
    // {"Map", builtin::prim_ctor::Map},
    // {"Lambda", builtin::prim_ctor::Lambda},
};

#endif // BUILTIN_H
