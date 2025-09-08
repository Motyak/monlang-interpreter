#include <monlang-interpreter/builtin/typefn.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#define unless(x) if (!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

static value_t get_type(const prim_value_t&);

const value_t builtin::typefn __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("$type() takes 1 arg");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        auto type = builtin::prim_ctor::Str_(argVal);
        return std::visit(overload{
            [](prim_value_t* val) -> value_t {return get_type(*val);},
            [](type_value_t* val) -> value_t {return new prim_value_t{(prim_value_t::Str)val->type};},
            [](struct_value_t* val) -> value_t {return new prim_value_t{(prim_value_t::Str)val->type};},
            [](enum_value_t* val) -> value_t {return new prim_value_t{(prim_value_t::Str)val->type};},
            [](char*) -> value_t {SHOULD_NOT_HAPPEN();},
        }, argVal);
    }
}};

static value_t get_type(const prim_value_t& primVal) {
    static auto bool_type = new prim_value_t{prim_value_t::Str("Bool")};
    static auto byte_type = new prim_value_t{prim_value_t::Str("Byte")};
    static auto int_type = new prim_value_t{prim_value_t::Str("Int")};
    static auto float_type = new prim_value_t{prim_value_t::Str("Float")};
    static auto char_type = new prim_value_t{prim_value_t::Str("Char")};
    static auto str_type = new prim_value_t{prim_value_t::Str("Str")};
    static auto list_type = new prim_value_t{prim_value_t::Str("List")};
    static auto map_type = new prim_value_t{prim_value_t::Str("Map")};
    static auto lambda_type = new prim_value_t{prim_value_t::Str("Lambda")};
    return std::visit(overload{
        [](prim_value_t::Bool){return bool_type;},
        [](prim_value_t::Byte){return byte_type;},
        [](prim_value_t::Int){return int_type;},
        [](prim_value_t::Float){return float_type;},
        [](prim_value_t::Char){return char_type;},
        [](const prim_value_t::Str&){return str_type;},
        [](const prim_value_t::List&){return list_type;},
        [](const prim_value_t::Map&){return map_type;},
        [](const prim_value_t::Lambda&){return lambda_type;},
    }, primVal.variant);
}
