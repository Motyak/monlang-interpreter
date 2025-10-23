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
#include <monlang-LV2/ast/stmt/NullStatement.h>
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
void performStatement(const Statement&, std::shared_ptr<Environment>);
value_t evaluateValue(const Expression&, std::shared_ptr<Environment>);
value_t* evaluateLvalue(const Lvalue&, std::shared_ptr<Environment>, bool subscripted = false);

/* performStatement */
void performStatement(const Assignment&, std::shared_ptr<Environment>);
void performStatement(const Accumulation&, std::shared_ptr<Environment>);
void performStatement(const LetStatement&, std::shared_ptr<Environment>);
void performStatement(const VarStatement&, std::shared_ptr<Environment>);
void performStatement(const ReturnStatement&, const std::shared_ptr<Environment>);
void performStatement(const BreakStatement&, const std::shared_ptr<Environment>);
void performStatement(const ContinueStatement&, const std::shared_ptr<Environment>);
void performStatement(const DieStatement&, const std::shared_ptr<Environment>);
void performStatement(const ForeachStatement&, const std::shared_ptr<Environment>);
void performStatement(const WhileStatement&, const std::shared_ptr<Environment>);
void performStatement(const DoWhileStatement&, const std::shared_ptr<Environment>);
void performStatement(const NullStatement&, const std::shared_ptr<Environment>);
void performStatement(const ExpressionStatement&, std::shared_ptr<Environment>);

/* evaluateValue */
value_t evaluateValue(const Operation&, std::shared_ptr<Environment>);
value_t evaluateValue(const FunctionCall&, std::shared_ptr<Environment>);
value_t evaluateValue(const Lambda&, std::shared_ptr<Environment>);
value_t evaluateValue(const BlockExpression&, std::shared_ptr<Environment>);
value_t evaluateValue(const FieldAccess&, std::shared_ptr<Environment>);
value_t evaluateValue(const Subscript&, std::shared_ptr<Environment>);
value_t evaluateValue(const ListLiteral&, std::shared_ptr<Environment>);
value_t evaluateValue(const MapLiteral&, std::shared_ptr<Environment>);
value_t evaluateValue(const SpecialSymbol&, const std::shared_ptr<Environment>);
value_t evaluateValue(const Numeral&, const std::shared_ptr<Environment>);
value_t evaluateValue(const StrLiteral&, const std::shared_ptr<Environment>);
value_t evaluateValue(const Symbol&, const std::shared_ptr<Environment>);

/* evaluateLvalue */
value_t* evaluateLvalue(const FieldAccess&, std::shared_ptr<Environment>);
value_t* evaluateLvalue(const Subscript&, std::shared_ptr<Environment>);
value_t* evaluateLvalue(const Symbol&, const std::shared_ptr<Environment>, bool subscripted = false);

#endif // INTERPRET_H
