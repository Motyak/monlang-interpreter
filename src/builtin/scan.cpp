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
        ::activeCallStack.push_back(arg.expr); // no need to pop
        defer {safe_pop_back(::activeCallStack);};
        ASSERT (std::holds_alternative<prim_value_t*>(argVal)); // TODO: tmp
        auto argPrimValPtr = std::get<prim_value_t*>(argVal);
        if (argPrimValPtr == nullptr) {
            throw InterpretError("scan() arg cannot be $nil");
        }
        unless (std::holds_alternative<prim_value_t::Int>(argPrimValPtr->variant)) {
            throw InterpretError("scan() arg must be an Int");
        }
        auto argInt = std::get<prim_value_t::Int>(argPrimValPtr->variant);
        if (argInt < 0) {
            throw InterpretError("scan() arg can't be negative");
        }

        auto str = std::string(argInt, '\0');
        std::cin.read(str.data(), argInt);
        str.resize(std::cin.gcount());

        return new prim_value_t{(prim_value_t::Str)str};
    }
}};
