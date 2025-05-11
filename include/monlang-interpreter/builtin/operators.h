/*
    standalone header, no .cpp

    individual impl in src/builtin/operators/
*/

#ifndef BUILTIN_OPERATORS_H
#define BUILTIN_OPERATORS_H

#include <monlang-interpreter/types.h>

namespace builtin {
namespace op {

    extern const prim_value_t::Lambda plus;
    extern const prim_value_t::Lambda logical_and;
    extern const prim_value_t::Lambda logical_or;

}} // end of builtin::op::

#endif // BUILTIN_OPERATORS_H
