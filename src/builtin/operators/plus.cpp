#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/interpret.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/loop-utils.h>

#define unless(x) if (!(x))

static value_t addByte(prim_value_t::Byte firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t addInt(prim_value_t::Int firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t addFloat(prim_value_t::Float firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t concatStr(const prim_value_t::Str& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t concatList(const prim_value_t::List& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);
static value_t concatMap(const prim_value_t::Map& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env);

const prim_value_t::Lambda builtin::op::plus __attribute__((init_priority(3000))) =
[](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
    // should throw runtime error
    unless (args.size() >= 2) SHOULD_NOT_HAPPEN();

    auto firstArgValue = evaluateValue(args.at(0).expr, env);
    // should throw runtime error
    unless (std::holds_alternative<prim_value_t*>(firstArgValue)) SHOULD_NOT_HAPPEN();
    auto firstArgValue_ = *std::get<prim_value_t*>(firstArgValue);
    auto otherArgs = std::vector<FunctionCall::Argument>{args.begin() + 1, args.end()};

    // dispatch impl based on first argument type
    return std::visit(overload{
        [&otherArgs, env](prim_value_t::Byte byte) -> value_t {return addByte(byte, otherArgs, env);},
        [&otherArgs, env](prim_value_t::Int int_) -> value_t {return addInt(int_, otherArgs, env);},
        [&otherArgs, env](prim_value_t::Float float_) -> value_t {return addFloat(float_, otherArgs, env);},
        [&otherArgs, env](const prim_value_t::Str& str) -> value_t {return concatStr(str, otherArgs, env);},
        [&otherArgs, env](const prim_value_t::List& list) -> value_t {return concatList(list, otherArgs, env);},
        [&otherArgs, env](const prim_value_t::Map& map) -> value_t {return concatMap(map, otherArgs, env);},

        /* should throw runtime error */
        [](prim_value_t::Bool) -> value_t {SHOULD_NOT_HAPPEN();},
        [](const prim_value_t::Lambda&) -> value_t {SHOULD_NOT_HAPPEN();},
    }, firstArgValue_.variant);
};

static value_t addByte(prim_value_t::Byte firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    TODO();
}

static value_t addInt(prim_value_t::Int firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    auto sum = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, env);

        // should throw runtime error
        unless (std::holds_alternative<prim_value_t*>(argValue)) SHOULD_NOT_HAPPEN();
        auto primVal = *std::get<prim_value_t*>(argValue);

        // should throw runtime error
        unless (std::holds_alternative<prim_value_t::Int>(primVal.variant)) SHOULD_NOT_HAPPEN();
        auto intVal = std::get<prim_value_t::Int>(primVal.variant);

        sum += intVal;
    }

    return new prim_value_t{sum};
}

static value_t addFloat(prim_value_t::Float firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    TODO();
}

static value_t concatStr(const prim_value_t::Str& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    TODO();
}

static value_t concatList(const prim_value_t::List& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    TODO();
}

static value_t concatMap(const prim_value_t::Map& firstArgValue, const std::vector<FunctionCall::Argument>& args, Environment* env) {
    TODO();
}

