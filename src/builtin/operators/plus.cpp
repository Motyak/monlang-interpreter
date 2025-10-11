#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/loop-utils.h>
#include <utils/vec-utils.h>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Char = prim_value_t::Char;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static value_t addByte(Byte firstArgValue, const std::vector<FlattenArg>& args);
static value_t addInt(Int firstArgValue, const std::vector<FlattenArg>& args);
static value_t addFloat(Float firstArgValue, const std::vector<FlattenArg>& args);
static value_t addChar(Char firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args);

static value_t concatStr(const Str& firstArgValue, const std::vector<FlattenArg>& args);
static value_t concatStr(const Str& firstArgValue, const Str& secondArgValue, const std::vector<FlattenArg>& args);

static value_t concatList(const List& firstArgValue, const std::vector<FlattenArg>& args);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::op::plus __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("+() takes 2+ args");

        auto firstArg = args.at(0);
        auto firstArgValue = evaluateValue(firstArg.expr, firstArg.env);
        ASSERT (std::holds_alternative<prim_value_t*>(firstArgValue)); // TODO: tmp
        auto firstArgPrimValuePtr = std::get<prim_value_t*>(firstArgValue);
        if (firstArgPrimValuePtr == nullptr) {
            throw InterpretError("+() first arg cannot be $nil");
        }
        auto otherArgs = std::vector<FlattenArg>{args.begin() + 1, args.end()};

        // dispatch impl based on first argument type
        return std::visit(overload{
            [&otherArgs](Char char_) -> value_t {
                auto secondArg = otherArgs.at(0);
                auto secondArgValue = evaluateValue(secondArg.expr, secondArg.env);
                ASSERT (std::holds_alternative<prim_value_t*>(secondArgValue)); // TODO: tmp
                auto secondArgPrimValuePtr = std::get<prim_value_t*>(secondArgValue);
                if (secondArgPrimValuePtr == nullptr) {
                    throw InterpretError("+(<Char>, <..>) second arg cannot be $nil");
                }
                auto otherOtherArgs = std::vector<FlattenArg>{otherArgs.begin() + 1, otherArgs.end()};
                if (std::holds_alternative<Str>(secondArgPrimValuePtr->variant)) {
                    return concatStr(Str(1, char_), std::get<Str>(secondArgPrimValuePtr->variant), otherOtherArgs);
                }
                if (std::holds_alternative<Char>(secondArgPrimValuePtr->variant)) {
                    return concatStr(Str(1, char_), Str(1, std::get<Char>(secondArgPrimValuePtr->variant)), otherOtherArgs);
                }
                ::activeCallStack.push_back(secondArg.expr);
                return addChar(char_, secondArgPrimValuePtr, otherOtherArgs);
            },
            [&otherArgs](Byte byte) -> value_t {return addByte(byte, otherArgs);},
            [&otherArgs](Int int_) -> value_t {return addInt(int_, otherArgs);},
            [&otherArgs](Float float_) -> value_t {return addFloat(float_, otherArgs);},
            [&otherArgs](const Str& str) -> value_t {return concatStr(str, otherArgs);},
            [&otherArgs](const List& list) -> value_t {return concatList(list, otherArgs);},

            [](Bool) -> value_t {throw InterpretError("+() first arg cannot be Bool");},
            [](const Map&) -> value_t {throw InterpretError("+() first arg cannot be Map");},
            [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("+() first arg cannot be Lambda");},
        }, firstArgPrimValuePtr->variant);
    }
}};

static value_t addByte(Byte firstArgValue, const std::vector<FlattenArg>& args) {
    auto sum = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = builtin::prim_ctor::Byte_(argValue);
        safe_pop_back(::activeCallStack); // arg.expr
        sum += intVal;
    }

    return new prim_value_t{sum};
}

static value_t addInt(Int firstArgValue, const std::vector<FlattenArg>& args) {
    auto sum = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = builtin::prim_ctor::Int_(argValue);
        safe_pop_back(::activeCallStack); // arg.expr
        sum += intVal;
    }

    return new prim_value_t{sum};
}

static value_t addFloat(Float firstArgValue, const std::vector<FlattenArg>& args) {
    auto sum = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = builtin::prim_ctor::Float_(argValue);
        safe_pop_back(::activeCallStack); // arg.expr
        sum += intVal;
    }

    return new prim_value_t{sum};
}

static value_t addChar(Char firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args) {
    auto sum = uint8_t(uint8_t(firstArgValue) + builtin::prim_ctor::Byte_(secondArgValue));
    safe_pop_back(::activeCallStack); // from before addChar() call

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = uint8_t(builtin::prim_ctor::Byte_(argValue));
        safe_pop_back(::activeCallStack); // arg.expr
        sum += intVal;
    }

    return new prim_value_t{Char(sum)};
}

static value_t concatStr(const Str& firstArgValue, const std::vector<FlattenArg>& args) {
    auto res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        auto strVal = builtin::prim_ctor::Str_(argValue);
        res += strVal;
    }

    return new prim_value_t{res};
}

static value_t concatStr(const Str& firstArgValue, const Str& secondArgValue, const std::vector<FlattenArg>& args) {
    auto res = firstArgValue + secondArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        auto strVal = builtin::prim_ctor::Str_(argValue);
        res += strVal;
    }

    return new prim_value_t{res};
}

static value_t concatList(const List& firstArgValue, const std::vector<FlattenArg>& args) {
    List res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto currList = builtin::prim_ctor::List_(argValue);
        safe_pop_back(::activeCallStack); // arg.expr
        res.insert(res.end(), currList.begin(), currList.end());
    }

    return new prim_value_t{res};
}
