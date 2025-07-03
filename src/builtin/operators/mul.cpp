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

static value_t mulByte(Byte firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args);
static value_t mulInt(Int firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args);
static value_t mulFloat(Float firstArgValue, const std::vector<FlattenArg>& args);
static value_t buildStr(Int firstArgValue, const Str& secondArgValue);

const value_t builtin::op::mul __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    new prim_value_t{prim_value_t::Int(2)},
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
            [&otherArgs](Byte byte) -> value_t {
                auto secondArg = otherArgs.at(0);
                auto secondArgValue = evaluateValue(secondArg.expr, secondArg.env);
                unless (std::holds_alternative<prim_value_t*>(secondArgValue)) SHOULD_NOT_HAPPEN(); // TODO: tmp
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
                }
                return mulByte(byte, secondArgPrimValuePtr, otherOtherArgs);
            },
            [&otherArgs](Int int_) -> value_t {
                auto secondArg = otherArgs.at(0);
                auto secondArgValue = evaluateValue(secondArg.expr, secondArg.env);
                unless (std::holds_alternative<prim_value_t*>(secondArgValue)) SHOULD_NOT_HAPPEN(); // TODO: tmp
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
                }
                return mulInt(int_, secondArgPrimValuePtr, otherOtherArgs);
            },
            [&otherArgs](Float float_) -> value_t {return mulFloat(float_, otherArgs);},

            [](Bool) -> value_t {throw InterpretError("*() first arg cannot be Bool");},
            [](Char) -> value_t {throw InterpretError("*() first arg cannot be Char");},
            [](const prim_value_t::Str&) -> value_t {throw InterpretError("*() first arg cannot be Str");},
            [](const prim_value_t::List&) -> value_t {throw InterpretError("*() first arg cannot be List");},
            [](const prim_value_t::Map&) -> value_t {throw InterpretError("*() first arg cannot be Map");},
            [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("*() first arg cannot be Lambda");},
        }, firstArgPrimValuePtr->variant);
    }
}};

static value_t mulByte(Byte firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args) {
    Int res = Int(firstArgValue) * Int(builtin::prim_ctor::Byte_(secondArgValue));

    for (auto arg: args) {
        auto argValue = evaluateValue(arg.expr, arg.env);
        auto intVal = Int(builtin::prim_ctor::Byte_(argValue));
        res *= intVal;
    }

    return new prim_value_t{Byte(res)};
}

static value_t mulInt(Int firstArgValue, prim_value_t* secondArgValue, const std::vector<FlattenArg>& args) {
    Int res = firstArgValue * builtin::prim_ctor::Int_(secondArgValue);

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

static value_t buildStr(Int firstArgValue, const Str& secondArgValue) {
    auto res = Str("");
    for (uint8_t i = 1; i <= firstArgValue; ++i) {
        res += secondArgValue;
    }
    return new prim_value_t{res};
}
