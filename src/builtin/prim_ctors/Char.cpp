#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#define unless(x) if(!(x))

const value_t builtin::prim_ctor::Char __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("Char() takes 1 argument");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        return new prim_value_t(Char_(argVal));
    }
}};

static prim_value_t::Char to_char(const prim_value_t&);
static prim_value_t::Char to_char(const type_value_t&);
static prim_value_t::Char to_char(const struct_value_t&);
static prim_value_t::Char to_char(const enum_value_t&);

prim_value_t::Char builtin::prim_ctor::Char_(const value_t& val) {
    return std::visit(overload{
        [](auto* val) -> prim_value_t::Char {
            if (val == nullptr){
                throw InterpretError("Char() arg cannot be $nil");
            }
            else {
                return to_char(*val);
            }
        },
        [](char*) -> prim_value_t::Char {SHOULD_NOT_HAPPEN();},
    }, val);
}

static prim_value_t::Char to_char(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Char char_) -> prim_value_t::Char {return char_;},
        [](prim_value_t::Byte byte) -> prim_value_t::Char {return byte;},
        [](prim_value_t::Int int_) -> prim_value_t::Char {return uint8_t(int_);},
        [](const prim_value_t::Str& str) -> prim_value_t::Char {
            if (str.empty()) {
                throw InterpretError("Char() arg cannot be an empty Str");
            }
            if (str.size() > 1) {
                throw InterpretError("Char() arg cannot be a 2+ character Str");
            }
            return str[0];
        },

        [](prim_value_t::Bool) -> prim_value_t::Char {throw InterpretError("Char() arg cannot be a Bool");},
        [](prim_value_t::Float) -> prim_value_t::Char {throw InterpretError("Char() arg cannot be a Float");},
        [](const prim_value_t::List&) -> prim_value_t::Char {throw InterpretError("Char() arg cannot be a List");},
        [](const prim_value_t::Map&) -> prim_value_t::Char {throw InterpretError("Char() arg cannot be a Map");},
        [](const prim_value_t::Lambda&) -> prim_value_t::Char {throw InterpretError("Char() arg cannot be a Lambda");},
    }, primVal.variant);
}

static prim_value_t::Char to_char(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

static prim_value_t::Char to_char(const struct_value_t&) {
    throw InterpretError("Char() arg cannot be a struct val");
}

static prim_value_t::Char to_char(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}
