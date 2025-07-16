#include <monlang-interpreter/builtin/getline.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>

#include <iostream>

const value_t builtin::getline __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::ZERO,
    [](const std::vector<FlattenArg>&) -> value_t {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty() && std::cin.eof()) {
            return nil_value_t();
        }
        return new prim_value_t{prim_value_t::Str(line)};
    }
}};
