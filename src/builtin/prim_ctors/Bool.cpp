#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/variant-utils.h>
#include <utils/assert-utils.h>

#define unless(x) if(!(x))

const value_t BoolConst::TRUE __attribute__((init_priority(2000))) = new prim_value_t{prim_value_t::Bool(true)};
const value_t BoolConst::FALSE __attribute__((init_priority(2000))) = new prim_value_t{prim_value_t::Bool(false)};

const value_t builtin::prim_ctor::Bool __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    new prim_value_t{prim_value_t::Int(1)},
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("Bool() takes 1 argument");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        return Bool_(argVal)? BoolConst::TRUE : BoolConst::FALSE;
    }
}};

static prim_value_t::Bool to_bool(const prim_value_t&);
static prim_value_t::Bool to_bool(const type_value_t&);
static prim_value_t::Bool to_bool(const struct_value_t&);
static prim_value_t::Bool to_bool(const enum_value_t&);

prim_value_t::Bool builtin::prim_ctor::Bool_(const value_t& val) {
    return std::visit(overload{
        [](prim_value_t* primVal) -> prim_value_t::Bool {
            if (primVal == nullptr) {
                #ifdef TOGGLE_NIL_CAST_TO_BOOL
                return false;
                #endif
                throw InterpretError("Bool() arg cannot be $nil"
                        " (TOGGLE_NIL_CAST_TO_BOOL macro wasn't provided)");
            }
            return to_bool(*primVal);
        },
        [](auto*) -> prim_value_t::Bool {SHOULD_NOT_HAPPEN();}, // TODO: tmp
    }, val);
}

static prim_value_t::Bool to_bool(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Byte byte) -> prim_value_t::Bool {return byte;},
        [](prim_value_t::Bool bool_) -> prim_value_t::Bool {return bool_;},
        [](prim_value_t::Int int_) -> prim_value_t::Bool {return int_;},

        [](prim_value_t::Float) -> prim_value_t::Bool {throw InterpretError("Bool() arg cannot be a Float");},
        [](prim_value_t::Char) -> prim_value_t::Bool {throw InterpretError("Bool() arg cannot be a Char");},
        [](prim_value_t::Str) -> prim_value_t::Bool {throw InterpretError("Bool() arg cannot be a Str");},
        [](prim_value_t::List) -> prim_value_t::Bool {throw InterpretError("Bool() arg cannot be a List");},
        [](prim_value_t::Map) -> prim_value_t::Bool {throw InterpretError("Bool() arg cannot be a Map");},
        [](prim_value_t::Lambda) -> prim_value_t::Bool {throw InterpretError("Bool() arg cannot be a Lambda");},
    }, primVal.variant);
}

static prim_value_t::Bool to_bool(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); //TODO: tmp
}

static prim_value_t::Bool to_bool(const struct_value_t&) {
    throw InterpretError("Bool() arg cannot be a struct val");
}

static prim_value_t::Bool to_bool(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); //TODO: tmp
}
