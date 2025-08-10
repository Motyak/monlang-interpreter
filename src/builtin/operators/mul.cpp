#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/loop-utils.h>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Char = prim_value_t::Char;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static value_t mulByte(Byte firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args);
static value_t mulInt(Int firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args);
static value_t mulFloat(Float firstArgValue, const std::vector<FlattenArg>& args);

static value_t buildStr(const Str& firstArgValue, const std::vector<FlattenArg>& args);
static value_t buildStr(Int firstArgValue, const Str& secondArgValue);
static value_t buildList(const List& firstArgValue, const std::vector<FlattenArg>& args);
static value_t buildList(Int firstArgValue, const List& secondArgValue);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::op::mul __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("*() takes 2+ args");

        auto firstArg = args.at(0);
        auto firstArgValue = evaluateValue(firstArg.expr, firstArg.env);
        ASSERT (std::holds_alternative<prim_value_t*>(firstArgValue)); // TODO: tmp
        auto firstArgPrimValuePtr = std::get<prim_value_t*>(firstArgValue);
        if (firstArgPrimValuePtr == nullptr) {
            throw InterpretError("*() first arg cannot be $nil");
        }
        auto otherArgs = std::vector<FlattenArg>{args.begin() + 1, args.end()};

        // dispatch impl based on first argument type
        return std::visit(overload{
            [&otherArgs](Byte byte) -> value_t {
                auto secondArg = otherArgs.at(0);
                auto secondArgValue = evaluateValue(secondArg.expr, secondArg.env);
                ASSERT (std::holds_alternative<prim_value_t*>(secondArgValue)); // TODO: tmp
                auto secondArgPrimValuePtr = std::get<prim_value_t*>(secondArgValue);
                if (secondArgPrimValuePtr == nullptr) {
                    throw InterpretError("*(<Byte>, <..>) second arg cannot be $nil");
                }
                auto otherOtherArgs = std::vector<FlattenArg>{otherArgs.begin() + 1, otherArgs.end()};
                if (otherOtherArgs.empty()) {
                    if (std::holds_alternative<Str>(secondArgPrimValuePtr->variant)) {
                        return buildStr(byte, std::get<Str>(secondArgPrimValuePtr->variant));
                    }
                    if (std::holds_alternative<Char>(secondArgPrimValuePtr->variant)) {
                        return buildStr(byte, Str(1, std::get<Char>(secondArgPrimValuePtr->variant)));
                    }
                    if (std::holds_alternative<List>(secondArgPrimValuePtr->variant)) {
                        return buildList(byte, std::get<List>(secondArgPrimValuePtr->variant));
                    }
                }
                ::activeCallStack.push_back(secondArg.expr);
                return mulByte(byte, secondArgPrimValuePtr, otherOtherArgs);
            },

            [&otherArgs](Int int_) -> value_t {
                auto secondArg = otherArgs.at(0);
                auto secondArgValue = evaluateValue(secondArg.expr, secondArg.env);
                ASSERT (std::holds_alternative<prim_value_t*>(secondArgValue)); // TODO: tmp
                auto secondArgPrimValuePtr = std::get<prim_value_t*>(secondArgValue);
                if (secondArgPrimValuePtr == nullptr) {
                    throw InterpretError("*(<Int>, <..>) second arg cannot be $nil");
                }
                auto otherOtherArgs = std::vector<FlattenArg>{otherArgs.begin() + 1, otherArgs.end()};
                if (otherOtherArgs.empty()) {
                    if (std::holds_alternative<Str>(secondArgPrimValuePtr->variant)) {
                        return buildStr(int_, std::get<Str>(secondArgPrimValuePtr->variant));
                    }
                    if (std::holds_alternative<Char>(secondArgPrimValuePtr->variant)) {
                        return buildStr(int_, Str(1, std::get<Char>(secondArgPrimValuePtr->variant)));
                    }
                    if (std::holds_alternative<List>(secondArgPrimValuePtr->variant)) {
                        return buildList(int_, std::get<List>(secondArgPrimValuePtr->variant));
                    }
                }
                ::activeCallStack.push_back(secondArg.expr);
                return mulInt(int_, secondArgPrimValuePtr, otherOtherArgs);
            },

            [&otherArgs](Float float_) -> value_t {return mulFloat(float_, otherArgs);},
            [&otherArgs](Char char_) -> value_t {return buildStr(Str(1, char_), otherArgs);},
            [&otherArgs](const Str& str) -> value_t {return buildStr(str, otherArgs);},
            [&otherArgs](const List& list) -> value_t {return buildList(list, otherArgs);},

            [](Bool) -> value_t {throw InterpretError("*() first arg cannot be Bool");},
            [](const Map&) -> value_t {throw InterpretError("*() first arg cannot be Map");},
            [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("*() first arg cannot be Lambda");},

        }, firstArgPrimValuePtr->variant);
    }
}};

static value_t mulByte(Byte firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args) {
    Int res = Int(firstArgValue) * Int(builtin::prim_ctor::Byte_(secondArgValue));
    ::activeCallStack.pop_back(); // from before mulByte() call

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = Int(builtin::prim_ctor::Byte_(argValue));
        ::activeCallStack.pop_back(); // arg.expr
        res *= intVal;
    }

    return new prim_value_t{Byte(res)};
}

static value_t mulInt(Int firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args) {
    Int res = firstArgValue * builtin::prim_ctor::Int_(secondArgValue);
    ::activeCallStack.pop_back(); // from before mulInt() call

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intVal = builtin::prim_ctor::Int_(argValue);
        ::activeCallStack.pop_back(); // arg.expr
        res *= intVal;
    }

    return new prim_value_t{res};
}

static value_t mulFloat(Float firstArgValue, const std::vector<FlattenArg>& args) {
    auto res = firstArgValue;

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto floatVal = builtin::prim_ctor::Float_(argValue);
        ::activeCallStack.pop_back(); // arg.expr
        res *= floatVal;
    }

    return new prim_value_t{res};
}

static value_t buildStr(const Str& firstArgValue, const std::vector<FlattenArg>& args) {
    auto multiplier = Int(1);
    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intValue = builtin::prim_ctor::Int_(argValue);
        ::activeCallStack.pop_back(); // arg.expr
        multiplier *= intValue;
    }
    return buildStr(multiplier, firstArgValue);
}

static value_t buildStr(Int firstArgValue, const Str& secondArgValue) {
    auto res = Str("");
    for (Int i = 1; i <= firstArgValue; ++i) {
        res += secondArgValue;
    }
    return new prim_value_t{res};
}

static value_t buildList(const List& firstArgValue, const std::vector<FlattenArg>& args) {
    auto multiplier = Int(1);
    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        auto intValue = builtin::prim_ctor::Int_(argValue);
        ::activeCallStack.pop_back(); // arg.expr
        multiplier *= intValue;
    }
    return buildList(multiplier, firstArgValue);
}

static value_t buildList(Int firstArgValue, const List& secondArgValue) {
    auto res = List();
    res.reserve(secondArgValue.size() * firstArgValue);
    for (Int i = 1; i <= firstArgValue; ++i) {
        res.insert(res.end(), secondArgValue.begin(), secondArgValue.end());
    }
    return new prim_value_t{res};
}
