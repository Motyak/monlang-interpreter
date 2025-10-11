#include <monlang-interpreter/builtin/sleep.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>

#include <thread>

#define unless(x) if (!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::sleep __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 1) throw InterpretError("sleep() takes 1 arg");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        ::activeCallStack.push_back(arg.expr);
        defer {safe_pop_back(::activeCallStack);};
        auto duration = builtin::prim_ctor::Float_(argVal);
        std::this_thread::sleep_for(std::chrono::milliseconds{int64_t(duration * 1000)});
        return nil_value_t();
    }
}};
