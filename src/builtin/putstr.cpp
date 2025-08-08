#include <monlang-interpreter/builtin/putstr.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/builtin/print.h>

#define unless(x) if(!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::putstr __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() <= 1) throw InterpretError("putstr() takes at most 1 arg");
        if (args.size() == 1) {
            auto arg = args.at(0);
            auto argVal = evaluateValue(arg.expr, arg.env);
            builtin::putstr_({argVal});
        }
        return nil_value_t();
    }
}};
