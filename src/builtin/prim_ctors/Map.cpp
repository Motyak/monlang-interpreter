#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/vec-utils.h>

#define unless(x) if(!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::prim_ctor::Map __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ZERO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        prim_value_t::Map res;
        for (auto arg: args) {
            auto argVal = evaluateValue(arg.expr, arg.env);
            ASSERT (std::holds_alternative<prim_value_t*>(argVal)); // TODO: tmp
            auto argPrimValPtr = std::get<prim_value_t*>(argVal);
            ::activeCallStack.push_back(arg.expr);
            // auto argAsList = List_(argPrimValPtr); // <== this doesn't make sense after all
            unless (std::holds_alternative<prim_value_t::List>(argPrimValPtr->variant)) {
                throw InterpretError("Map() argument isn't a list");
            }
            auto argAsList = std::get<prim_value_t::List>(argPrimValPtr->variant);
            unless (argAsList.size() == 2) {
                throw InterpretError("Map() list arguments must contain 2 elements");
            }
            safe_pop_back(::activeCallStack); // arg.expr
            res[argAsList.at(0)] = argAsList.at(1);
        }
        return new prim_value_t{res};
    }
}};

// FROM NOW ON, WE DEFINE FOR `Map_` /////////////////

static prim_value_t::Map to_map(const prim_value_t&);
static prim_value_t::Map to_map(const type_value_t&);
static prim_value_t::Map to_map(const struct_value_t&);
static prim_value_t::Map to_map(const enum_value_t&);

prim_value_t::Map builtin::prim_ctor::Map_(const value_t& val) {
    return std::visit(overload{
        [](auto* val) -> prim_value_t::Map {
            if (val == nullptr){
                throw InterpretError("$nil is not a Map");
            }
            else {
                return to_map(*val);
            }
        },
        [](char*) -> prim_value_t::Map {SHOULD_NOT_HAPPEN();},
    }, val);
}

static prim_value_t::Map to_map(const prim_value_t& primVal) {
    return std::visit(overload{
        [](const prim_value_t::List& list) -> prim_value_t::Map {
            auto res = prim_value_t::Map();
            for (auto elem: list) {
                ASSERT (std::holds_alternative<prim_value_t*>(elem));
                auto elemPrimValPtr = std::get<prim_value_t*>(elem);
                unless (std::holds_alternative<prim_value_t::List>(elemPrimValPtr->variant)) {
                    throw InterpretError("this List is not a Map");
                }
                auto elemAsList = std::get<prim_value_t::List>(elemPrimValPtr->variant);
                unless (elemAsList.size() == 2) {
                    throw InterpretError("this List is not a Map");
                }
                res[elemAsList.at(0)] = elemAsList.at(1);
            }
            return res;
        },
        [](const prim_value_t::Map& map) -> prim_value_t::Map {return map;},

        [](prim_value_t::Bool) -> prim_value_t::Map {throw InterpretError("Bool is not a Map");},
        [](prim_value_t::Byte) -> prim_value_t::Map {throw InterpretError("Byte is not a Map");},
        [](prim_value_t::Int) -> prim_value_t::Map {throw InterpretError("Int is not a Map");},
        [](prim_value_t::Float) -> prim_value_t::Map {throw InterpretError("Float is not a Map");},
        [](const prim_value_t::Str&) -> prim_value_t::Map {throw InterpretError("Str is not a Map");},
        [](const prim_value_t::Lambda&) -> prim_value_t::Map {throw InterpretError("Lambda is not a Map");},
    }, primVal.variant);
}

static prim_value_t::Map to_map(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

static prim_value_t::Map to_map(const struct_value_t&) {
    throw InterpretError("a struct val is not a Map"); // TODO: maybe this will change later on ?
}

static prim_value_t::Map to_map(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}
