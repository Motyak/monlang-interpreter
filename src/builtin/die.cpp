#include <monlang-interpreter/builtin/die.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/ProgramAssertion.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <cstdlib>

#define unless(x) if (!(x))

const value_t builtin::die __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    IntConst::ZERO,
    [] [[noreturn]] (const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() <= 1) throw InterpretError("die() takes at most 1 argument");
        std::string msg;
        if (args.size() == 1) {
            auto argVal = evaluateValue(args[0].expr, args[0].env);
            msg = builtin::prim_ctor::Str_(argVal);
        }
        throw ProgramAssertion(msg);
    }
}};
