#include <monlang-interpreter/InterpretVisitor.h>

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

void InterpretVisitor::operator()(const Program& prog) {
    for (auto stmt: prog.statements) {
        operator()(stmt);
    }
}

void InterpretVisitor::operator()(const Statement& stmt) {
    std::visit(*this, stmt);
}

void InterpretVisitor::operator()(const Expression& expr) {
    std::visit(*this, expr);
}

//==============================================================
// STMT
//==============================================================

void InterpretVisitor::operator()(Assignment*) {
    TODO();
}

void InterpretVisitor::operator()(Accumulation*) {
    TODO();
}

void InterpretVisitor::operator()(LetStatement*) {
    TODO();
}

void InterpretVisitor::operator()(VarStatement*) {
    TODO();
}

void InterpretVisitor::operator()(ReturnStatement*) {
    TODO();
}

void InterpretVisitor::operator()(BreakStatement*) {
    TODO();
}

void InterpretVisitor::operator()(ContinueStatement*) {
    TODO();
}

void InterpretVisitor::operator()(DieStatement*) {
    TODO();
}

void InterpretVisitor::operator()(ForeachStatement*) {
    TODO();
}

void InterpretVisitor::operator()(WhileStatement*) {
    TODO();
}

void InterpretVisitor::operator()(DoWhileStatement*) {
    TODO();
}

void InterpretVisitor::operator()(ExpressionStatement* exprStmt) {
    operator()(exprStmt->expression);
}

//==============================================================
// EXPR
//==============================================================

void InterpretVisitor::operator()(Operation*) {
    TODO();
}

void InterpretVisitor::operator()(FunctionCall* fnCall) {
    // resolve function value
    operator()(fnCall->function);

    // resolve arguments (get their respective value or build a thunk to compute it later)

}

void InterpretVisitor::operator()(Lambda*) {
    TODO();
}

void InterpretVisitor::operator()(BlockExpression*) {
    TODO();
}

void InterpretVisitor::operator()(FieldAccess*) {
    TODO();
}

void InterpretVisitor::operator()(Subscript*) {
    TODO();
}

void InterpretVisitor::operator()(ListLiteral*) {
    TODO();
}

void InterpretVisitor::operator()(MapLiteral*) {
    TODO();
}

void InterpretVisitor::operator()(SpecialSymbol*) {
    TODO();
}

void InterpretVisitor::operator()(Numeral*) {
    TODO();
}

void InterpretVisitor::operator()(StrLiteral*) {
    TODO();
}

void InterpretVisitor::operator()(Symbol*) {
    TODO();
}

