#ifndef INTERPRET_H
#define INTERPRET_H

#include <monlang-interpreter/Environment.h>

#include <monlang-LV2/ast/Program.h>
#include <monlang-LV2/ast/Lvalue.h>

using namespace LV2;

void interpretProgram(const Program&);
void performStatement(const Statement&, Environment&);
value_t evaluateValue(const Expression&, const Environment&);
value_t& evaluateLvalue(const Lvalue&, const Environment&);

/* performStatement */
void performStatement(const Assignment&, Environment&);
void performStatement(const Accumulation&, Environment&);
void performStatement(const LetStatement&, Environment&);
void performStatement(const VarStatement&, Environment&);
void performStatement(const ReturnStatement&, Environment&);
void performStatement(const BreakStatement&, Environment&);
void performStatement(const ContinueStatement&, Environment&);
void performStatement(const DieStatement&, Environment&);
void performStatement(const ForeachStatement&, Environment&);
void performStatement(const WhileStatement&, Environment&);
void performStatement(const DoWhileStatement&, Environment&);
void performStatement(const ExpressionStatement&, Environment&);

/* evaluateValue */
value_t evaluateValue(const Operation&, const Environment&);
value_t evaluateValue(const FunctionCall&, const Environment&);
value_t evaluateValue(const Lambda&, const Environment&);
value_t evaluateValue(const BlockExpression&, const Environment&);
value_t evaluateValue(const FieldAccess&, const Environment&);
value_t evaluateValue(const Subscript&, const Environment&);
value_t evaluateValue(const ListLiteral&, const Environment&);
value_t evaluateValue(const MapLiteral&, const Environment&);
value_t evaluateValue(const SpecialSymbol&, const Environment&);
value_t evaluateValue(const Numeral&, const Environment&);
value_t evaluateValue(const StrLiteral&, const Environment&);
value_t evaluateValue(const Symbol&, const Environment&);

/* evaluateLvalue */
value_t& evaluateLvalue(const FieldAccess&, const Environment&);
value_t& evaluateLvalue(const Subscript&, const Environment&);
value_t& evaluateLvalue(const Symbol&, const Environment&);

#endif // INTERPRET_H
