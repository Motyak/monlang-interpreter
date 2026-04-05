#ifndef BUILTIN_TYPE_H
#define BUILTIN_TYPE_H

#include <monlang-interpreter/types.h>

namespace builtin {
    extern const value_t typefn;

    /* for internal use, not part of the builtin table */

    prim_value_t::Str typefn_(const value_t&);
}

#endif // BUILTIN_TYPE_H
