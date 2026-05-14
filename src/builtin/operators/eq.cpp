/*
    Precisely reporting type coercion errors..
    ..while doing recursive compare with two Lists/Maps..
    ..is tough.
    So in this case we simply report the rhs List argument..
    .., we don't go any further.
*/

#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>

#define unless(x) if (!(x))

using Int = prim_value_t::Int;
using Byte = prim_value_t::Byte;
using Bool = prim_value_t::Bool;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static bool compareValue(const value_t&, const value_t&, bool fromMain = false);
static bool comparePrimValPtr(prim_value_t*, const value_t&, bool fromMain = false);

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::op::eq __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("==() takes 2+ argument");
        value_t lhsVal;
        bool res = true;
        bool first_it = true;
        for (auto arg: args) {
            auto argVal = evaluateValue(arg.expr, arg.env);

            // // NOTE: at the moment we allow == with $nil, to check for $nil
            // if (is_nil(argVal)) {
            //     throw InterpretError("==() arg cannot be $nil");
            // }

            /* compare with lhs (arg from last iteration) */
            if (!first_it) {
                if (!is_enum_nil(argVal)) {
                    ::activeCallStack.push_back(arg.expr);
                }
                res &= is_enum_nil(argVal)? is_enum_nil(lhsVal) : compareValue(lhsVal, argVal, /*fromMain*/true);
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

static bool compareValue(const value_t& lhsVal, const value_t& rhsVal, bool fromMain) {
    return std::visit(overload{
        [rhsVal, fromMain](prim_value_t* lhsPrimValPtr) -> bool {
            return comparePrimValPtr(lhsPrimValPtr, rhsVal, fromMain);
        },
        [rhsVal, fromMain](type_value_t* lhs) -> bool {
            auto unwrapped_lhs = rec_unwrap_typeval(lhs->underlyingVal);
            return compareValue(unwrapped_lhs, rhsVal, fromMain);
        },
        [rhsVal](struct_value_t* lhs) -> bool {
            unless (std::holds_alternative<struct_value_t*>(rhsVal)) {
                throw InterpretError("==() rhs isn't a struct");
            }
            auto* rhs_struct = std::get<struct_value_t*>(rhsVal);
            unless (lhs->type == rhs_struct->type) {
                throw InterpretError("==() rhs is a different struct");
            }
            ASSERT (lhs->fields.size() == rhs_struct->fields.size());
            for (size_t i = 0; i < lhs->fields.size(); ++i) {
                if (!compareValue(lhs->fields.at(i).val, rhs_struct->fields.at(i).val)) {
                    return false;
                }
            }
            return true;
        },
        [rhsVal, fromMain](enum_value_t* lhs) -> bool {
            auto unwrapped_lhs = rec_unwrap_typeval(lhs->enumerate);
            return compareValue(unwrapped_lhs, rhsVal, fromMain);
        },
        [](char*) -> bool {
            SHOULD_NOT_HAPPEN();
        },
        [](FieldLvalue*) -> bool {
            SHOULD_NOT_HAPPEN();
        },
    }, lhsVal);
}

static bool comparePrimValPtr(prim_value_t* primValPtr_lhs, const value_t& rhs, bool fromMain) {
    if (primValPtr_lhs == nullptr) {
        if (fromMain) safe_pop_back(activeCallStack); // from before compareValue() call in builtin::op::eq()
        return false; // because we already checked that rhs isn't $nil
    }
    ASSERT (primValPtr_lhs != nullptr);
    return std::visit(overload{
        [rhs, fromMain](Bool bool_) -> bool {
            defer {if (fromMain) safe_pop_back(activeCallStack);}; // from before compareValue() call in builtin::op::eq()
            return bool_ == builtin::prim_ctor::Bool_(rhs);
        },
        [rhs, fromMain](Byte byte) -> bool {
            defer {if (fromMain) safe_pop_back(activeCallStack);}; // from before compareValue() call in builtin::op::eq()
            return byte == builtin::prim_ctor::Byte_(rhs);
        },
        [rhs, fromMain](Int int_) -> bool {
            defer {if (fromMain) safe_pop_back(activeCallStack);}; // from before compareValue() call in builtin::op::eq()
            return int_ == builtin::prim_ctor::Int_(rhs);
        },
        [rhs, fromMain](Float float_) -> bool {
            defer {if (fromMain) safe_pop_back(activeCallStack);}; // from before compareValue() call in builtin::op::eq()
            return float_ == builtin::prim_ctor::Float_(rhs);
        },
        [rhs, fromMain](const Str& str) -> bool {
            defer {if (fromMain) safe_pop_back(activeCallStack);}; // from before compareValue() call in builtin::op::eq()
            return str == builtin::prim_ctor::Str_(rhs);
        },
        [rhs, fromMain](const List& list) -> bool {
            defer {if (fromMain) safe_pop_back(activeCallStack);}; // from before compareValue() call in builtin::op::eq()
            auto rhsAsList = builtin::prim_ctor::List_(rhs);
            if (list.size() != rhsAsList.size()) {
                return false;
            }
            for (size_t i = 0; i < list.size(); ++i) {
                if (!compareValue(list[i], rhsAsList[i])) {
                    return false;
                }
            }
            return true;
        },
        [rhs, fromMain](const Map& map) -> bool {
            defer {if (fromMain) safe_pop_back(activeCallStack);}; // from before compareValue() call in builtin::op::eq()
            auto rhsAsMap = builtin::prim_ctor::Map_(rhs);
            if (map.size() != rhsAsMap.size()) {
                return false;
            }
            auto lhsIt = map.begin();
            auto rhsIt = rhsAsMap.begin();
            for (; lhsIt != map.end(); lhsIt++, rhsIt++) {
                if (!compareValue(lhsIt->first, rhsIt->first)) {
                    return false;
                }
                if (!compareValue(lhsIt->second, rhsIt->second)) {
                    return false;
                }
            }
            return true;
        },
        [rhs, fromMain](const prim_value_t::Lambda& lambda) -> bool {
            auto rhsAsLambda = builtin::prim_ctor::Lambda_(rhs);
            if (fromMain) safe_pop_back(activeCallStack); // from before compareValue() call in builtin::op::eq()
            return lambda.id == rhsAsLambda.id;
        },
    }, primValPtr_lhs->variant);
}
