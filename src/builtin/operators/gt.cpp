#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/stdfunc-utils.h>

#define unless(x) if (!(x))

using Int = prim_value_t::Int;
using Byte = prim_value_t::Byte;
using Bool = prim_value_t::Bool;
using Float = prim_value_t::Float;
using Char = prim_value_t::Char;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static int compareValue(value_t, value_t);
static int comparePrimValPtr(prim_value_t*, prim_value_t*);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::op::gt __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError(">() takes 2+ argument");
        value_t lhsVal;
        bool res = true;
        bool first_it = true;
        for (auto arg: args) {
            auto argVal = evaluateValue(arg.expr, arg.env);
            if (is_nil(argVal)) {
                throw InterpretError(">() arg is $nil");
            }

            /* compare with lhs (arg from last iteration) */
            if (!first_it) {
                res &= compareValue(lhsVal, argVal) > 0;
            }

            if (res == false) {
                return BoolConst::FALSE;
            }

            // setup for next iteration
            lhsVal = argVal;
            first_it = false;
        }
        return BoolConst::TRUE;
    }
}};

static int compareValue(value_t lhsVal, value_t rhsVal) {
    if (lhsVal.index() != rhsVal.index()) {
        TODO();
    }
    return std::visit(overload{
        [rhsVal](prim_value_t* lhsPrimValPtr) -> int {
            return comparePrimValPtr(lhsPrimValPtr, std::get<prim_value_t*>(rhsVal));
        },
        [](type_value_t*) -> int {
            TODO();
        },
        [](struct_value_t*) -> int {
            TODO();
        },
        [](enum_value_t*) -> int {
            TODO();
        },
        [](char*) -> int {
            SHOULD_NOT_HAPPEN();
        },
    }, lhsVal);
}

static int comparePrimValPtr(prim_value_t* primValPtr_lhs, prim_value_t* primValPtr_rhs) {
    ASSERT (primValPtr_lhs != nullptr);
    return std::visit(overload{
        [primValPtr_rhs](Bool bool_) -> int {
            auto rhsAsBool = builtin::prim_ctor::Bool_(primValPtr_rhs);
            return bool_ < rhsAsBool? -1 : bool_ > rhsAsBool? 1 : 0;
        },
        [primValPtr_rhs](Byte byte) -> int {
            auto rhsAsByte = builtin::prim_ctor::Byte_(primValPtr_rhs);
            return byte < rhsAsByte? -1 : byte > rhsAsByte? 1 : 0;
        },
        [primValPtr_rhs](Int int_) -> int {
            auto rhsAsInt = builtin::prim_ctor::Int_(primValPtr_rhs);
            return int_ < rhsAsInt? -1 : int_ > rhsAsInt? 1 : 0;
        },
        [primValPtr_rhs](Float float_) -> int {
            auto rhsAsFloat = builtin::prim_ctor::Float_(primValPtr_rhs);
            return float_ < rhsAsFloat? -1 : float_ > rhsAsFloat? 1 : 0;
        },
        [primValPtr_rhs](Char char_) -> int {
            auto rhsAsChar = builtin::prim_ctor::Char_(primValPtr_rhs);
            return char_ < rhsAsChar? -1 : char_ > rhsAsChar? 1 : 0;
        },
        [primValPtr_rhs](const Str& str) -> int {
            auto rhsAsStr = builtin::prim_ctor::Str_(primValPtr_rhs);
            return str < rhsAsStr? -1 : str > rhsAsStr? 1 : 0;
        },
        [primValPtr_rhs](const List& list) -> int {
            auto rhsAsList = builtin::prim_ctor::List_(primValPtr_rhs);
            if (list.size() != rhsAsList.size()) {
                return list.size() < rhsAsList.size() ? -1 : 1;
            }
            for (size_t i = 0; i < list.size(); ++i) {
                auto cmp = compareValue(list[i], rhsAsList[i]);
                if (cmp != 0) {
                    return cmp;
                }
            }
            return 0;
        },
        [primValPtr_rhs](const Map&) -> int {
            // return map > builtin::prim_ctor::Map_(primValPtr_rhs);
            TODO();
        },
        [primValPtr_rhs](const prim_value_t::Lambda&) -> int {
            // return lambda > builtin::prim_ctor::Lambda_(primValPtr_rhs);
            TODO();
        },
    }, primValPtr_lhs->variant);
}
