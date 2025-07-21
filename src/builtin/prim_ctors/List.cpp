#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

const value_t builtin::prim_ctor::List __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::ZERO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        std::vector<value_t> res;
        res.reserve(args.size());
        for (auto arg: args) {
            auto currArgVal = evaluateValue(arg.expr, arg.env);
            res.push_back(currArgVal);
        }
        return new prim_value_t{prim_value_t::List(res)};
    }
}};

static prim_value_t::List to_list(const prim_value_t&);
static prim_value_t::List to_list(const type_value_t&);
static prim_value_t::List to_list(const struct_value_t&);
static prim_value_t::List to_list(const enum_value_t&);

prim_value_t::List builtin::prim_ctor::List_(const value_t& container) {
    return std::visit(overload{
        [](auto* val) -> prim_value_t::List {
            if (val == nullptr){
                throw InterpretError("$nil is not a container");
            }
            else {
                return to_list(*val);
            }
        },
        [](char*) -> prim_value_t::List {SHOULD_NOT_HAPPEN();},
    }, container);
}

static prim_value_t::List to_list(const prim_value_t& primVal) {
    return std::visit(overload{
        [](const prim_value_t::List& list) -> prim_value_t::List {return list;},
        [](const prim_value_t::Str& str) -> prim_value_t::List {
            std::vector<value_t> res;
            res.reserve(str.size());
            for (auto c: str) {
                res.push_back(new prim_value_t{prim_value_t::Char(c)});
            }
            return res;
        },
        [](const prim_value_t::Map&) -> prim_value_t::List {TODO();},

        [](prim_value_t::Byte) -> prim_value_t::List {throw InterpretError("Byte is not a container");},
        [](prim_value_t::Char) -> prim_value_t::List {throw InterpretError("Char is not a container");},
        [](prim_value_t::Int) -> prim_value_t::List {throw InterpretError("Int is not a container");},
        [](prim_value_t::Bool) -> prim_value_t::List {throw InterpretError("Bool is not a container");},
        [](prim_value_t::Float) -> prim_value_t::List {throw InterpretError("Float is not a container");},
        [](const prim_value_t::Lambda&) -> prim_value_t::List {throw InterpretError("Lambda is not a container");},
    }, primVal.variant);
}

static prim_value_t::List to_list(const type_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}

static prim_value_t::List to_list(const struct_value_t&) {
    throw InterpretError("a struct val is not a container");
}

static prim_value_t::List to_list(const enum_value_t&) {
    SHOULD_NOT_HAPPEN(); // TODO: tmp
}
