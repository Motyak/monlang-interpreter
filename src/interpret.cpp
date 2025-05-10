#include <monlang-interpreter/interpret.h>

/* impl only */

#include <monlang-interpreter/builtin.h>

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
#include <utils/str-utils.h>

#include <cmath>

#define unless(x) if(!(x))

thread_local bool INTERACTIVE_MODE = false;

using Int = prim_value_t::Int;
using Byte = prim_value_t::Byte;
using Bool = prim_value_t::Bool;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;
using Lambda_ = prim_value_t::Lambda;

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

void performStatement(const Assignment& assignment, Environment& env) {
    auto* lvalue = evaluateLvalue(assignment.variable, env);
    auto old_value = evaluateValue(assignment.variable, env);
    auto new_value = evaluateValue(assignment.value, env);
    *lvalue = new_value;
    env.symbolTable["$old"] = Environment::ConstValue{old_value};
}

void performStatement(const Accumulation& acc, Environment& env) {
    auto op = (Expression)new Symbol{acc.operator_};
    auto lhs = (Expression)acc.variable;
    auto rhs = acc.value;
    auto fnCall = (Expression)new FunctionCall{op, {lhs, rhs}};

    auto variable = acc.variable;
    auto assignment = Assignment{variable, fnCall};

    performStatement(assignment, env);

    delete std::get<Symbol*>(op);
    delete std::get<FunctionCall*>(fnCall);
}

void performStatement(const LetStatement&, Environment& env) {
    TODO();
}

void performStatement(const VarStatement& varStmt, Environment& env) {
    // caught during static analysis
    ASSERT (!env.symbolTable.contains(varStmt.variable.name));

    auto value = evaluateValue(varStmt.value, env);
    auto* var = new value_t(value);
    env.symbolTable[varStmt.variable.name] = Environment::Variable{var};
}

void performStatement(const ReturnStatement&, Environment&) {
    SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
}

void performStatement(const BreakStatement&, Environment&) {
    SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
}

void performStatement(const ContinueStatement&, Environment&) {
    SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
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
    if (INTERACTIVE_MODE) {
        if (!is_nil(value)) {
            builtin::print({value});
        }
    }
}


//==============================================================
// evaluateValue
//==============================================================

value_t evaluateValue(const Operation&, const Environment& env) {
    TODO();
}

value_t evaluateValue(const FunctionCall& fnCall, const Environment& env) {

    /* TODO: tmp */
    ASSERT (std::holds_alternative<Symbol*>(fnCall.function));
    auto symbol = *std::get<Symbol*>(fnCall.function);
    ASSERT (BUILTIN_TABLE.contains(symbol.name));
    auto fn = BUILTIN_TABLE.at(symbol.name);
    std::vector<value_t> fnArgs;
    for (auto arg: fnCall.arguments) {
        auto argVal = evaluateValue(arg.expr, env);
        fnArgs.push_back(argVal);
    }
    return fn(fnArgs);


    // value_t function;

    // /*BREAKABLE BLOCK*/ for (int i = 1; i <= 1; ++i)
    // {
    //     unless (std::holds_alternative<Symbol*>(fnCall.function)) break;
    //     auto symbol = *std::get<Symbol*>(fnCall.function);
    //     if (env.enclosingEnv->symbolTable.contains(symbol.value)) {
    //         function = evaluateValue(symbol, env);
    //     }
    //     else if (symbol.value == "print") {
    //         std::vector<value_t> printArgs;
    //         for (auto arg: fnCall.arguments) {
    //             auto argVal = evaluateValue(arg.expr, env);
    //             printArgs.push_back(argVal);
    //         }
    //         builtin::print(printArgs);
    //         return nullptr;
    //     }
    //     // else if (...){...}
    //     else {
    //         // die: unbound symbol
    //     }
    // }

    // auto fnEnv = new Environment{{}, env};
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

value_t evaluateValue(const SpecialSymbol& specialSymbol, const Environment& env) {
    // should throw a runtime error here
    ASSERT (env.contains(specialSymbol.name));

    auto specialSymbolVal = env.at(specialSymbol.name);
    return std::visit(overload{
        [](Environment::ConstValue const_) -> value_t {return const_;},
        [](Environment::Variable) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToNonConst) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToLvalue /*or PassByRef*/) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::PassByDelayed) -> value_t {SHOULD_NOT_HAPPEN();},
    }, specialSymbolVal);
}

value_t evaluateValue(const Numeral& numeral, const Environment& env) {
    if (numeral.type == "int") {
        return new prim_value_t(Int(std::stoll(numeral.int1)));
    }

    if (numeral.type == "hex") {
        return new prim_value_t(Int(std::stoll(numeral.int1, nullptr, 16)));
    }
    
    if (numeral.type == "bin") {
        return new prim_value_t(Int(std::stoll(numeral.int1, nullptr, 2)));
    }

    if (numeral.type == "oct") {
        return new prim_value_t(Int(std::stoll(numeral.int1, nullptr, 8)));
    }

    if (numeral.type == "frac") {
        auto numerator = std::stoll(numeral.int1);
        auto denominator = std::stoll(numeral.int2);
        auto division = (double)numerator / denominator;
        return new prim_value_t(Float(division));
    }

    if (numeral.type == "pow") {
        auto base = std::stoll(numeral.int1);
        auto exponent = std::stoll(numeral.int2);
        auto power = std::powl(base, exponent);
        return new prim_value_t(Int(power));
    }

    if (numeral.type == "fix_only") {
        auto int_part = std::stoll(numeral.int1);
        auto numerator = std::stoll(numeral.fixed);
        auto denominator = std::powl(10, numeral.fixed.size());
        auto division = (double)numerator / denominator;
        auto sum = int_part + division;
        return new prim_value_t(Float(sum));
    }

    if (numeral.type == "per_only") {
        auto int_part = std::stoll(numeral.int1);
        auto numerator = std::stoll(numeral.periodic);
        auto denominator = std::powl(10, numeral.periodic.size()) - 1;
        auto division = (double)numerator / denominator;
        auto sum = int_part + division;
        return new prim_value_t(Float(sum));
    }

    if (numeral.type == "fix_and_per") {
        auto int_part = std::stoll(numeral.int1);
        auto fixed_part_numerator = std::stoll(numeral.fixed);
        auto fixed_part_denominator = std::powl(10, numeral.fixed.size());
        auto fixed_part_division = (double)fixed_part_numerator / fixed_part_denominator;
        auto periodic_part_numerator = std::stoll(numeral.periodic);
        auto periodic_part_denominator = (std::powl(10, numeral.periodic.size()) - 1) * fixed_part_denominator;
        auto periodic_part_division = (double)periodic_part_numerator / periodic_part_denominator;
        auto sum = int_part + fixed_part_division + periodic_part_division;
        return new prim_value_t(Float(sum));
    }

    SHOULD_NOT_HAPPEN(); // BUG unknown numeral type
}

value_t evaluateValue(const StrLiteral& strLiteral, const Environment& env) {
    return new prim_value_t(Str(strLiteral.str));
}

value_t evaluateValue(const Symbol& symbol, const Environment& env) {
    if (env.contains(symbol.name)) {
        auto symbol_val = env.at(symbol.name);
        return std::visit(overload{
            [](Environment::ConstValue /*or LabelToConst*/ const_){return const_;},
            [](Environment::Variable var){return *var;},
            [](Environment::LabelToNonConst label){return label();},
            [](Environment::LabelToLvalue /*or PassByRef*/ label_ref){return *label_ref();},
            [](Environment::PassByDelayed delayed){return delayed();},
        }, symbol_val);
    }
    else {
        /* technically it can't happen, so let's add a hidden feature
           for those who don't run the static analysis on prog
           before interpreting it
           => will return the symbol as a Str
        */
       return new prim_value_t(Str(symbol.name));
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

value_t* evaluateLvalue(const Symbol& symbol, const Environment& env) {
    // variable unbound, caught during static analysis
    ASSERT (env.contains(symbol.name));

    auto symbolVal = env.at(symbol.name);
    return std::visit(overload{
        [](const Environment::Variable& var) -> value_t* {return var;},
        [](Environment::LabelToLvalue& /*or PassByRef*/ var) -> value_t* {return var();},

        /* caught during static analysis */
        [](Environment::ConstValue /*or LabelToConst*/) -> value_t* {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToNonConst) -> value_t* {SHOULD_NOT_HAPPEN();},
        [](Environment::PassByDelayed) -> value_t* {SHOULD_NOT_HAPPEN();},
    }, symbolVal);

}
