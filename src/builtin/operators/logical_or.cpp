#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/interpret.h>

#include <utils/assert-utils.h>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;

const prim_value_t::Lambda builtin::op::logical_or __attribute__((init_priority(3000))) =
[](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
    // should throw runtime error
    unless (args.size() >= 2) SHOULD_NOT_HAPPEN();
    for (auto arg: args) {
        auto argVal = evaluateValue(arg.expr, env);
        // should throw runtime error
        unless (std::holds_alternative<prim_value_t*>(argVal)) SHOULD_NOT_HAPPEN();
        auto argPrimValPtr = std::get<prim_value_t*>(argVal);
        if (argPrimValPtr == nullptr) {
            continue;
        }
        // should throw runtime error
        unless (std::holds_alternative<Bool>(argPrimValPtr->variant)) SHOULD_NOT_HAPPEN();
        auto argBool = std::get<Bool>(argPrimValPtr->variant);
        if (argBool) {
            return new prim_value_t(Bool(true));
        }
    }
    return new prim_value_t(Bool(false));
};
