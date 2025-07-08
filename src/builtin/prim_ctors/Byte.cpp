#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#define unless(x) if(!(x))

const value_t builtin::prim_ctor::Byte __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("Byte() takes 1 argument");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        return new prim_value_t(Byte_(argVal));
    }
}};

static prim_value_t::Byte to_byte(const prim_value_t&);
static prim_value_t::Byte to_byte(const type_value_t&);
static prim_value_t::Byte to_byte(const struct_value_t&);
static prim_value_t::Byte to_byte(const enum_value_t&);

prim_value_t::Byte builtin::prim_ctor::Byte_(const value_t& val) {
    return std::visit(overload{
        [](auto* val) -> prim_value_t::Byte {
            if (val == nullptr){
                throw InterpretError("Byte() arg cannot be $nil");
            }
            else {
                return to_byte(*val);
            }
        },
        [](char*) -> prim_value_t::Byte {SHOULD_NOT_HAPPEN();},
    }, val);
}

static prim_value_t::Byte to_byte(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Byte byte) -> prim_value_t::Byte {return byte;},
        [](prim_value_t::Char char_) -> prim_value_t::Byte {return char_;},
        [](prim_value_t::Int int_) -> prim_value_t::Byte {return int_;},

        [](prim_value_t::Bool) -> prim_value_t::Byte {throw InterpretError("Byte() arg cannot be a Bool");},
        [](prim_value_t::Float) -> prim_value_t::Byte {throw InterpretError("Byte() arg cannot be a Float");},
        [](const prim_value_t::Str&) -> prim_value_t::Byte {throw InterpretError("Byte() arg cannot be a Str");},
        [](const prim_value_t::List&) -> prim_value_t::Byte {throw InterpretError("Byte() arg cannot be a List");},
        [](const prim_value_t::Map&) -> prim_value_t::Byte {throw InterpretError("Byte() arg cannot be a Map");},
        [](const prim_value_t::Lambda&) -> prim_value_t::Byte {throw InterpretError("Byte() arg cannot be a Lambda");},
    }, primVal.variant);
}

static prim_value_t::Byte to_byte(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

static prim_value_t::Byte to_byte(const struct_value_t&) {
    throw InterpretError("Byte() arg cannot be a struct val");
}

static prim_value_t::Byte to_byte(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}
