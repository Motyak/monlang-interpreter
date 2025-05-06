#include <monlang-interpreter/builtin/operators.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/loop-utils.h>

#define unless(x) if (!(x))

static value_t addByte(const std::vector<value_t>& varargs);
static value_t addInt(const std::vector<value_t>& varargs);
static value_t addFloat(const std::vector<value_t>& varargs);
static value_t concatStr(const std::vector<value_t>& varargs);
static value_t concatList(const std::vector<value_t>& varargs);
static value_t concatMap(const std::vector<value_t>& varargs);

value_t builtin::op::plus(const std::vector<value_t>& varargs) {

    // should throw runtime error
    unless (varargs.size() >= 2) SHOULD_NOT_HAPPEN();

    // should throw runtime error
    unless (std::holds_alternative<prim_value_t*>(varargs.at(0))) SHOULD_NOT_HAPPEN();
    auto first_arg = *std::get<prim_value_t*>(varargs.at(0));

    // dispatch impl based on first argument type
    return std::visit(overload{
        [&varargs](prim_value_t::Byte) -> value_t {return addByte(varargs);},
        [&varargs](prim_value_t::Int) -> value_t {return addInt(varargs);},
        [&varargs](prim_value_t::Float) -> value_t {return addFloat(varargs);},
        [&varargs](const prim_value_t::Str&) -> value_t {return concatStr(varargs);},
        [&varargs](const prim_value_t::List&) -> value_t {return concatList(varargs);},
        [&varargs](const prim_value_t::Map&) -> value_t {return concatMap(varargs);},

        /* should throw runtime error */
        [](prim_value_t::Bool) -> value_t {SHOULD_NOT_HAPPEN();},
        [](const prim_value_t::Lambda&) -> value_t {SHOULD_NOT_HAPPEN();},
    }, first_arg.variant);
}

static value_t addByte(const std::vector<value_t>& varargs) {
    TODO();
}

static value_t addInt(const std::vector<value_t>& varargs) {
    auto sum = prim_value_t::Int{0};

    LOOP for (auto arg: varargs) {
    if (__first_it)
    {
        // should throw runtime error
        unless (std::holds_alternative<prim_value_t*>(arg)) SHOULD_NOT_HAPPEN();
    }
        auto primVal = *std::get<prim_value_t*>(arg);

    if (__first_it)
    {
        // should throw runtime error
        unless (std::holds_alternative<prim_value_t::Int>(primVal.variant)) SHOULD_NOT_HAPPEN();
    }
        auto intVal = std::get<prim_value_t::Int>(primVal.variant);
        sum += intVal;

        ENDLOOP
    }

    return new prim_value_t{sum};
}

static value_t addFloat(const std::vector<value_t>& varargs) {
    TODO();
}

static value_t concatStr(const std::vector<value_t>& varargs) {
    TODO();
}

static value_t concatList(const std::vector<value_t>& varargs) {
    TODO();
}

static value_t concatMap(const std::vector<value_t>& varargs) {
    TODO();
}

