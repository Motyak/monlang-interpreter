#include <monlang-interpreter/builtin/len.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#define unless(x) if (!(x))

static value_t strLength(const prim_value_t::Str&);
static value_t listLength(const prim_value_t::List&);
static value_t mapLength(const prim_value_t::Map&);
static value_t requiredParams(const prim_value_t::Lambda&);

const prim_value_t::Lambda builtin::len __attribute__((init_priority(3000))) = {
    new prim_value_t{prim_value_t::Int(1)},
    [](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
        unless (args.size() == 1) throw InterpretError("len() takes 1 arg");
        auto argVal = evaluateValue(args.at(0).expr, env);
        unless (std::holds_alternative<prim_value_t*>(argVal)) SHOULD_NOT_HAPPEN(); // TODO: tmp
        auto argPrimValPtr = std::get<prim_value_t*>(argVal);
        if (argPrimValPtr == nullptr) {
            throw InterpretError("len() arg cannot be $nil");
        }

        // dispatch impl based on argument type
        return std::visit(overload{
            [](const prim_value_t::Str& str) -> value_t {return strLength(str);},
            [](const prim_value_t::List& list) -> value_t {return listLength(list);},
            [](const prim_value_t::Map& map) -> value_t {return mapLength(map);},
            [](const prim_value_t::Lambda& lambda) -> value_t {return requiredParams(lambda);},

            [](prim_value_t::Byte) -> value_t {throw InterpretError("len() arg cannot be Byte");},
            [](prim_value_t::Int) -> value_t {throw InterpretError("len() arg cannot be Int");},
            [](prim_value_t::Float) -> value_t {throw InterpretError("len() arg cannot be Float");},
            [](prim_value_t::Bool) -> value_t {throw InterpretError("len() arg cannot be Bool");},

        }, argPrimValPtr->variant);
    }
};

static value_t strLength(const prim_value_t::Str&) {
    TODO();
}

static value_t listLength(const prim_value_t::List&) {
    TODO();
}

static value_t mapLength(const prim_value_t::Map&) {
    TODO();
}

static value_t requiredParams(const prim_value_t::Lambda& lambda) {
    return lambda.requiredParams;
}
