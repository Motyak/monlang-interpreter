#include <monlang-interpreter/builtin.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/loop-utils.h>

#include <cmath>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Char = prim_value_t::Char;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static value_t bitwise_not_Byte(Byte argValue);
static value_t bitwise_not_Int(Int argValue);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::bitwise_not __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("~() takes 1 arg");

        auto arg = args.at(0);
        auto argValue = evaluateValue(arg.expr, arg.env);
        ASSERT (std::holds_alternative<prim_value_t*>(argValue)); // TODO: tmp
        auto argPrimValuePtr = std::get<prim_value_t*>(argValue);
        if (argPrimValuePtr == nullptr) {
            throw InterpretError("~() arg cannot be $nil");
        }
        // dispatch impl based on first argument type
        return std::visit(overload{
            [](Byte byte) -> value_t {return bitwise_not_Byte(byte);},
            [](Int int_) -> value_t {return bitwise_not_Int(int_);},

            [](Bool) -> value_t {throw InterpretError("~() arg cannot be Bool");},
            [](Float) -> value_t {throw InterpretError("~() arg cannot be Float");},
            [](Char) -> value_t {throw InterpretError("~() arg cannot be Char");},
            [](const Str&) -> value_t {throw InterpretError("~() arg cannot be Str");},
            [](const List&) -> value_t {throw InterpretError("~() arg cannot be List");},
            [](const Map&) -> value_t {throw InterpretError("~() arg cannot be Map");},
            [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("~() arg cannot be Lambda");},

        }, argPrimValuePtr->variant);
    }
}};

static value_t bitwise_not_Byte(Byte argValue) {
    return new prim_value_t{Byte(~argValue)};
}

static value_t bitwise_not_Int(Int argValue) {
    return new prim_value_t{Int(~argValue)};
}
