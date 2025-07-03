#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/loop-utils.h>

#define unless(x) if (!(x))

using Int = prim_value_t::Int;
using Byte = prim_value_t::Byte;
using Bool = prim_value_t::Bool;
using Float = prim_value_t::Float;
using Char = prim_value_t::Char;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static value_t mulByte(Byte firstArgValue, const std::vector<FlattenArg>& args);
static value_t mulInt(Int firstArgValue, const std::vector<FlattenArg>& args);
static value_t mulFloat(Float firstArgValue, const std::vector<FlattenArg>& args);
static value_t buildStr(const Str& firstArgValue, const std::vector<FlattenArg>& args);

const value_t builtin::op::mul __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("*() takes 2+ args");

        auto firstArg = args.at(0);
        auto firstArgValue = evaluateValue(firstArg.expr, firstArg.env);
        unless (std::holds_alternative<prim_value_t*>(firstArgValue)) SHOULD_NOT_HAPPEN(); // TODO: tmp
        auto firstArgPrimValuePtr = std::get<prim_value_t*>(firstArgValue);
        if (firstArgPrimValuePtr == nullptr) {
            throw InterpretError("*() first arg cannot be $nil");
        }
        auto otherArgs = std::vector<FlattenArg>{args.begin() + 1, args.end()};

        // dispatch impl based on first argument type
        return std::visit(overload{
            [&otherArgs](Byte byte_) -> value_t {return mulByte(byte_, otherArgs);},
            [&otherArgs](Int int_) -> value_t {return mulInt(int_, otherArgs);},
            [&otherArgs](Float float_) -> value_t {return mulFloat(float_, otherArgs);},
            [&otherArgs](const prim_value_t::Str& str) -> value_t {return buildStr(str, otherArgs);},
            [&otherArgs](Char char_) -> value_t {return buildStr(Str(1, char_), otherArgs);},

            [](Bool) -> value_t {throw InterpretError("*() first arg cannot be Bool");},
            [](const prim_value_t::List&) -> value_t {throw InterpretError("*() first arg cannot be List");},
            [](const prim_value_t::Map&) -> value_t {throw InterpretError("*() first arg cannot be Map");},
            [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("*() first arg cannot be Lambda");},
        }, firstArgPrimValuePtr->variant);
    }
}};

static value_t mulByte(Byte firstArgValue, const std::vector<FlattenArg>& args) {
    Int res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        auto intVal = builtin::prim_ctor::Byte_(argValue);
        res *= intVal;
    }

    return new prim_value_t{Byte(res)};
}

static value_t mulInt(Int firstArgValue, const std::vector<FlattenArg>& args) {
    Int res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        auto intVal = builtin::prim_ctor::Int_(argValue);
        res *= intVal;
    }

    return new prim_value_t{res};
}

static value_t mulFloat(Float firstArgValue, const std::vector<FlattenArg>& args) {
    auto res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        auto floatVal = builtin::prim_ctor::Float_(argValue);
        res *= floatVal;
    }

    return new prim_value_t{res};
}

static value_t buildStr(const Str& firstArgValue, const std::vector<FlattenArg>& args) {
    auto multiplier = Int(1);
    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        auto intValue = builtin::prim_ctor::Int_(argValue);
        multiplier *= intValue;
    }

    auto res = Str("");
    for (uint8_t i = 1; i <= multiplier; ++i) {
        res += firstArgValue;
    }

    return new prim_value_t{res};
}
