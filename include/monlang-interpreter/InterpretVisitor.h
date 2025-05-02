#ifndef INTERPRET_VISITOR_H
#define INTERPRET_VISITOR_H

#include <monlang-interpreter/Environment.h>

#include <monlang-LV2/ast/visitors/visitor.h>

using namespace LV2;

class InterpretVisitor : AstVisitor<void> {
    // using value_t = int; // TODO: tmp

  private:
    Environment& env;
    // std::vector<value_t> stack;

  public:
    InterpretVisitor(Environment& env) : env(env){}

    void operator()(const Program&) override;
    void operator()(const Statement&) override;
    void operator()(const Expression&) override;

    /* statements */
    void operator()(Assignment*);
    void operator()(Accumulation*);
    void operator()(LetStatement*);
    void operator()(VarStatement*);
    void operator()(ReturnStatement*);
    void operator()(BreakStatement*);
    void operator()(ContinueStatement*);
    void operator()(DieStatement*);
    void operator()(ForeachStatement*);
    void operator()(WhileStatement*);
    void operator()(DoWhileStatement*);
    void operator()(ExpressionStatement*);

    /* expressions */
    void operator()(Operation*);
    void operator()(FunctionCall*);
    void operator()(Lambda*);
    void operator()(BlockExpression*);
    void operator()(FieldAccess*);
    void operator()(Subscript*);
    void operator()(ListLiteral*);
    void operator()(MapLiteral*);
    void operator()(SpecialSymbol*);
    void operator()(Numeral*);
    void operator()(StrLiteral*);
    void operator()(Symbol*);
};

#endif // INTERPRET_VISITOR_H
