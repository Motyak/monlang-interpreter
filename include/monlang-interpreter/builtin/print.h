#ifndef BUILTIN_PRINT_H
#define BUILTIN_PRINT_H

#include <monlang-interpreter/types.h>

#include <iostream>

namespace builtin {
    extern const prim_value_t::Lambda print;

    // for internal use, not part of the builtin table
    value_t print_(const std::vector<value_t>& varargs, std::ostream& = std::cout);
}

#endif // BUILTIN_PRINT_H
