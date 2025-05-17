#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;

const prim_value_t::Lambda builtin::op::logical_and __attribute__((init_priority(3000))) =
[](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
    unless (args.size() >= 2) throw InterpretError("&&() takes 2+ argument");
    for (auto arg: args) {
        auto argVal = evaluateValue(arg.expr, env);
        // should throw runtime error
        unless (std::holds_alternative<prim_value_t*>(argVal)) SHOULD_NOT_HAPPEN(); //TODO: tmp
        auto argPrimValPtr = std::get<prim_value_t*>(argVal);
        auto argBool = builtin::prim_ctor::Bool_(argPrimValPtr);
        if (!argBool) {
            return new prim_value_t(Bool(false));
        }
    }
    return new prim_value_t(Bool(true));
};
