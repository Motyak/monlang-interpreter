#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>

#define unless(x) if(!(x))

value_t IntConst::ZERO(){return std::make_unique<prim_value_t>(prim_value_t::Int(0));}
value_t IntConst::ONE(){return std::make_unique<prim_value_t>(prim_value_t::Int(1));}
value_t IntConst::TWO(){return std::make_unique<prim_value_t>(prim_value_t::Int(2));}

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::prim_ctor::Int __attribute__((init_priority(3000))) = std::make_unique<prim_value_t>(prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE(),
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("Int() takes 1 argument");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        defer {safe_pop_back(::activeCallStack);};
        return std::make_unique<prim_value_t>(Int_(argVal));
    }
});

static prim_value_t::Int to_int(const prim_value_t&);
static prim_value_t::Int to_int(const type_value_t&);
static prim_value_t::Int to_int(const struct_value_t&);
static prim_value_t::Int to_int(const enum_value_t&);

prim_value_t::Int builtin::prim_ctor::Int_(const value_t& val) {
    return std::visit(overload{
        [](const std::unique_ptr<prim_value_t>& val) -> prim_value_t::Int {
            if (val == nullptr){
                throw InterpretError("Int() arg cannot be $nil");
            }
            else {
                return to_int(*val);
            }
        },
        [](char*) -> prim_value_t::Int {SHOULD_NOT_HAPPEN();},
        [](auto*) -> prim_value_t::Int {TODO();},
    }, val);
}

static prim_value_t::Int to_int(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Bool bool_) -> prim_value_t::Int {return bool_;},
        [](prim_value_t::Byte byte) -> prim_value_t::Int {return byte;},
        [](prim_value_t::Int int_) -> prim_value_t::Int {return int_;},
        [](prim_value_t::Float float_) -> prim_value_t::Int {return float_;},
        [](prim_value_t::Char char_) -> prim_value_t::Int {return (unsigned char)char_;},

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
