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
#include <monlang-interpreter/builtin/getline.h>
#include <monlang-interpreter/builtin/putstr.h>
#include <monlang-interpreter/builtin/slurpfile.h>
#include <monlang-interpreter/builtin/die.h>
#include <monlang-interpreter/builtin/exit.h>
#include <monlang-interpreter/builtin/sleep.h>
#include <monlang-interpreter/builtin/len.h>
#include <monlang-interpreter/builtin/bitwise_not.h>

#include <monlang-interpreter/builtin/operators.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <map>

static const std::map<std::string, value_t>
BUILTIN_TABLE __attribute__((init_priority(6000))) = {
    {"print", builtin::print},
    {"getline", builtin::getline},
    {"putstr", builtin::putstr},
    {"slurpfile", builtin::slurpfile},
    {"die", builtin::die},
    {"exit", builtin::exit},
    {"sleep", builtin::sleep},
    {"len", builtin::len},
    {"~", builtin::bitwise_not},

    /* operators */
    {"&&", builtin::op::logical_and},
    {"||", builtin::op::logical_or},
    {"==", builtin::op::eq},
    {">", builtin::op::gt},
    {"+", builtin::op::plus},
    {"*", builtin::op::mul},
    {"/", builtin::op::div},
    {"//", builtin::op::intdiv},
    {"%", builtin::op::mod},
    {"**", builtin::op::pow},
    {"<<", builtin::op::leftshift},
    {">>", builtin::op::rightshift},
    {"&", builtin::op::bitwise_and},
    {"|", builtin::op::bitwise_or},
    {"^", builtin::op::bitwise_xor},

    /* primitive constructors */
    {"Bool", builtin::prim_ctor::Bool},
    {"Byte", builtin::prim_ctor::Byte},
    {"Int", builtin::prim_ctor::Int},
    {"Float", builtin::prim_ctor::Float},
    {"Char", builtin::prim_ctor::Char},
    {"Str", builtin::prim_ctor::Str},
    {"List", builtin::prim_ctor::List},
    {"Map", builtin::prim_ctor::Map},
    {"Lambda", builtin::prim_ctor::Lambda},
};

#endif // BUILTIN_H
