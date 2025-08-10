#include <monlang-interpreter/builtin/exit.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <cstdlib>

#define unless(x) if (!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::exit __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE,
    [] [[noreturn]] (const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("exit() takes 1 arg");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr); // no need to pop
        auto exitCode = builtin::prim_ctor::Byte_(argVal);
        ::exit(exitCode);
    }
}};
