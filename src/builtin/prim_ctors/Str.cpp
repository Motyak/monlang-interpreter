#include <monlang-interpreter/builtin/prim_ctors.h>

/* impl only */
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/print.h>

#include <utils/assert-utils.h>

#include <sstream>

#define unless(x) if(!(x))

const value_t builtin::prim_ctor::Str __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("Str() takes 1 argument");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        return new prim_value_t(Str_(argVal));
    }
}};

prim_value_t::Str builtin::prim_ctor::Str_(const value_t& val) {
    //TODO: if val is a type_value_t that inherits from Str,..
    // ..then we return its Str value instead

    std::ostringstream oss;
    print_({val}, oss);
    auto res = oss.str();
    res.pop_back(); // remove trailing newline
    return res;
}
