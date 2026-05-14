#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/builtin.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/loop-utils.h>
#include <utils/vec-utils.h>

#define unless(x) if (!(x))

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static value_t addByte(Byte firstArgValue, const std::vector<FlattenArg>& args);
static value_t addInt(Int firstArgValue, const std::vector<FlattenArg>& args);
static value_t addFloat(Float firstArgValue, const std::vector<FlattenArg>& args);

static value_t concatStr(const Str& firstArgValue, const std::vector<FlattenArg>& args);
static value_t concatList(const List& firstArgValue, const std::vector<FlattenArg>& args);

// special case for when lhs is a type_value_t based on List/Map
// ..AND its ctor seems to be a tagging ctor (no required arg)
static value_t concatListFromTypeVal(
    const std::string& typeTag,
    const FlattenArg& firstArg,
    const List& firstArgValue,
    const std::vector<FlattenArg>& args
);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::op::plus __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("+() takes 2+ args");

        auto firstArg = args.at(0);
        auto firstArgValue = evaluateValue(firstArg.expr, firstArg.env);
        std::optional<std::string> typeTag;
        if (std::holds_alternative<type_value_t*>(firstArgValue)) {
            auto typeVal = std::get<type_value_t*>(firstArgValue);
            typeTag = typeVal->typeTag;
            firstArgValue = rec_unwrap_typeval(firstArgValue);
        }
        if (std::holds_alternative<struct_value_t*>(firstArgValue)) {
            throw InterpretError("+() first arg cannot be a struct");
        }
        ASSERT (std::holds_alternative<prim_value_t*>(firstArgValue));
        auto firstArgPrimValuePtr = std::get<prim_value_t*>(firstArgValue);
        if (firstArgPrimValuePtr == nullptr) {
            throw InterpretError("+() first arg cannot be $nil");
        }
        auto otherArgs = std::vector<FlattenArg>{args.begin() + 1, args.end()};

        // dispatch impl based on first argument type
        return std::visit(overload{
            [&otherArgs](Byte byte) -> value_t {return addByte(byte, otherArgs);},
            [&otherArgs](Int int_) -> value_t {return addInt(int_, otherArgs);},
            [&otherArgs](Float float_) -> value_t {return addFloat(float_, otherArgs);},
            [&otherArgs](const Str& str) -> value_t {return concatStr(str, otherArgs);},
            [&firstArg, &otherArgs, &typeTag](const List& list) -> value_t {
                return typeTag? concatListFromTypeVal(*typeTag, firstArg, list, otherArgs)
                        : concatList(list, otherArgs);
            },

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

static value_t concatStr(const Str& firstArgValue, const std::vector<FlattenArg>& args) {
    auto res = firstArgValue;

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

static value_t concatListFromTypeVal(
    const std::string& typeTag,
    const FlattenArg& firstArg,
    const List& firstArgValue,
    const std::vector<FlattenArg>& args
)
{
    value_t val = concatList(firstArgValue, args);

    // HUGE HACK INCOMING

    bool returnTypeVal = false;
    /* breakable block */ for (int z = 1; z <= 1; z++)
    {
        unless (std::holds_alternative<FunctionCall*>(firstArg.expr)) break;
        auto fncall = *std::get<FunctionCall*>(firstArg.expr);
        unless (std::holds_alternative<Symbol*>(fncall.function)) break;
        auto symbol = std::get<Symbol*>(fncall.function);
        prim_value_t::Lambda lambda;
        if (firstArg.env->contains(symbol->name)) {
            auto symVal = firstArg.env->at(symbol->name);
            value_t val;
            if (std::holds_alternative<Environment::Variable>(symVal)) {
                val = *std::get<Environment::Variable>(symVal);
            }
            else if (std::holds_alternative<Environment::LabelToLvalue /*or PassByRef*/>(symVal)) {
                val = std::get<Environment::LabelToLvalue>(symVal).value();
            }
            else if (std::holds_alternative<Environment::PassByDelay>(symVal)) {
                auto passByDelay = *std::get<Environment::PassByDelay>(symVal);
                if (std::holds_alternative<thunk_with_memoization_t<value_t>*>(passByDelay)) {
                    auto thunk = std::get<thunk_with_memoization_t<value_t>*>(passByDelay);
                    val = (*thunk)();
                }
                else if (std::holds_alternative<value_t*>(passByDelay)) {
                    val = *std::get<value_t*>(passByDelay);
                }
                else SHOULD_NOT_HAPPEN();
            }
            else break;
            val = rec_unwrap_typeval(val);
            ASSERT (std::holds_alternative<prim_value_t*>(val));
            auto prim_val = *std::get<prim_value_t*>(val);
            ASSERT (std::holds_alternative<prim_value_t::Lambda>(prim_val.variant));
            lambda = std::get<prim_value_t::Lambda>(prim_val.variant);
        }
        else if (BUILTIN_TABLE.contains(symbol->name)) {
            auto val = BUILTIN_TABLE.at(symbol->name);
            ASSERT (std::holds_alternative<prim_value_t*>(val));
            auto prim_val = *std::get<prim_value_t*>(val);
            ASSERT (std::holds_alternative<prim_value_t::Lambda>(prim_val.variant));
            lambda = std::get<prim_value_t::Lambda>(prim_val.variant);
        }
        else break;

        returnTypeVal = lambda.requiredArgs == IntConst::ZERO;
    }

    return returnTypeVal? new type_value_t{typeTag, val} : val;
}
