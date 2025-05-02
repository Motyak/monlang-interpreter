#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretVisitor.h>

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

value_t& evaluateLvalue(const Lvalue& lvalue, const Environment& env) {
    return std::visit(
        [&env](auto* expr) -> value_t& {return evaluateLvalue(*expr, env);},
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

void performStatement(const VarStatement&, Environment& env) {
    TODO();
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

void performStatement(const ExpressionStatement&, Environment& env) {
    TODO();
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

value_t evaluateValue(const Numeral&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const StrLiteral&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const Symbol&, const Environment& env) {
    TODO();
}


//==============================================================
// evaluateLvalue
//==============================================================

value_t& evaluateLvalue(const FieldAccess&, const Environment& env) {
    TODO();
}

value_t& evaluateLvalue(const Subscript&, const Environment& env) {
    TODO();
}

value_t& evaluateLvalue(const Symbol&, const Environment& env) {
    TODO();
}
