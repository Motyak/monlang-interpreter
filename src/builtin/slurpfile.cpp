#include <monlang-interpreter/builtin/slurpfile.h>

/* impl only */

#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin/prim_ctors.h>

#include <utils/assert-utils.h>

#include <fstream>

#define unless(x) if(!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp

const value_t builtin::slurpfile __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::ONE,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() <= 1) throw InterpretError("slurpfile() takes 1 argument");
        auto arg = args.at(0);
        auto argVal = evaluateValue(arg.expr, arg.env);
        ASSERT (std::holds_alternative<prim_value_t*>(argVal)); // TODO
        auto argPrimValPtr = std::get<prim_value_t*>(argVal);
        unless (std::holds_alternative<prim_value_t::Str>(argPrimValPtr->variant)) {
            throw InterpretError("slurpfile() expects a Str argument");
        }

        auto filename = std::get<prim_value_t::Str>(argPrimValPtr->variant);
        auto file = std::ifstream(filename);
        if (!file.is_open()) {
            throw InterpretError("Can't open file `" + filename + "`");
        }
        auto res = std::string(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );

        return new prim_value_t{res};
    }
}};
