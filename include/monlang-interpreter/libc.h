#ifndef LIBC_H
#define LIBC_H

#include <monlang-interpreter/types.h>

#include <utils/assert-utils.h>

#include <iostream>

// TODO: we should have a map that caches the libc_lambdas,
// we should also pre-store the number of fixed parameter for
// each individual function ?
inline value_t create_libc_lambda(void* libc_fn) {
    ASSERT (libc_fn);
    std::cerr << "libc_fn: " << libc_fn << std::endl;

    return new prim_value_t{prim_value_t::Lambda{
        new prim_value_t{prim_value_t::Int(-1)}, // TODO: tmp
        [](const std::vector<FlattenArg>& args) -> value_t {
            
        }
    }};
}

#endif // LIBC_H
