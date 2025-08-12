/*
    standalone header, no .cpp

    individual impl in src/builtin/operators/
*/

#ifndef BUILTIN_OPERATORS_H
#define BUILTIN_OPERATORS_H

#include <monlang-interpreter/types.h>

namespace builtin {
namespace op {

    extern const value_t plus;
    extern const value_t mul;
    extern const value_t div;
    extern const value_t intdiv;
    extern const value_t mod;
    extern const value_t pow;
    extern const value_t leftshift;
    extern const value_t rightshift;
    extern const value_t bitwise_and;
    extern const value_t bitwise_or;
    extern const value_t bitwise_xor;
    extern const value_t logical_and;
    extern const value_t logical_or;
    extern const value_t eq;
    extern const value_t gt;

}} // end of builtin::op::

#endif // BUILTIN_OPERATORS_H
