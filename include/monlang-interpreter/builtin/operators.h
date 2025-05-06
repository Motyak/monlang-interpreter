/*
    standalone header, no .cpp

    individual impl in src/builtin/operators/
*/

#ifndef BUILTIN_OPERATORS_H
#define BUILTIN_OPERATORS_H

#include <monlang-interpreter/types.h>

namespace builtin {
namespace op {

    value_t plus(const std::vector<value_t>& varargs);

}} // end of builtin::op::

#endif // BUILTIN_OPERATORS_H
