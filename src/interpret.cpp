#include <monlang-interpreter/interpret.h>

/* impl only */

/* statements */
#include <monlang-LV2/ast/stmt/Assignment.h>
#include <monlang-LV2/ast/stmt/Accumulation.h>
#include <monlang-LV2/ast/stmt/LetStatement.h>
#include <monlang-LV2/ast/stmt/VarStatement.h>
#include <monlang-LV2/ast/stmt/ReturnStatement.h>
#include <monlang-LV2/ast/stmt/BreakStatement.h>
#include <monlang-LV2/ast/stmt/ContinueStatement.h>
#include <monlang-LV2/ast/stmt/DieStatement.h>
#include <monlang-LV2/ast/stmt/ForeachStatement.h>
#include <monlang-LV2/ast/stmt/WhileStatement.h>
#include <monlang-LV2/ast/stmt/ExpressionStatement.h>

/* expressions */
#include <monlang-LV2/ast/expr/Operation.h>
#include <monlang-LV2/ast/expr/FunctionCall.h>
#include <monlang-LV2/ast/expr/Lambda.h>
#include <monlang-LV2/ast/expr/BlockExpression.h>
#include <monlang-LV2/ast/expr/FieldAccess.h>
#include <monlang-LV2/ast/expr/Subscript.h>
#include <monlang-LV2/ast/expr/ListLiteral.h>
#include <monlang-LV2/ast/expr/MapLiteral.h>
#include <monlang-LV2/ast/expr/SpecialSymbol.h>
#include <monlang-LV2/ast/expr/Numeral.h>
#include <monlang-LV2/ast/expr/StrLiteral.h>
#include <monlang-LV2/ast/expr/Symbol.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#include <cmath> // TODO: tmp?

thread_local bool INTERACTIVE_MODE = false;

void interpretProgram(const Program& prog) {
    Environment env;
    for (auto stmt: prog.statements) {
        performStatement(stmt, /*OUT*/env);
    }
}

void performStatement(const Statement& stmt, Environment& env) {
    std::visit(
        [&env](auto* stmt){performStatement(*stmt, /*OUT*/env);},
        stmt
    );
}

value_t evaluateValue(const Expression& expr, const Environment& env) {
    return std::visit(
        [&env](auto* expr){return evaluateValue(*expr, env);},
        expr
    );
}

value_t* evaluateLvalue(const Lvalue& lvalue, const Environment& env) {
    return std::visit(
        [&env](auto* expr){return evaluateLvalue(*expr, env);},
        lvalue.variant
    );
}

//==============================================================
// performStatement
//==============================================================

void performStatement(const Assignment&, Environment& env) {
    TODO();
}

void performStatement(const Accumulation&, Environment& env) {
    TODO();
}

void performStatement(const LetStatement&, Environment& env) {
    TODO();
}

void performStatement(const VarStatement& varStmt, Environment& env) {
    if (env.symbolTable.contains(varStmt.name.value)) {
        SHOULD_NOT_HAPPEN(); // TODO: die with a stacktrace
    }
    auto value = evaluateValue(varStmt.value, env);
    env.symbolTable[varStmt.name.value] = value;
}

void performStatement(const ReturnStatement&, Environment& env) {
    TODO();
}

void performStatement(const BreakStatement&, Environment& env) {
    TODO();
}

void performStatement(const ContinueStatement&, Environment& env) {
    TODO();
}

void performStatement(const DieStatement&, Environment& env) {
    TODO();
}

void performStatement(const ForeachStatement&, Environment& env) {
    TODO();
}

void performStatement(const WhileStatement&, Environment& env) {
    TODO();
}

void performStatement(const DoWhileStatement&, Environment& env) {
    TODO();
}

void performStatement(const ExpressionStatement& exprStmt, Environment& env) {
    auto value = evaluateValue(exprStmt.expression, env);
    std::cout << value << "\n"; // TODO: should only be done in interactive (REPL) mode
}


//==============================================================
// evaluateValue
//==============================================================

value_t evaluateValue(const Operation&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const FunctionCall&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const Lambda&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const BlockExpression&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const FieldAccess&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const Subscript&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const ListLiteral&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const MapLiteral&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const SpecialSymbol&, const Environment& env) {
    TODO();
}

#include <utils/str-utils.h>

value_t evaluateValue(const Numeral& numeral, const Environment& env) {
    /*
        most of this logic will be moved into parsing phase
        (and stored in the Numeral struct)

        we will add struct fields:
        - type (str enum): int,hex,bin,oct,div,pow,fix_only,per_only,fix_and_per
        - int1 (str, without digit sep) // used everywhere
        - int2 (str, without digit sep) // used in 'pow and 'div
        - fixed (str, without digit sep) // used in 'fix_only and 'fix_and_per
        - periodic (str, without digit sep) // used in 'per_only and 'fix_and_per
    */

    auto num_without_sep = replace_all(numeral.fmt, "'", "");

    if (numeral.fmt.starts_with("0x")) {
        return std::stoll(num_without_sep, nullptr, 16);
    }
    
    if (numeral.fmt.starts_with("0b")) {
        return std::stoll(num_without_sep, nullptr, 2);
    }

    if (numeral.fmt.starts_with("0o")) {
        return std::stoll(num_without_sep, nullptr, 8);
    }

    if (numeral.fmt.contains("/")) {
        auto numerator = split(num_without_sep, "/").at(0);
        auto denominator = split(num_without_sep, "/").at(1);
        // return std::stod(std::stoll(numerator) / std::stoll(denominator));
        return 0; // TODO: tmp
    }

    if (numeral.fmt.contains("^")) {
        auto base = split(num_without_sep, "^").at(0);
        auto power = split(num_without_sep, "^").at(1);
        // return std::powl(std::stoll(base), std::stoll(power));
        return 0; // TODO: tmp
    }

    if (numeral.fmt.contains(".")) {
        auto int_part = split(num_without_sep, ".").at(0);
        // if it contains a periodic part..
        if (split(num_without_sep, ".").at(1).ends_with(")")) {
            // TODO: construct fraction from int part, fixed dec part and periodic dec part..
            // ..and then convert to Float
            return 0; // TODO: tmp
        }
        else {
            auto dec_part = split(num_without_sep, ".").at(1);
            // return new prim_value_t::Float(std::stod(int_part + dec_part));
            return 0; // TODO: tmp
        }
    }

    return std::stoll(num_without_sep);
}

value_t evaluateValue(const StrLiteral&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const Symbol& symbol, const Environment& env) {
    if (env.symbolTable.contains(symbol.value)) {
        auto symbol_val = env.symbolTable.at(symbol.value);
        return std::visit(overload{
            [](Environment::ConstValue /*or LabelToConst*/ const_){return const_;},
            [](Environment::Variable var){return *var;},
            [](Environment::LabelToNonConst label){return label();},
            [](Environment::LabelToLvalue /*or PassByRef*/ label_ref){return *label_ref();},
            [](Environment::PassByDelayed delayed){return delayed();},
        }, symbol_val);
    }
    // else if (BUILTIN_TABLE.contains(symbol.value)) {
    //     // TODO: impl builtins
    // }
    // else {
    //     // TODO: return symbol name as a Str
    // }
    else {
        TODO();
    }
}


//==============================================================
// evaluateLvalue
//==============================================================

value_t* evaluateLvalue(const FieldAccess&, const Environment& env) {
    TODO();
}

value_t* evaluateLvalue(const Subscript&, const Environment& env) {
    TODO();
}

value_t* evaluateLvalue(const Symbol&, const Environment& env) {
    TODO();
}
