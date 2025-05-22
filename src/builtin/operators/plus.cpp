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
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static value_t addByte(Byte firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t addInt(Int firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t addFloat(Float firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t concatStr(const Str& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t concatList(const List& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t concatMap(const Map& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);

const prim_value_t::Lambda builtin::op::plus __attribute__((init_priority(3000))) =
[](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
    unless (args.size() >= 2) throw InterpretError("+() takes 2+ args");

    auto firstArgValue = evaluateValue(args.at(0).expr, env);
    unless (std::holds_alternative<prim_value_t*>(firstArgValue)) SHOULD_NOT_HAPPEN(); // TODO: tmp
    auto firstArgPrimValuePtr = std::get<prim_value_t*>(firstArgValue);
    if (firstArgPrimValuePtr == nullptr) {
        throw InterpretError("+() first arg cannot be $nil");
    }
    auto otherArgs = std::vector<FunctionCall::Argument>{args.begin() + 1, args.end()};

    // dispatch impl based on first argument type
    return std::visit(overload{
        [&otherArgs, env](Byte byte) -> value_t {return addByte(byte, otherArgs, env);},
        [&otherArgs, env](Int int_) -> value_t {return addInt(int_, otherArgs, env);},
        [&otherArgs, env](Float float_) -> value_t {return addFloat(float_, otherArgs, env);},
        [&otherArgs, env](const Str& str) -> value_t {return concatStr(str, otherArgs, env);},
        [&otherArgs, env](const List& list) -> value_t {return concatList(list, otherArgs, env);},
        [&otherArgs, env](const Map& map) -> value_t {return concatMap(map, otherArgs, env);},

        [](Bool) -> value_t {throw InterpretError("+() first arg cannot be Bool");},
        [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("+() first arg cannot be Lambda");},
    }, firstArgPrimValuePtr->variant);
};

static value_t addByte(Byte firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    auto sum = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, env);
        auto intVal = builtin::prim_ctor::Byte_(argValue);
        sum += intVal;
    }

    return new prim_value_t{sum};
}

static value_t addInt(Int firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    auto sum = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, env);
        auto intVal = builtin::prim_ctor::Int_(argValue);
        sum += intVal;
    }

    return new prim_value_t{sum};
}

static value_t addFloat(Float firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    auto sum = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, env);
        auto intVal = builtin::prim_ctor::Float_(argValue);
        sum += intVal;
    }

    return new prim_value_t{sum};
}

static value_t concatStr(const Str& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    auto res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, env);
        auto strVal = builtin::prim_ctor::Str_(argValue);
        res += strVal;
    }

    return new prim_value_t{res};
}

static value_t concatList(const List& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    TODO();
}

static value_t concatMap(const Map& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    TODO();
}

