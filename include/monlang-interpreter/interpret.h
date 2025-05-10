#ifndef INTERPRET_H
#define INTERPRET_H

#include <monlang-interpreter/Environment.h>

#include <monlang-LV2/ast/Program.h>
#include <monlang-LV2/ast/Statement.h>
#include <monlang-LV2/ast/Expression.h>
#include <monlang-LV2/ast/Lvalue.h>

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

using namespace LV2;

extern thread_local bool INTERACTIVE_MODE;

void interpretProgram(const Program&, Environment* = nullptr);
void performStatement(const Statement&, Environment* = nullptr);
value_t evaluateValue(const Expression&, Environment* = nullptr);
value_t* evaluateLvalue(const Lvalue&, const Environment* = nullptr);

/* performStatement */
void performStatement(const Assignment&, Environment* = nullptr);
void performStatement(const Accumulation&, Environment* = nullptr);
void performStatement(const LetStatement&, const Environment* = nullptr);
void performStatement(const VarStatement&, Environment* = nullptr);
void performStatement(const ReturnStatement&, const Environment* = nullptr);
void performStatement(const BreakStatement&, const Environment* = nullptr);
void performStatement(const ContinueStatement&, const Environment* = nullptr);
void performStatement(const DieStatement&, const Environment* = nullptr);
void performStatement(const ForeachStatement&, const Environment* = nullptr);
void performStatement(const WhileStatement&, const Environment* = nullptr);
void performStatement(const DoWhileStatement&, const Environment* = nullptr);
void performStatement(const ExpressionStatement&, Environment* = nullptr);

/* evaluateValue */
value_t evaluateValue(const Operation&, const Environment* = nullptr);
value_t evaluateValue(const FunctionCall&, Environment* = nullptr);
value_t evaluateValue(const Lambda&, Environment* = nullptr);
value_t evaluateValue(const BlockExpression&, Environment* = nullptr);
value_t evaluateValue(const FieldAccess&, const Environment* = nullptr);
value_t evaluateValue(const Subscript&, const Environment* = nullptr);
value_t evaluateValue(const ListLiteral&, const Environment* = nullptr);
value_t evaluateValue(const MapLiteral&, const Environment* = nullptr);
value_t evaluateValue(const SpecialSymbol&, const Environment* = nullptr);
value_t evaluateValue(const Numeral&, const Environment* = nullptr);
value_t evaluateValue(const StrLiteral&, const Environment* = nullptr);
value_t evaluateValue(const Symbol&, const Environment* = nullptr);

/* evaluateLvalue */
value_t* evaluateLvalue(const FieldAccess&, const Environment* = nullptr);
value_t* evaluateLvalue(const Subscript&, const Environment* = nullptr);
value_t* evaluateLvalue(const Symbol&, const Environment* = nullptr);

#endif // INTERPRET_H
