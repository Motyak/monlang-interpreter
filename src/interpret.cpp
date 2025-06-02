#include <monlang-interpreter/interpret.h>

/* impl only */

#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/str-utils.h>
#include <utils/defer-util.h>

#include <cmath>
#include <vector>

#define unless(x) if(!(x))

thread_local bool INTERACTIVE_MODE = false;
thread_local bool top_level_stmt = true;
thread_local std::vector<Expression> activeCallStack;

using Int = prim_value_t::Int;
using Byte = prim_value_t::Byte;
using Bool = prim_value_t::Bool;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;
namespace LV2 {using Lambda = Lambda;}

void interpretProgram(const Program& prog) {
    Environment env;
    for (auto stmt: prog.statements) {
        performStatement(stmt, &env);
    }
}

void performStatement(const Statement& stmt, Environment* env) {
    std::visit(
        [env](auto* stmt){performStatement(*stmt, env);},
        stmt
    );
}

value_t evaluateValue(const Expression& expr, Environment* env) {
    return std::visit(overload{
        [expr, env](FunctionCall* fnCall){
            bool should_pop = false;
            /*breakable block*/for (int i = 1; i <= 1; ++i)
            {
                if (std::holds_alternative<Symbol*>(fnCall->function)) {
                    auto symbolName = std::get<Symbol*>(fnCall->function)->name;
                    if (!env->contains(symbolName) && !BUILTIN_TABLE.contains(symbolName)) {
                        break;
                    }
                }

                ::activeCallStack.push_back(expr);
                should_pop = true;
            }

            defer {
                if (should_pop) {
                    ::activeCallStack.pop_back();
                }
            };

            return evaluateValue(*fnCall, env);
        },
        [env](Operation* op){
            return evaluateValue(*op, env);
        },
        [env](auto* expr){
            ::activeCallStack.push_back(expr);
            defer {::activeCallStack.pop_back();};
            return evaluateValue(*expr, env);
        },
    }, expr);
}

value_t* evaluateLvalue(const Lvalue& lvalue, Environment* env) {
    return std::visit(
        [lvalue, env](auto* expr){
            ::activeCallStack.push_back(lvalue);
            defer {::activeCallStack.pop_back();};
            return evaluateLvalue(*expr, env);
        },
        lvalue.variant
    );
}

//==============================================================
// performStatement
//==============================================================

void performStatement(const Assignment& assignment, Environment* env) {
    auto new_value = evaluateValue(assignment.value, env);
    if (std::holds_alternative<Symbol*>(assignment.variable.variant)
            && std::get<Symbol*>(assignment.variable.variant)->name == "_") {
        return;
    }
    auto* lvalue = evaluateLvalue(assignment.variable, env);
    auto old_value = *lvalue;
    *lvalue = new_value;
    env->symbolTable["$old"] = Environment::ConstValue{old_value};
}

void performStatement(const Accumulation& acc, Environment* env) {
    auto opPtr = new Symbol{acc.operator_};
    auto lhs = (Expression)acc.variable;
    auto rhs = acc.value;
    auto fnCallPtr = new FunctionCall{(Expression)opPtr, {lhs, rhs}};
    fnCallPtr->_tokenId = acc.operator_._tokenId;

    auto variable = acc.variable;
    auto assignment = Assignment{variable, (Expression)fnCallPtr};

    performStatement(assignment, env);

    delete opPtr;
    delete fnCallPtr;
}

void performStatement(const LetStatement&, const Environment*) {
    TODO();
}

void performStatement(const VarStatement& varStmt, Environment* env) {
    if (varStmt.variable.name == "_") {
        ::activeCallStack.push_back(const_cast<Symbol*>(&varStmt.variable));
        defer {::activeCallStack.pop_back();};
        throw InterpretError("Redefinition of a special name");
    }

    if (env->symbolTable.contains(varStmt.variable.name)) {
        ::activeCallStack.push_back(const_cast<Symbol*>(&varStmt.variable));
        defer {::activeCallStack.pop_back();};
        throw SymbolRedefinitionError(varStmt.variable.name);
    }

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

void performStatement(const DieStatement&, const Environment*) {
    TODO();
}

void performStatement(const ForeachStatement&, const Environment*) {
    TODO();
}

void performStatement(const WhileStatement&, const Environment*) {
    TODO();
}

void performStatement(const DoWhileStatement&, const Environment*) {
    TODO();
}

void performStatement(const ExpressionStatement& exprStmt, Environment* env) {
    auto value = evaluateValue(exprStmt.expression, env);
    if (INTERACTIVE_MODE && ::top_level_stmt) {
        if (!is_nil(value)) {
            builtin::print_({value});
        }
    }
}


//==============================================================
// evaluateValue
//==============================================================

value_t evaluateValue(const Operation& operation, Environment* env) {
    auto opPtr = new Symbol{operation.operator_};
    auto lhs = operation.leftOperand;
    auto rhs = operation.rightOperand;
    auto fnCallPtr = new FunctionCall{(Expression)opPtr, {lhs, rhs}};
    fnCallPtr->_tokenId = operation.operator_._tokenId;

    auto res = evaluateValue((Expression)fnCallPtr, env);

    delete opPtr;
    delete fnCallPtr;

    return res;
}

value_t evaluateValue(const FunctionCall& fnCall, Environment* env) {
    auto fnVal = evaluateValue(fnCall.function, env);
    ASSERT (std::holds_alternative<prim_value_t*>(fnVal)); // TODO: tmp
    auto fnPrimValPtr = std::get<prim_value_t*>(fnVal);
    if (fnPrimValPtr == nullptr) {
        throw InterpretError("Calling a $nil");
    }
    if (!std::holds_alternative<prim_value_t::Lambda>(fnPrimValPtr->variant)) {
        throw InterpretError("Calling a non-Lambda");
    }
    auto function = std::get<prim_value_t::Lambda>(fnPrimValPtr->variant);
    return function(fnCall.arguments, env);
}

value_t evaluateValue(const LV2::Lambda& lambda, Environment* env) {
    for (size_t i = 0; i < lambda.parameters.size(); ++i) {
        if (lambda.parameters[i].name == "_") continue;
        for (size_t j = i + 1; j < lambda.parameters.size(); ++j) {
            if (lambda.parameters[i].name == lambda.parameters[j].name) {
                throw DuplicateParamError(lambda.parameters[j].name);
            }
        }
    }

    Environment* envAtCreation = new Environment{*env};
    auto lambdaVal = prim_value_t::Lambda{
        [envAtCreation, lambda](const std::vector<FunctionCall::Argument>& args, Environment* envAtApp) -> value_t {

            /*
                transform `args` into `flatten_args`.

                each "single" argument will get transformed
                to a <arg, env> pair where env is envAtApp.

                all arguments, whether from a "single" or a "variadic arguments",
                are concatenated together
            */
            std::vector<std::pair<FunctionCall::Argument, Environment*>>
            flatten_args;

            for (auto arg: args) {
                /* handle "variadic arguments" argument */
                /* breakable block */for (int z = 1; z <= 1; ++z)
                {
                    unless (std::holds_alternative<Symbol*>(arg.expr)) break;
                    auto symbol = std::get<Symbol*>(arg.expr);
                    unless (envAtApp->contains(symbol->name)) break;
                    auto symbolVal = envAtApp->at(symbol->name);
                    unless (std::holds_alternative<Environment::VariadicArguments>(symbolVal)) break;
                    auto varargs = std::get<Environment::VariadicArguments>(symbolVal);

                    for (auto [arg, env]: varargs) {
                        flatten_args.push_back({arg, env});
                    }

                    continue;
                }
                
                /* handle "single" argument */
                flatten_args.push_back({arg, envAtApp});
            }

            /*
                create a temporary new environment, based on the captured-one,
                in which we resolve each flatten_args argument with respect with their associated environment,
                and then bind each value to its argument-associated lambda parameter..
            */
            auto parametersBinding = std::map<Environment::SymbolName, Environment::SymbolValue>{};
            if (!lambda.variadicParameters && flatten_args.size() != lambda.parameters.size()) {
                auto lambda_ = lambda; // mutable copy
                ::activeCallStack.push_back(&lambda_);
                throw WrongNbOfArgsError(lambda.parameters, flatten_args);
            }
            
            size_t i = 0;

            /* binding required parameters (<> variadic parameters) */
            for (; i < lambda.parameters.size(); ++i) {
                auto currParam = lambda.parameters.at(i);
                auto [currArg, _] = flatten_args.at(i);
                auto [_, currArgEnv] = flatten_args.at(i);

                if (parametersBinding.contains(currParam.name)
                        && currParam.name != "_") {
                    SHOULD_NOT_HAPPEN(); // BUG: Lambda had two parameters with the same name
                }

                if (currArg.passByRef) {
                    auto currArg_ = Lvalue{currArg.expr};
                    parametersBinding[currParam.name] = Environment::PassByRef{
                        [currArg_, currArgEnv]() -> value_t {
                            return evaluateValue(currArg_, currArgEnv);
                        },
                        [currArg_, currArgEnv]() -> value_t* {
                            return evaluateLvalue(currArg_, currArgEnv);
                        }
                    };
                }
                else {
                    #ifdef TOGGLE_PASS_BY_VALUE
                    auto var = new value_t{evaluateValue(currArg.expr, currArgEnv)}; // TODO: leak
                    parametersBinding[currParam.name] = Environment::Variable{var};
                    #else // lazy passing a.k.a pass by delayed
                    auto* thunkEnv = new Environment{*currArgEnv};
                    auto* delayed = new thunk_with_memoization_t<value_t>{
                        [currArg, thunkEnv]() -> value_t {
                            return evaluateValue(currArg.expr, thunkEnv);
                        }
                    };
                    parametersBinding[currParam.name] = Environment::PassByDelayed{delayed};
                    #endif
                }
            }

            /* binding variadic parameters (if remains args and no error thrown yet) */
            if (i < flatten_args.size()) {
                ASSERT (lambda.variadicParameters); // otherwise BUG (due to earlier checks)
                auto variadicParams = *lambda.variadicParameters;
                auto varargs = Environment::VariadicArguments{
                    flatten_args.begin() + i,
                    flatten_args.end()
                };
                parametersBinding[variadicParams.name] = varargs;
                parametersBinding["$#varargs"] = Environment::ConstValue{new prim_value_t{Int(varargs.size())}};
            }

            auto lambdaEnv = Environment{.symbolTable = parametersBinding, .enclosingEnv = envAtCreation};

            /*

                ..then interpret the lambda's body with respect with this freshly-created
                environment.

                NOTE: lambda's parameters must be in the same environment
                as lambda's body local variables.
                Therefore we can't just call `return evaluateValue(lambda.body, &lambdaEnv);`
            */

            if (lambda.body.statements.size() == 0) {
                return nil_value_t();
            }

            auto backup_top_level_stmt = ::top_level_stmt;
            ::top_level_stmt = false;
            defer {::top_level_stmt = backup_top_level_stmt;};

            i = 0;
            for (; i < lambda.body.statements.size() - 1; ++i) {
                performStatement(lambda.body.statements.at(i), &lambdaEnv);
            }

            if (std::holds_alternative<ExpressionStatement*>(lambda.body.statements.at(i))) {
                auto exprStmt = *std::get<ExpressionStatement*>(lambda.body.statements.at(i));
                return evaluateValue(exprStmt.expression, &lambdaEnv);
            }
            performStatement(lambda.body.statements.at(i), &lambdaEnv);
            return nil_value_t();
        }
    };
    return new prim_value_t{lambdaVal};
}

value_t evaluateValue(const BlockExpression& blockExpr, Environment* env) {
    auto backup_top_level_stmt = ::top_level_stmt;
    ::top_level_stmt = false;
    defer {::top_level_stmt = backup_top_level_stmt;};

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

value_t evaluateValue(const FieldAccess&, const Environment*) {
    TODO();
}

value_t evaluateValue(const Subscript&, const Environment*) {
    TODO();
}

value_t evaluateValue(const ListLiteral&, const Environment*) {
    TODO();
}

value_t evaluateValue(const MapLiteral&, const Environment*) {
    TODO();
}

value_t evaluateValue(const SpecialSymbol& specialSymbol, const Environment* env) {
    static auto Bool_true = prim_value_t(Bool(true));
    static auto Bool_false = prim_value_t(Bool(false));

    if (specialSymbol.name == "$nil") {
        return nil_value_t();
    }

    if (specialSymbol.name == "$true") {
        return &Bool_true;
    }

    if (specialSymbol.name == "$false") {
        return &Bool_false;
    }

    if (!env->contains(specialSymbol.name)) {
        throw InterpretError("Unbound symbol `" + specialSymbol.name + "`");
    }

    auto specialSymbolVal = env->at(specialSymbol.name);
    return std::visit(overload{
        [](Environment::ConstValue const_) -> value_t {return const_;},
        [](Environment::Variable) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToNonConst) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToLvalue /*or PassByRef*/) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::PassByDelayed) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::PassByRef) -> value_t {SHOULD_NOT_HAPPEN();},
        [](Environment::VariadicArguments) -> value_t {SHOULD_NOT_HAPPEN();},
    }, specialSymbolVal);
}

value_t evaluateValue(const Numeral& numeral, const Environment*) {
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

value_t evaluateValue(const StrLiteral& strLiteral, const Environment*) {
    return new prim_value_t(Str(strLiteral.str));
}

value_t evaluateValue(const Symbol& symbol, Environment* env) {
    if (symbol.name == "_") {
        return nil_value_t();
    }

    if (env->contains(symbol.name)) {
        auto symbolVal = env->at(symbol.name);
        return std::visit(overload{
            [](Environment::ConstValue /*or LabelToConst*/ const_) -> value_t {return const_;},
            [](Environment::Variable var) -> value_t {return *var;},
            [](Environment::LabelToNonConst label) -> value_t {return label();},
            [](Environment::LabelToLvalue /*or PassByRef*/ label_ref) -> value_t {return *label_ref();},
            [](Environment::PassByDelayed delayed) -> value_t {return (*delayed)();},
            [](Environment::PassByRef ref) -> value_t {return ref.value();},

            [](Environment::VariadicArguments) -> value_t {SHOULD_NOT_HAPPEN();},
        }, symbolVal);
    }
    else if (BUILTIN_TABLE.contains(symbol.name)) {
        auto builtin_fn = BUILTIN_TABLE.at(symbol.name);
        return new prim_value_t{builtin_fn};
    }
    #ifdef TOGGLE_UNBOUND_SYM_AS_STR
    /*
        useful for checking in-code whether a symbol is unbound or not
        (for testing purposes)
    */
    return new prim_value_t(Str(symbol.name));
    #else
    throw InterpretError("Unbound symbol `" + symbol.name + "`");
    #endif
}


//==============================================================
// evaluateLvalue
//==============================================================

value_t* evaluateLvalue(const FieldAccess&, Environment*) {
    TODO();
}

value_t* evaluateLvalue(const Subscript&, Environment*) {
    TODO();
}

value_t* evaluateLvalue(const Symbol& symbol, Environment* env) {
    static value_t DISPOSABLE_LVALUE;
    if (symbol.name == "_") {
        return &DISPOSABLE_LVALUE;
    }

    if (!env->contains(symbol.name)) {
        throw InterpretError("Unbound symbol `" + symbol.name + "`");
    }

    auto& symbolVal = env->at(symbol.name);
    return std::visit(overload{
        [](const Environment::Variable& var) -> value_t* {return var;},
        [](Environment::LabelToLvalue& /*or PassByRef*/ var) -> value_t* {return var();},
        [&symbolVal](Environment::PassByDelayed&) -> value_t* {
            auto* var = new value_t{};
            // auto* var = new value_t{(*delayed)()};
            symbolVal = Environment::Variable{var};
            return var;
        },
        [](Environment::PassByRef& ref) -> value_t* {return ref.lvalue();},

        /*
            TODO: runtime error: Not an lvalue
              -> when non-lvalue is passed by ref, or assigned a value
                -> at this time, we can't trigger it, we need the let stmt
        */
        [](Environment::ConstValue /*or LabelToConst*/) -> value_t* {SHOULD_NOT_HAPPEN();},
        [](Environment::LabelToNonConst) -> value_t* {SHOULD_NOT_HAPPEN();},
        [](Environment::VariadicArguments) -> value_t* {SHOULD_NOT_HAPPEN();},
    }, symbolVal);

}
