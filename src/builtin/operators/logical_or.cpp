#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;

const value_t builtin::op::logical_or __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("||() takes 2+ argument");
        for (auto arg: args) {
            auto argVal = evaluateValue(arg.expr, arg.env);
            unless (std::holds_alternative<prim_value_t*>(argVal)) SHOULD_NOT_HAPPEN(); //TODO: tmp
            auto argPrimValPtr = std::get<prim_value_t*>(argVal);
            // Bool_ is responsible for $nil handling
            auto argBool = builtin::prim_ctor::Bool_(argPrimValPtr);
            if (argBool) {
                return BoolConst::TRUE;
            }
        }
        return BoolConst::FALSE;
    }
}};
