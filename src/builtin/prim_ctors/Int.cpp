#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#define unless(x) if(!(x))

const value_t IntConst::ZERO __attribute__((init_priority(2000))) = new prim_value_t{prim_value_t::Int(0)};
const value_t IntConst::ONE __attribute__((init_priority(2000))) = new prim_value_t{prim_value_t::Int(1)};
const value_t IntConst::TWO __attribute__((init_priority(2000))) = new prim_value_t{prim_value_t::Int(2)};

const prim_value_t::Lambda builtin::prim_ctor::Int __attribute__((init_priority(3000))) = {
    IntConst::ONE,
    [](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
        unless (args.size() == 1) throw InterpretError("Int() takes 1 argument");
        auto argVal = evaluateValue(args.at(0).expr, env);
        return new prim_value_t(Int_(argVal));
    }
};

static prim_value_t::Int to_int(const prim_value_t&);
static prim_value_t::Int to_int(const type_value_t&);
static prim_value_t::Int to_int(const struct_value_t&);
static prim_value_t::Int to_int(const enum_value_t&);

prim_value_t::Int builtin::prim_ctor::Int_(const value_t& val) {
    return std::visit(
        [](auto* val){
            if (val == nullptr){
                throw InterpretError("Int() arg cannot be $nil");
            }
            else {
                return to_int(*val);
            }
        }
        , val
    );
}

static prim_value_t::Int to_int(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Byte byte) -> prim_value_t::Int {return byte;},
        [](prim_value_t::Bool bool_) -> prim_value_t::Int {return bool_;},
        [](prim_value_t::Int int_) -> prim_value_t::Int {return int_;},
        [](prim_value_t::Float float_) -> prim_value_t::Int {return float_;},

        [](const prim_value_t::Str&) -> prim_value_t::Int {throw InterpretError("Int() arg cannot be a Str");},
        [](const prim_value_t::List&) -> prim_value_t::Int {throw InterpretError("Int() arg cannot be a List");},
        [](const prim_value_t::Map&) -> prim_value_t::Int {throw InterpretError("Int() arg cannot be a Map");},
        [](const prim_value_t::Lambda&) -> prim_value_t::Int {throw InterpretError("Int() arg cannot be a Lambda");},
    }, primVal.variant);
}

static prim_value_t::Int to_int(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

static prim_value_t::Int to_int(const struct_value_t&) {
    throw InterpretError("Int() arg cannot be a struct val");
}

static prim_value_t::Int to_int(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}
