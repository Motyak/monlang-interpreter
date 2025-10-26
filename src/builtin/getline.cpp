#include <monlang-interpreter/builtin/getline.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>

#include <iostream>

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::getline __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    own(IntConst::ZERO()),
    [](const std::vector<FlattenArg>&) -> value_t {
        prim_value_t::Str line;
        std::getline(std::cin, line);
        if (line.empty() && std::cin.eof()) {
            return nil_value_t();
        }
        return new prim_value_t{line};
    }
}};
