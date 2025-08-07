#ifndef BUILTIN_PRINT_H
#define BUILTIN_PRINT_H

#include <monlang-interpreter/types.h>

#include <iostream>

namespace builtin {
    extern const value_t print;

    /* for internal use, not part of the builtin table */

    void print_(const std::vector<value_t>& varargs, std::ostream& = std::cout);

    // same as print_ but without the trailing newline
    void putstr_(const std::vector<value_t>& varargs, std::ostream& = std::cout);
}

#endif // BUILTIN_PRINT_H
