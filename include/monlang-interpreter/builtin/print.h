#ifndef BUILTIN_PRINT_H
#define BUILTIN_PRINT_H

#include <monlang-interpreter/types.h>

namespace builtin {
    value_t print(const std::vector<value_t>& varargs);
}

#endif // BUILTIN_PRINT_H
