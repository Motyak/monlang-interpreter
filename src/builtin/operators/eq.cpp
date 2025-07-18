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

static bool comparePrimValPtr(prim_value_t*, prim_value_t*);

const value_t builtin::op::eq __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("==() takes 2+ argument");
        value_t lhsVal;
        bool res = true;
        bool first_it = true;
        for (auto arg: args) {
            auto argVal = evaluateValue(arg.expr, arg.env);

            /* compare with lhs (arg from last iteration) */
            if (!first_it) {
                if (is_nil(argVal)) {
                    res &= is_nil(lhsVal);
                }
                else {
                    if (lhsVal.index() != argVal.index()) {
                        TODO();
                    }
                    std::visit(overload{
                        [argVal, &res](prim_value_t* lhsPrimValPtr){
                            res &= comparePrimValPtr(lhsPrimValPtr, std::get<prim_value_t*>(argVal));
                        },
                        [](type_value_t*){
                            TODO();
                        },
                        [](struct_value_t*){
                            TODO();
                        },
                        [](enum_value_t*){
                            TODO();
                        },
                        [](char*){
                            SHOULD_NOT_HAPPEN();
                        },
                    }, lhsVal);
                }
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

static bool comparePrimValPtr(prim_value_t* primValPtr_lhs, prim_value_t* primValPtr_rhs) {
    ASSERT (primValPtr_lhs != nullptr);
    return std::visit(overload{
        [primValPtr_rhs](Byte byte) -> bool {
            return byte == builtin::prim_ctor::Byte_(primValPtr_rhs);
        },
        [primValPtr_rhs](Bool bool_) -> bool {
            return bool_ == builtin::prim_ctor::Bool_(primValPtr_rhs);
        },
        [primValPtr_rhs](Int int_) -> bool {
            return int_ == builtin::prim_ctor::Int_(primValPtr_rhs);
        },
        [primValPtr_rhs](Float float_) -> bool {
            return float_ == builtin::prim_ctor::Float_(primValPtr_rhs);
        },
        [primValPtr_rhs](Char char_) -> bool {
            return char_ == builtin::prim_ctor::Char_(primValPtr_rhs);
        },
        [primValPtr_rhs](const Str& str) -> bool {
            return str == builtin::prim_ctor::Str_(primValPtr_rhs);
        },
        [primValPtr_rhs](const List&) -> bool {
            // return list == builtin::prim_ctor::List_(primValPtr_rhs);
            TODO();
        },
        [primValPtr_rhs](const Map&) -> bool {
            // return map == builtin::prim_ctor::Map_(primValPtr_rhs);
            TODO();
        },
        [primValPtr_rhs](const prim_value_t::Lambda&) -> bool {
            // return lambda == builtin::prim_ctor::Lambda_(primValPtr_rhs);
            TODO();
        },
    }, primValPtr_lhs->variant);
}
