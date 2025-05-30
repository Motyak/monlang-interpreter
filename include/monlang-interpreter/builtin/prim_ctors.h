/*
    standalone header, no .cpp

    individual impl in src/builtin/prim_ctors/
*/

#ifndef BUILTIN_PRIM_CTORS_H
#define BUILTIN_PRIM_CTORS_H

#include <monlang-interpreter/types.h>

namespace builtin {
namespace prim_ctor {

    extern const prim_value_t::Lambda Byte;
    extern const prim_value_t::Lambda Bool;
    extern const prim_value_t::Lambda Int;
    extern const prim_value_t::Lambda Float;
    extern const prim_value_t::Lambda Str;
    extern const prim_value_t::Lambda List;
    extern const prim_value_t::Lambda Map;
    extern const prim_value_t::Lambda Lambda;

    /* for internal use, not part of the builtin table */
    prim_value_t::Byte Byte_(const value_t&);
    prim_value_t::Bool Bool_(const value_t&);
    prim_value_t::Int Int_(const value_t&);
    prim_value_t::Float Float_(const value_t&);
    prim_value_t::Str Str_(const value_t&);

}} // end of builtin::prim_ctor::

#endif // BUILTIN_PRIM_CTORS_H
