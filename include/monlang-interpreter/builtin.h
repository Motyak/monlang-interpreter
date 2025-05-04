#ifndef BUILTIN_H
#define BUILTIN_H

#include <monlang-interpreter/types.h>

#include <utils/loop-utils.h>

#include <vector>
#include <iostream>

namespace builtin
{

void print(const std::vector<value_t>& varargs);

} // end of builtin namespace

#endif // BUILTIN_H
