#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>

#define unless(x) if(!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::prim_ctor::Float __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    own(IntConst::ONE()),
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("Float() takes 1 argument");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        defer {safe_pop_back(::activeCallStack);};
        return new prim_value_t(Float_(argVal));
    }
}};

static prim_value_t::Float to_float(const prim_value_t&);
static prim_value_t::Float to_float(const type_value_t&);
static prim_value_t::Float to_float(const struct_value_t&);
static prim_value_t::Float to_float(const enum_value_t&);

prim_value_t::Float builtin::prim_ctor::Float_(const value_t& val) {
    return std::visit(overload{
        [](auto* val) -> prim_value_t::Float {
            if (val == nullptr){
                throw InterpretError("Float() arg cannot be $nil");
            }
            else {
                return to_float(*val);
            }
        },
        [](char*) -> prim_value_t::Float {SHOULD_NOT_HAPPEN();},
    }, val);
}

static prim_value_t::Float to_float(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Byte byte) -> prim_value_t::Float {return byte;},
        [](prim_value_t::Int int_) -> prim_value_t::Float {return int_;},
        [](prim_value_t::Float float_) -> prim_value_t::Float {return float_;},

        [](prim_value_t::Bool) -> prim_value_t::Float {throw InterpretError("Float() arg cannot be a Bool");},
        [](prim_value_t::Char) -> prim_value_t::Float {throw InterpretError("Float() arg cannot be a Char");},
        [](const prim_value_t::Str&) -> prim_value_t::Float {throw InterpretError("Float() arg cannot be a Str");},
        [](const prim_value_t::List&) -> prim_value_t::Float {throw InterpretError("Float() arg cannot be a List");},
        [](const prim_value_t::Map&) -> prim_value_t::Float {throw InterpretError("Float() arg cannot be a Map");},
        [](const prim_value_t::Lambda&) -> prim_value_t::Float {throw InterpretError("Float() arg cannot be a Lambda");},
    }, primVal.variant);
}

static prim_value_t::Float to_float(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

static prim_value_t::Float to_float(const struct_value_t&) {
    throw InterpretError("Float() arg cannot be a struct val");
}

static prim_value_t::Float to_float(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}
