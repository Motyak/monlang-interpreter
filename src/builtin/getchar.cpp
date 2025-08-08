#include <monlang-interpreter/builtin/getchar.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>

#include <cstdio>

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::getchar __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ZERO,
    [](const std::vector<FlattenArg>&) -> value_t {
        char c = ::getchar();
        if (c == EOF && feof(stdin)) {
            return nil_value_t();
        }
        return new prim_value_t{prim_value_t::Char(c)};
    }
}};
