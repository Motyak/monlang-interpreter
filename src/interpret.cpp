#include <monlang-interpreter/interpret.h>

/* impl only */

#include <monlang-interpreter/builtin.h>

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
namespace LV2 {using Lambda = Lambda;}

void interpretProgram(const Program& prog, Environment* env) {
    if (!env) {
        env = new Environment{}; // TODO: leak?
    }

    for (auto stmt: prog.statements) {
        performStatement(stmt, env);
    }
}

void performStatement(const Statement& stmt, Environment* env) {
    std::visit(
        [&env](auto* stmt){performStatement(*stmt, env);},
        stmt
    );
}

value_t evaluateValue(const Expression& expr, Environment* env) {
    return std::visit(overload{
        [&env](Lambda* lambda){return evaluateValue(*lambda, env);},
        [&env](auto* expr){return evaluateValue(*expr, env);},
    }, expr);
}

value_t* evaluateLvalue(const Lvalue& lvalue, const Environment* env) {
    return std::visit(
        [&env](auto* expr){return evaluateLvalue(*expr, env);},
        lvalue.variant
    );
}

//==============================================================
// performStatement
//==============================================================

void performStatement(const Assignment& assignment, Environment* env) {
    auto* lvalue = evaluateLvalue(assignment.variable, env);
    auto old_value = *lvalue;
    auto new_value = evaluateValue(assignment.value, env);
    *lvalue = new_value;
    env->symbolTable["$old"] = Environment::ConstValue{old_value};
}

void performStatement(const Accumulation& acc, Environment* env) {
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

void performStatement(const LetStatement&, const Environment* env) {
    TODO();
}

void performStatement(const VarStatement& varStmt, Environment* env) {
    // caught during static analysis
    ASSERT (!env->symbolTable.contains(varStmt.variable.name));

    auto value = evaluateValue(varStmt.value, env);
    auto* var = new value_t(value);
    env->symbolTable[varStmt.variable.name] = Environment::Variable{var};
}

void performStatement(const ReturnStatement&, const Environment*) {
    SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
}

void performStatement(const BreakStatement&, const Environment*) {
    SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
}

void performStatement(const ContinueStatement&, const Environment*) {
    SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
}

void performStatement(const DieStatement&, const Environment* env) {
    TODO();
}

void performStatement(const ForeachStatement&, const Environment* env) {
    TODO();
}

void performStatement(const WhileStatement&, const Environment* env) {
    TODO();
}

void performStatement(const DoWhileStatement&, const Environment* env) {
    TODO();
}

void performStatement(const ExpressionStatement& exprStmt, Environment* env) {
    auto value = evaluateValue(exprStmt.expression, env);
    if (INTERACTIVE_MODE) {
        if (!is_nil(value)) {
            builtin::print_({value});
        }
    }
}


//==============================================================
// evaluateValue
//==============================================================

value_t evaluateValue(const Operation& optor, Environment* env) {
    auto op = (Expression)new Symbol{optor.operator_};
    auto lhs = optor.leftOperand;
    auto rhs = optor.rightOperand;
    auto fnCall = (Expression)new FunctionCall{op, {lhs, rhs}};

    auto res = evaluateValue(fnCall, env);

    delete std::get<Symbol*>(op);
    delete std::get<FunctionCall*>(fnCall);

    return res;
}

value_t evaluateValue(const FunctionCall& fnCall, Environment* env) {
    auto fnVal = evaluateValue(fnCall.function, env);
    ASSERT (std::holds_alternative<prim_value_t*>(fnVal)); // TODO: tmp
    auto fnPrimVal = *std::get<prim_value_t*>(fnVal);
    ASSERT (std::holds_alternative<prim_value_t::Lambda>(fnPrimVal.variant)); // TODO: tmp
    auto function = std::get<prim_value_t::Lambda>(fnPrimVal.variant);
    return function(fnCall.arguments, env);
}

// non-const env param?
value_t evaluateValue(const LV2::Lambda& lambda, Environment* env) {
    Environment* envAtCreation = env;
    auto lambdaVal = prim_value_t::Lambda{
        [envAtCreation, &lambda](const std::vector<FunctionCall::Argument>& args, Environment* envAtApp) -> value_t {
            /*
                create a temporary new environment, based on the captured-one,
                in which we resolve each fn call arg with respect with the environment at application time,
                and then bind each value to its argument-associated lambda parameter..
            */
            auto parametersBinding = std::map<Environment::SymbolName, Environment::SymbolValue>{};
            // TODO: variadic functions
            for (size_t i = 0; i < args.size(); ++i) {
                auto& currParam = lambda.parameters.at(i);
                auto& currArg = args.at(i);

                if (currArg.passByRef) {
                    auto currArg_ = Lvalue{currArg.expr};
                    parametersBinding[currParam.name] = Environment::PassByRef{
                        [&currArg_, envAtApp]() -> value_t* {
                            return evaluateLvalue(currArg_, envAtApp);
                        }
                    };
                }
                else {
                    auto var = new value_t{evaluateValue(currArg.expr, envAtApp)}; // TODO: leak
                    parametersBinding[currParam.name] = Environment::Variable{var};
                }
            }
            auto lambdaEnv = Environment{.symbolTable = parametersBinding, .enclosingEnv = envAtCreation};

            /*

                ..then interpret the lambda's body with respect with this freshly-created
                environment.
            */
            return evaluateValue(lambda.body, &lambdaEnv);
        }
    };
    return new prim_value_t{lambdaVal};
}

value_t evaluateValue(const BlockExpression& blockExpr, Environment* env) {
    if (blockExpr.statements.size() == 0) {
        return nil_value_t();
    }
    auto newEnv = new Environment{{}, env};

    size_t i = 0;
    for (; i < blockExpr.statements.size() - 1; ++i) {
        performStatement(blockExpr.statements.at(i), newEnv);
    }

    if (std::holds_alternative<ExpressionStatement*>(blockExpr.statements.at(i))) {
        auto exprStmt = *std::get<ExpressionStatement*>(blockExpr.statements.at(i));
        return evaluateValue(exprStmt.expression, newEnv);
    }
    performStatement(blockExpr.statements.at(i), newEnv);
    return nil_value_t();
}

value_t evaluateValue(const FieldAccess&, const Environment* env) {
    TODO();
}

value_t evaluateValue(const Subscript&, const Environment* env) {
    TODO();
}

value_t evaluateValue(const ListLiteral&, const Environment* env) {
    TODO();
}

value_t evaluateValue(const MapLiteral&, const Environment* env) {
    TODO();
}

value_t evaluateValue(const SpecialSymbol& specialSymbol, const Environment* env) {
    static auto Bool_true = prim_value_t(Bool(true));
    static auto Bool_false = prim_value_t(Bool(false));

    if (specialSymbol.name == "$true") {
        return &Bool_true;
    }

    if (specialSymbol.name == "$false") {
        return &Bool_false;
    }

    // should throw a runtime error here
    ASSERT (env->contains(specialSymbol.name));

    auto specialSymbolVal = env->at(specialSymbol.name);
    return std::visit(overload{
        [](Environment::ConstValue const_) -> value_t {return const_;},
        [](Environment::Variable) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToNonConst) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToLvalue /*or PassByRef*/) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::PassByDelayed) -> value_t {SHOULD_NOT_HAPPEN();},
    }, specialSymbolVal);
}

value_t evaluateValue(const Numeral& numeral, const Environment* env) {
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

value_t evaluateValue(const StrLiteral& strLiteral, const Environment* env) {
    return new prim_value_t(Str(strLiteral.str));
}

value_t evaluateValue(const Symbol& symbol, const Environment* env) {
    if (env->contains(symbol.name)) {
        auto symbol_val = env->at(symbol.name);
        return std::visit(overload{
            [](Environment::ConstValue /*or LabelToConst*/ const_){return const_;},
            [](Environment::Variable var){return *var;},
            [](Environment::LabelToNonConst label){return label();},
            [](Environment::LabelToLvalue /*or PassByRef*/ label_ref){return *label_ref();},
            [](Environment::PassByDelayed delayed){return delayed();},
        }, symbol_val);
    }
    else if (BUILTIN_TABLE.contains(symbol.name)) {
        auto builtin_fn = BUILTIN_TABLE.at(symbol.name);
        return new prim_value_t{builtin_fn};
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

value_t* evaluateLvalue(const FieldAccess&, const Environment* env) {
    TODO();
}

value_t* evaluateLvalue(const Subscript&, const Environment* env) {
    TODO();
}

value_t* evaluateLvalue(const Symbol& symbol, const Environment* env) {
    // variable unbound, caught during static analysis
    ASSERT (env->contains(symbol.name));

    auto symbolVal = env->at(symbol.name);
    return std::visit(overload{
        [](const Environment::Variable& var) -> value_t* {return var;},
        [](Environment::LabelToLvalue& /*or PassByRef*/ var) -> value_t* {return var();},

        /* caught during static analysis */
        [](Environment::ConstValue /*or LabelToConst*/) -> value_t* {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToNonConst) -> value_t* {SHOULD_NOT_HAPPEN();},
        [](Environment::PassByDelayed) -> value_t* {SHOULD_NOT_HAPPEN();},
    }, symbolVal);

}
