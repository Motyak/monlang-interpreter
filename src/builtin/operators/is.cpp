#include <monlang-interpreter/builtin/operators.h>

/* impl only */

#include <monlang-interpreter/builtin/prim_ctors.h>
#include <monlang-interpreter/builtin/typefn.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/assert-utils.h>

#define unless(x) if (!(x))

extern uint64_t builtin_lambda_id; // defined in src/interpret.cpp
extern std::map<std::string, std::vector<std::string>> type_table; // defined in src/interpret.cpp

const value_t builtin::op::is __attribute__((init_priority(3000))) = new prim_value_t{prim_value_t::Lambda{
    builtin_lambda_id++,
    IntConst::TWO,
    [](const std::vector<FlattenArg>& args) -> value_t {
        unless (args.size() == 2) throw InterpretError("is() takes 2 argument");
        auto rhs = args.at(1);
        auto rhsVal = evaluateValue(rhs.expr, rhs.env);
        unless (is_(rhsVal, "Str")) {
            throw InterpretError("is() rhs must be a Str");
        }
        auto rhsStr = prim_ctor::Str_(rhsVal);

        auto lhs = args.at(0);
        auto lhsVal = evaluateValue(lhs.expr, lhs.env);
        return is_(lhsVal, rhsStr)? BoolConst::TRUE : BoolConst::FALSE;
    }
}};

bool builtin::op::is_(const value_t& lhs, const std::string& rhs) {
    return is_(typefn_(lhs), rhs);
}

bool builtin::op::is_(const std::string& lhs, const std::string& rhs) {
    if (lhs == rhs) {
        return true;
    }
    if (lhs == "$nil") {
        return false;
    }
    ASSERT (type_table.contains(lhs));
    // recursive
    for (const auto& subtype: type_table.at(lhs)) {
        if (rhs == subtype || is_(subtype, rhs)) {
            return true;
        }
    }
    return false;
}
