#include <monlang-interpreter/builtin/exit.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <cstdlib>

#define unless(x) if (!(x))

const prim_value_t::Lambda builtin::exit __attribute__((init_priority(3000))) = {
    IntConst::ONE,
    [] [[noreturn]] (const std::vector<FunctionCall::Argument>& args, Environment* env) -> value_t {
        unless (args.size() == 1) throw InterpretError("exit() takes 1 arg");
        auto argVal = evaluateValue(args.at(0).expr, env);
        auto exitCode = builtin::prim_ctor::Byte_(argVal);
        ::exit(exitCode);
    }
};
