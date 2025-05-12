#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#define unless(x) if(!(x))

const prim_value_t::Lambda builtin::prim_ctor::Byte __attribute__((init_priority(3000))) =
[](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
    unless (args.size() == 1) SHOULD_NOT_HAPPEN();
    auto argVal = evaluateValue(args.at(0).expr, env);
    return new prim_value_t(Byte_(argVal));
};

static prim_value_t::Byte to_byte(const prim_value_t&);
static prim_value_t::Byte to_byte(const type_value_t&);
static prim_value_t::Byte to_byte(const struct_value_t&);
static prim_value_t::Byte to_byte(const enum_value_t&);

prim_value_t::Byte builtin::prim_ctor::Byte_(const value_t& val) {
    return std::visit(
        [](auto* val){
            if (val == nullptr){
                SHOULD_NOT_HAPPEN();
            }
            else {
                return to_byte(*val);
            }
        }
        , val
    );
}

static prim_value_t::Byte to_byte(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Byte byte) -> prim_value_t::Byte {return byte;},
        [](prim_value_t::Int int_) -> prim_value_t::Byte {return int_;},

        [](prim_value_t::Bool) -> prim_value_t::Byte {SHOULD_NOT_HAPPEN();},
        [](prim_value_t::Float) -> prim_value_t::Byte {SHOULD_NOT_HAPPEN();},
        [](const prim_value_t::Str&) -> prim_value_t::Byte {SHOULD_NOT_HAPPEN();},
        [](const prim_value_t::List&) -> prim_value_t::Byte {SHOULD_NOT_HAPPEN();},
        [](const prim_value_t::Map&) -> prim_value_t::Byte {SHOULD_NOT_HAPPEN();},
        [](const prim_value_t::Lambda&) -> prim_value_t::Byte {SHOULD_NOT_HAPPEN();},
    }, primVal.variant);
}

static prim_value_t::Byte to_byte(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

static prim_value_t::Byte to_byte(const struct_value_t&) {
    SHOULD_NOT_HAPPEN();
}

static prim_value_t::Byte to_byte(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}
