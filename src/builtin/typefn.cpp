#include <monlang-interpreter/builtin/typefn.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#define unless(x) if (!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp


static auto PRIM_TYPE = std::map<std::string, prim_value_t*>{
    {"$nil", new prim_value_t{"$nil"}},
    {"Bool", new prim_value_t{"Bool"}},
    {"Int", new prim_value_t{"Int"}},
    {"Float", new prim_value_t{"Float"}},
    {"Str", new prim_value_t{"Str"}},
    {"List", new prim_value_t{"List"}},
    {"Map", new prim_value_t{"Map"}},
    {"Lambda", new prim_value_t{"Lambda"}},
};

const value_t builtin::typefn __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("$type() takes 1 arg");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        auto type = builtin::typefn_(argVal);
        return PRIM_TYPE.contains(type)? PRIM_TYPE.at(type) : new prim_value_t{type};
    }
}};

static prim_value_t::Str get_type(const prim_value_t&);

prim_value_t::Str builtin::typefn_(const value_t& val) {
    if (is_nil(val)) {
        return "$nil";
    }
    return std::visit(overload{
        [](prim_value_t* val) -> prim_value_t::Str {return get_type(*val);},
        [](type_value_t* val) -> prim_value_t::Str {return (prim_value_t::Str)val->typeTag;},
        [](struct_value_t* val) -> prim_value_t::Str {return (prim_value_t::Str)val->type;},
        [](enum_value_t* val) -> prim_value_t::Str {return (prim_value_t::Str)val->type;},
        [](char*) -> prim_value_t::Str {SHOULD_NOT_HAPPEN();},
        [](FieldLvalue*) -> prim_value_t::Str {SHOULD_NOT_HAPPEN();},
    }, val);
}

static prim_value_t::Str get_type(const prim_value_t& primVal) {
    return std::visit(overload{
        [](prim_value_t::Bool){return "Bool";},
        [](prim_value_t::Byte){return "Byte";},
        [](prim_value_t::Int){return "Int";},
        [](prim_value_t::Float){return "Float";},
        [](const prim_value_t::Str&){return "Str";},
        [](const prim_value_t::List&){return "List";},
        [](const prim_value_t::Map&){return "Map";},
        [](const prim_value_t::Lambda&){return "Lambda";},
    }, primVal.variant);
}
