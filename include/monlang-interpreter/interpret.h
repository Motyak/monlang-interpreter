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

extern std::string ARG0;
extern std::string SRCNAME;
extern std::vector<std::string> SRC_ARGS;
extern bool INTERACTIVE_MODE;

void interpretProgram(const Program&);
void performStatement(const Statement&, Environment*);
value_t evaluateValue(const Expression&, Environment*);
value_t* evaluateLvalue(const Lvalue&, Environment*, bool subscripted = false);

/* performStatement */
void performStatement(const Assignment&, Environment*);
void performStatement(const Accumulation&, Environment*);
void performStatement(const LetStatement&, Environment*);
void performStatement(const VarStatement&, Environment*);
void performStatement(const ReturnStatement&, const Environment*);
void performStatement(const BreakStatement&, const Environment*);
void performStatement(const ContinueStatement&, const Environment*);
void performStatement(const DieStatement&, const Environment*);
void performStatement(const ForeachStatement&, const Environment*);
void performStatement(const WhileStatement&, const Environment*);
void performStatement(const DoWhileStatement&, const Environment*);
void performStatement(const ExpressionStatement&, Environment*);

/* evaluateValue */
value_t evaluateValue(const Operation&, Environment*);
value_t evaluateValue(const FunctionCall&, Environment*);
value_t evaluateValue(const Lambda&, Environment*);
value_t evaluateValue(const BlockExpression&, Environment*);
value_t evaluateValue(const FieldAccess&, const Environment*);
value_t evaluateValue(const Subscript&, Environment*);
value_t evaluateValue(const ListLiteral&, Environment*);
value_t evaluateValue(const MapLiteral&, Environment*);
value_t evaluateValue(const SpecialSymbol&, const Environment*);
value_t evaluateValue(const Numeral&, const Environment*);
value_t evaluateValue(const StrLiteral&, const Environment*);
value_t evaluateValue(const Symbol&, const Environment*);

/* evaluateLvalue */
value_t* evaluateLvalue(const FieldAccess&, Environment*);
value_t* evaluateLvalue(const Subscript&, Environment*);
value_t* evaluateLvalue(const Symbol&, Environment*, bool subscripted = false);

#endif // INTERPRET_H
