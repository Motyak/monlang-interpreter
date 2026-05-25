#include <monlang-interpreter/builtin/scan.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <utils/assert-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>

#define unless(x) if(!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::scan __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("scan() takes 1 arg");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        auto argInt = builtin::prim_ctor::Int_(argVal);
        if (argInt < 0) {
            ::activeCallStack.push_back(arg.expr);
            throw InterpretError("scan() arg can't be negative");
        }

        auto str = std::string(argInt, '\0');
        std::cin.read(str.data(), argInt);
        str.resize(std::cin.gcount());

        return new prim_value_t{(prim_value_t::Str)str};
    }
}};
