#include <monlang-interpreter/builtin/operators.h>

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

static value_t bitwise_and_Byte(Byte firstArgValue, const std::vector<FlattenArg>& args);
static value_t bitwise_and_Int(Int firstArgValue, const std::vector<FlattenArg>& args);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::op::bitwise_and __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("&() takes 2+ args");

        auto firstArg = args.at(0);
        auto firstArgValue = evaluateValue(firstArg.expr, firstArg.env);
        ASSERT (std::holds_alternative<prim_value_t*>(firstArgValue)); // TODO: tmp
        auto firstArgPrimValuePtr = std::get<prim_value_t*>(firstArgValue);
        if (firstArgPrimValuePtr == nullptr) {
            throw InterpretError("&() first arg cannot be $nil");
        }
        auto otherArgs = std::vector<FlattenArg>{args.begin() + 1, args.end()};

        // dispatch impl based on first argument type
        return std::visit(overload{
            [&otherArgs](Byte byte) -> value_t {return bitwise_and_Byte(byte, otherArgs);},
            [&otherArgs](Int int_) -> value_t {return bitwise_and_Int(int_, otherArgs);},

            [](Bool) -> value_t {throw InterpretError("&() first arg cannot be Bool");},
            [](Float) -> value_t {throw InterpretError("&() first arg cannot be Float");},
            [](Char) -> value_t {throw InterpretError("&() first arg cannot be Char");},
            [](const Str&) -> value_t {throw InterpretError("&() first arg cannot be Str");},
            [](const List&) -> value_t {throw InterpretError("&() first arg cannot be List");},
            [](const Map&) -> value_t {throw InterpretError("&() first arg cannot be Map");},
            [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("&() first arg cannot be Lambda");},

        }, firstArgPrimValuePtr->variant);
    }
}};

static value_t bitwise_and_Byte(Byte firstArgValue, const std::vector<FlattenArg>& args) {
    auto res = Int(firstArgValue);

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = builtin::prim_ctor::Int_(argValue);
        ::activeCallStack.pop_back(); // arg.expr
        res &= intVal;
    }

    return new prim_value_t{Byte(res)};
}

static value_t bitwise_and_Int(Int firstArgValue, const std::vector<FlattenArg>& args) {
    auto res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = builtin::prim_ctor::Int_(argValue);
        ::activeCallStack.pop_back(); // arg.expr
        res &= intVal;
    }

    return new prim_value_t{res};
}
