/*
    standalone header, no .cpp

    individual impl in src/builtin/prim_ctors/
*/

#ifndef BUILTIN_PRIM_CTORS_H
#define BUILTIN_PRIM_CTORS_H

#include <monlang-interpreter/types.h>

namespace BoolConst {
    extern const value_t TRUE;
    extern const value_t FALSE;
}

namespace IntConst {
    extern const value_t ZERO;
    extern const value_t ONE;
    extern const value_t TWO;
}

namespace builtin {
namespace prim_ctor {

    extern const value_t Byte;
    extern const value_t Bool;
    extern const value_t Int;
    extern const value_t Float;
    extern const value_t Char;
    extern const value_t Str;
    extern const value_t List;
    extern const value_t Map;
    extern const value_t Lambda;

    /* for internal use, not part of the builtin table */
    prim_value_t::Byte Byte_(const value_t&);
    prim_value_t::Bool Bool_(const value_t&);
    prim_value_t::Int Int_(const value_t&);
    prim_value_t::Float Float_(const value_t&);
    prim_value_t::Char Char_(const value_t&);
    prim_value_t::Str Str_(const value_t&);

}} // end of builtin::prim_ctor::

#endif // BUILTIN_PRIM_CTORS_H
