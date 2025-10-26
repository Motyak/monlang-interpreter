#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/vec-utils.h>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::op::logical_or __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    own(IntConst::TWO()),
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("||() takes 2+ argument");
        for (auto arg: args) {
            auto argVal = evaluateValue(arg.expr, arg.env);
            ASSERT (std::holds_alternative<prim_value_t*>(argVal)); //TODO: tmp
            auto argPrimValPtr = std::get<prim_value_t*>(argVal);
            // Bool_ is responsible for $nil handling
            ::activeCallStack.push_back(arg.expr);
            auto argBool = builtin::prim_ctor::Bool_(argPrimValPtr);
            safe_pop_back(::activeCallStack); // arg.expr
            if (argBool) {
                return BoolConst::TRUE();
            }
        }
        return BoolConst::FALSE();
    }
}};
