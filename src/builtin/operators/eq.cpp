#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/loop-utils.h>
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

static value_t comparePrimVal(const prim_value_t::Variant& , const prim_value_t::Variant&);

const value_t builtin::op::eq __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() >= 2) throw InterpretError("==() takes 2+ argument");
        value_t lhsVal;
        LOOP for (auto arg: args) {
            auto argVal = evaluateValue(arg.expr, arg.env);

            /* compare with lhs (arg from last iteration) */
            if (!__first_it) {
                if (lhsVal.index() != argVal.index()) {
                    return BoolConst::FALSE;
                }
                if (is_nil(lhsVal) && is_nil(argVal)) {
                    return BoolConst::TRUE;
                }
                if (is_nil(lhsVal) != is_nil(argVal)) {
                    return BoolConst::FALSE;
                }
                /* at this point, we know both lhsVal and argVal are non-nil AND of the same value_t type */
                return std::visit(overload{
                    [argVal](prim_value_t* lhsPrimValPtr) -> value_t {
                        return comparePrimVal(lhsPrimValPtr->variant, std::get<prim_value_t*>(argVal)->variant);
                    },
                    [](type_value_t*) -> value_t {
                        TODO();
                    },
                    [](struct_value_t*) -> value_t {
                        TODO();
                    },
                    [](enum_value_t*) -> value_t {
                        TODO();
                    },
                }, lhsVal);
            }

            // setup for next iteration
            lhsVal = argVal;
            ENDLOOP
        }
        return BoolConst::TRUE;
    }
}};

static value_t comparePrimVal(const prim_value_t::Variant& primVal_lhs, const prim_value_t::Variant& primVal_rhs) {
    if (primVal_lhs.index() != primVal_rhs.index()) {
        return BoolConst::FALSE;
    }
    return std::visit(overload{
        [&primVal_rhs](Byte byte) -> value_t {
            return new prim_value_t{Bool(byte == std::get<Byte>(primVal_rhs))};
        },
        [&primVal_rhs](Bool bool_) -> value_t {
            return new prim_value_t{Bool(bool_ == std::get<Bool>(primVal_rhs))};
        },
        [&primVal_rhs](Int int_) -> value_t {
            return new prim_value_t{Bool(int_ == std::get<Int>(primVal_rhs))};
        },
        [&primVal_rhs](Float float_) -> value_t {
            return new prim_value_t{Bool(float_ == std::get<Float>(primVal_rhs))};
        },
        [&primVal_rhs](Char char_) -> value_t {
            return new prim_value_t{Bool(char_ == std::get<Char>(primVal_rhs))};
        },
        [&primVal_rhs](const Str& str) -> value_t {
            return new prim_value_t{Bool(str == std::get<Str>(primVal_rhs))};
        },
        [&primVal_rhs](const List& list) -> value_t {
            return new prim_value_t{Bool(list == std::get<List>(primVal_rhs))};
        },
        [&primVal_rhs](const Map& map) -> value_t {
            return new prim_value_t{Bool(map == std::get<Map>(primVal_rhs))};
        },
        [&primVal_rhs](const prim_value_t::Lambda& lambda) -> value_t {
            return new prim_value_t{Bool(get_addr(lambda.stdfunc) == get_addr(std::get<prim_value_t::Lambda>(primVal_rhs).stdfunc))};
        },
    }, primVal_lhs);
}
