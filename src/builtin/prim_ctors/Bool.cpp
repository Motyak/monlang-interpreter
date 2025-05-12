#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>

#include <utils/variant-utils.h>
#include <utils/assert-utils.h>

#define unless(x) if(!(x))

const prim_value_t::Lambda builtin::prim_ctor::Bool __attribute__((init_priority(3000))) =
[](const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
    unless (args.size() == 1) SHOULD_NOT_HAPPEN();
    auto argVal = evaluateValue(args.at(0).expr, env);
    return new prim_value_t(Bool_(argVal));
};

static prim_value_t::Bool to_bool(const prim_value_t&);

prim_value_t::Bool builtin::prim_ctor::Bool_(const value_t& val) {
    return std::visit(overload{
        [](prim_value_t* primVal) -> prim_value_t::Bool {
            if (primVal == nullptr) {
                #ifdef TOGGLE_NIL_CAST_TO_BOOL
                return false;
                #endif
                SHOULD_NOT_HAPPEN();
            }
            return to_bool(*primVal);
        },
        [](auto*) -> prim_value_t::Bool {SHOULD_NOT_HAPPEN();},
    }, val);
}

static prim_value_t::Bool to_bool(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Byte byte) -> prim_value_t::Bool {return byte;},
        [](prim_value_t::Bool bool_) -> prim_value_t::Bool {return bool_;},
        [](prim_value_t::Int int_) -> prim_value_t::Bool {return int_;},

        [](prim_value_t::Float) -> prim_value_t::Bool {SHOULD_NOT_HAPPEN();},
        [](prim_value_t::Str) -> prim_value_t::Bool {SHOULD_NOT_HAPPEN();},
        [](prim_value_t::List) -> prim_value_t::Bool {SHOULD_NOT_HAPPEN();},
        [](prim_value_t::Map) -> prim_value_t::Bool {SHOULD_NOT_HAPPEN();},
        [](prim_value_t::Lambda) -> prim_value_t::Bool {SHOULD_NOT_HAPPEN();},
    }, primVal.variant);
}
