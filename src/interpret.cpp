#include <monlang-interpreter/interpret.h>

/* impl only */

#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin.h>
#include <monlang-interpreter/deepcopy.h>
#include <monlang-interpreter/PathResolution.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/str-utils.h>
#include <utils/defer-util.h>
#include <utils/vec-utils.h>
#include <utils/abs63.h>
#include <utils/min0.h>

#include <cmath>
#include <vector>
#include <csetjmp>
#include <algorithm>

#define unless(x) if(!(x))

/* set by a "main.cpp", otherwise default values */
std::string ARG0;
std::string SRCNAME;
std::vector<std::string> SRC_ARGS;
bool INTERACTIVE_MODE = false;

thread_local std::vector<Expression> activeCallStack;
static bool top_level_stmt = true;
static std::optional<FunctionCall> is_tailcallable;
uint64_t builtin_lambda_id = 0;

/* sentinel value to allow chained autovivification
   , through PassByRef notably */
value_t SENTINEL_NEW_MAP = new prim_value_t();

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;
namespace LV2 {using Lambda = Lambda;}

namespace {
    struct BackupStack {
        jmp_buf jmpBuf;
        std::vector<Expression> activeCallStack;
    };
}

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
        [env](FunctionCall* fnCall){
            // evaluateValue(FunctionCall) handles its own..
            // ..special activeCallStack pushing/poping
            return evaluateValue(*fnCall, env);
        },
        [env](Operation* op){
            // we don't push for (Operation) because..
            // ..its redundant with (FunctionCall)
            return evaluateValue(*op, env);
        },
        [env](auto* expr){
            ::activeCallStack.push_back(expr);
            defer {safe_pop_back(::activeCallStack);};
            return evaluateValue(*expr, env);
        },
    }, expr);
}

value_t* evaluateLvalue(const Lvalue& lvalue, Environment* env, bool subscripted) {
    ::activeCallStack.push_back(lvalue);
    defer {safe_pop_back(::activeCallStack);};
    return std::visit(overload{
        [env, subscripted](Symbol* symbol){
            return evaluateLvalue(*symbol, env, subscripted);
        },
        [env](auto* lvalue){
            return evaluateLvalue(*lvalue, env);
        },
    }, lvalue.variant);
}

//==============================================================
// performStatement
//==============================================================

void performStatement(const Assignment& assignment, Environment* env) {
    auto new_value = evaluateValue(assignment.value, env);
    new_value = deepcopy(new_value);
    if (std::holds_alternative<Symbol*>(assignment.variable.variant)
            && std::get<Symbol*>(assignment.variable.variant)->name == "_") {
        return;
    }
    auto* lvalue = evaluateLvalue(assignment.variable, env);
    ASSERT (lvalue != nullptr);

    /* special case: assign to Str char */
    if (std::holds_alternative<char*>(*lvalue)) {
        auto* c = std::get<char*>(*lvalue);
        ::activeCallStack.push_back(assignment.value);
        defer {safe_pop_back(::activeCallStack);};
        auto newChar = builtin::prim_ctor::Byte_(new_value);
        *c = newChar;
        return;
    }

    *lvalue = new_value;
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

    // we should not delete opPtr or fnCallPtr
}

void performStatement(const LetStatement& letStmt, Environment* env) {
    if (letStmt.alias.name == "_") {
        ::activeCallStack.push_back(const_cast<Symbol*>(&letStmt.alias));
        throw InterpretError("Redefinition of a special name");
    }

    if (env->symbolTable.contains(letStmt.alias.name)) {
        ::activeCallStack.push_back(const_cast<Symbol*>(&letStmt.alias));
        throw SymbolRedefinitionError(letStmt.alias.name);
    }

    auto leftmostSymbol = leftmost(letStmt.variable);
    if (!env->contains(leftmostSymbol.name)) {
        ::activeCallStack.push_back(letStmt.variable);
        throw InterpretError("Unbound symbol `" + leftmostSymbol.name + "`");
    }

    #ifndef TOGGLE_LEGACY_REF
    if (containsAnySubscript(letStmt.variable)) {
        auto* pathResolution = new PathResolution{letStmt.variable, env->rec_deepcopy()};
        auto* thunkEnv = new Environment{*env}; // no need for rec_copy() here ?
        env->symbolTable[letStmt.alias.name] = Environment::LabelToLvalue{
            (thunk_t<value_t>)[pathResolution, thunkEnv]() -> value_t {
                return pathResolution->value(thunkEnv);
            },
            [pathResolution, thunkEnv](bool subscripted) -> value_t* {
                if (subscripted) {
                    pathResolution->createPaths(thunkEnv);
                }
                return pathResolution->lvalue(thunkEnv);
            }
        };
    }
    else
    #endif
    {
        auto* thunkEnv = new Environment{*env}; // no need for rec_copy() here ?
        env->symbolTable[letStmt.alias.name] = Environment::LabelToLvalue{
            (thunk_t<value_t>)[&letStmt, thunkEnv]() -> value_t {
                return evaluateValue(letStmt.variable, thunkEnv);
            },
            [&letStmt, thunkEnv](bool subscripted) -> value_t* {
                if (subscripted) {
                    createPaths(letStmt.variable, thunkEnv);
                }
                return evaluateLvalue(letStmt.variable, thunkEnv);
            }
        };
    }
}

void performStatement(const VarStatement& varStmt, Environment* env) {
    if (varStmt.variable.name == "_") {
        ::activeCallStack.push_back(const_cast<Symbol*>(&varStmt.variable));
        throw InterpretError("Redefinition of a special name");
    }

    if (env->symbolTable.contains(varStmt.variable.name)) {
        ::activeCallStack.push_back(const_cast<Symbol*>(&varStmt.variable));
        throw SymbolRedefinitionError(varStmt.variable.name);
    }

    auto value = evaluateValue(varStmt.value, env);
    value = deepcopy(value);
    auto* var = new value_t(value);
    env->symbolTable[varStmt.variable.name] = Environment::Variable{var};
}

void performStatement(const ReturnStatement&, const Environment*) {
    TODO();
    // SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
}

void performStatement(const BreakStatement&, const Environment*) {
    TODO();
    // SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
}

void performStatement(const ContinueStatement&, const Environment*) {
    TODO();
    // SHOULD_NOT_HAPPEN(); // not a top-level statement, caught during static analysis
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

void performStatement(const NullStatement&, const Environment*) {
    ;
}

void performStatement(const ExpressionStatement& exprStmt, Environment* env) {
    value_t value = nil_value_t();
    if (exprStmt.expression) {
        value = evaluateValue(*exprStmt.expression, env);
    }
    if (INTERACTIVE_MODE && top_level_stmt) {
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
    for (auto operand: {lhs, rhs}) {
        if (std::holds_alternative<Symbol*>(operand)) {
            auto symbol = std::get<Symbol*>(operand);
            if (env->contains(symbol->name)) {
                auto symbolVal = env->at(symbol->name);
                if (std::holds_alternative<Environment::VariadicArguments>(symbolVal)) {
                    ::activeCallStack.push_back(operand);
                    throw InterpretError("Cannot refer to variadic arguments as symbol");
                }
            }
        }
    }
    
    auto fnCallPtr = new FunctionCall{(Expression)opPtr, {lhs, rhs}};
    fnCallPtr->_tokenId = operation.operator_._tokenId;

    auto res = evaluateValue((Expression)fnCallPtr, env);
    // res = deepcopy(res); // TODO: no need ?

    // we should not delete opPtr or fnCallPtr

    return res;
}

value_t evaluateValue(const FunctionCall& fnCall, Environment* env) {
    auto fnVal = evaluateValue(fnCall.function, env);
    ASSERT (std::holds_alternative<prim_value_t*>(fnVal)); // TODO: tmp
    auto* fnPrimValPtr = std::get<prim_value_t*>(fnVal);
    if (fnPrimValPtr == nullptr) {
        ::activeCallStack.push_back(fnCall.function);
        throw InterpretError("Calling a $nil");
    }
    if (!std::holds_alternative<prim_value_t::Lambda>(fnPrimValPtr->variant)) {
        ::activeCallStack.push_back(fnCall.function);
        throw InterpretError("Calling a non-Lambda");
    }

    /*
        transform `args` into `flattenArgs`.

        each "single" argument will get transformed
        to a <arg, env> pair where env is envAtApp.

        all arguments, whether from a "single" or a "variadic arguments",
        are concatenated together
    */
    std::vector<FlattenArg> flattenArgs;
    for (auto arg: fnCall.arguments) {
        /* handle "variadic arguments" argument */
        /* breakable block */for (int z = 1; z <= 1; ++z)
        {
            unless (std::holds_alternative<Symbol*>(arg.expr)) break;
            auto symbol = std::get<Symbol*>(arg.expr);
            if (symbol->name == "_" && arg.passByRef) {
                ::activeCallStack.push_back(arg.expr);
                throw InterpretError("Can't pass special name by ref");
            }
            unless (env->contains(symbol->name)) break;
            auto symbolVal = env->at(symbol->name);
            unless (std::holds_alternative<Environment::VariadicArguments>(symbolVal)) break;
            if (arg.passByRef) {
                ::activeCallStack.push_back(arg.expr);
                throw InterpretError("Can't pass variadic arguments by ref");
            }
            auto varargs = std::get<Environment::VariadicArguments>(symbolVal);

            flattenArgs.insert(flattenArgs.end(), varargs.begin(), varargs.end());

            goto CONTINUE;
        }
        
        /* handle "single" argument */
        flattenArgs.push_back({arg, env});

        CONTINUE:
    }

    const auto& function = std::get<prim_value_t::Lambda>(fnPrimValPtr->variant);

    /* recursive tail call elimination */
    static auto savedCalledFns = std::map<uint64_t, BackupStack>{};
    if (savedCalledFns.contains(function.id)) {
        if (is_tailcallable && fnCall.arguments.size() == 0 && is_tailcallable->function == fnCall.function) {
            ::activeCallStack = savedCalledFns.at(function.id).activeCallStack;
            longjmp(savedCalledFns.at(function.id).jmpBuf, 1);
        }
    }
    else if (fnCall.arguments.size() == 0) {
        // map auto-vivification creates a default jmpBuf here
        savedCalledFns[function.id].activeCallStack = ::activeCallStack;
        if (setjmp(savedCalledFns.at(function.id).jmpBuf)) {
            ;
        }
    }

    // we only want to push the FunctionCall on the activeCallStack..
    // ..once we reach the function's body execution
    ::activeCallStack.push_back(const_cast<FunctionCall*>(&fnCall));
    auto res = function.stdfunc(flattenArgs);
    safe_pop_back(::activeCallStack);
    savedCalledFns.erase(function.id);
    return res;
}

static std::optional<FunctionCall> check_if_tailcallable(const Statement& stmt) {
    #ifdef TOGGLE_TAILCALL
    unless (std::holds_alternative<Assignment*>(stmt)) return {};
    auto* assignment = std::get<Assignment*>(stmt);
    unless (std::holds_alternative<Symbol*>(assignment->variable.variant)) return {};
    auto* varSymbol = std::get<Symbol*>(assignment->variable.variant);
    if (varSymbol->name == "_") {
        unless (std::holds_alternative<FunctionCall*>(assignment->value)) return {};
        return *std::get<FunctionCall*>(assignment->value);
    }
    #endif
    (void)stmt;
    return {};
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

    Environment* envAtCreation = env->rec_copy();
    static uint64_t lambda_id = 1000;
    ASSERT (lambda_id > builtin_lambda_id);
    ASSERT (lambda_id != uint64_t(-1));
    auto lambdaVal = prim_value_t::Lambda{
        lambda_id++,
        new prim_value_t{Int(lambda.parameters.size())},
        [envAtCreation, lambda](const std::vector<FlattenArg>& flattenArgs) -> value_t {
            /*
                create a temporary new environment, based on the captured-one,
                in which we resolve each flatten_args argument with respect with their associated environment,
                and then bind each value to its argument-associated lambda parameter..
            */
            auto parametersBinding = std::map<Environment::SymbolName, Environment::SymbolValue>{};
            if (!lambda.variadicParameter && flattenArgs.size() != lambda.parameters.size()) {
                ::activeCallStack.push_back(const_cast<LV2::Lambda*>(&lambda));
                throw WrongNbOfArgsError(lambda.parameters, flattenArgs);
            }
            if (lambda.variadicParameter && flattenArgs.size() < lambda.parameters.size()) {
                ::activeCallStack.push_back(const_cast<LV2::Lambda*>(&lambda));
                throw WrongNbOfArgsError(lambda.parameters, flattenArgs);
            }
            
            size_t i = 0;

            /* binding required parameters (<> variadic parameter) */
            for (; i < lambda.parameters.size(); ++i) {
                auto currParam = lambda.parameters.at(i);
                auto currArg = flattenArgs.at(i);

                if (parametersBinding.contains(currParam.name)
                        && currParam.name != "_") {
                    SHOULD_NOT_HAPPEN(); // then its a bug, Lambda had two parameters with the same name
                                         // and it wasn't caught earlier
                }

                if (currArg.passByRef) {
                    #ifndef TOGGLE_LEGACY_REF
                    // if Argument::passByRef is set, then we know Argument::expr is an lvalue
                    if (containsAnySubscript(/*Lvalue*/currArg.expr)) {
                        auto* pathResolution = new PathResolution{currArg.expr, currArg.env->rec_deepcopy()};
                        auto* thunkEnv = currArg.env->rec_copy();
                        parametersBinding[currParam.name] = Environment::PassByRef{
                            (thunk_t<value_t>)[pathResolution, thunkEnv]() -> value_t {
                                return pathResolution->value(thunkEnv);
                            },
                            [pathResolution, thunkEnv](bool subscripted) -> value_t* {
                                if (subscripted) {
                                    pathResolution->createPaths(thunkEnv);
                                }
                                return pathResolution->lvalue(thunkEnv);
                            }
                        };
                    }
                    else
                    #endif
                    {
                        auto* thunkEnv = currArg.env->rec_copy();
                        parametersBinding[currParam.name] = Environment::PassByRef{
                            (thunk_t<value_t>)[currArg, thunkEnv]() -> value_t {
                                return evaluateValue(currArg.expr, thunkEnv);
                            },
                            [currArg, thunkEnv](bool subscripted) -> value_t* {
                                if (subscripted) {
                                    #ifdef TOGGLE_LEGACY_REF
                                    createPaths(currArg.expr, thunkEnv);
                                    #else
                                    evaluateValue(currArg.expr, thunkEnv);
                                    #endif
                                }
                                return evaluateLvalue(currArg.expr, thunkEnv);
                            }
                        };
                    }
                }
                else {
                    #ifdef TOGGLE_PASS_BY_VALUE
                    auto var = new value_t{evaluateValue(currArg.expr, currArg.env)};
                    *var = deepcopy(*var);
                    parametersBinding[currParam.name] = Environment::Variable{var};
                    #else // lazy passing a.k.a pass by delayed
                    if (currArg.varargsPassByDelay) {
                        auto** _any = std::any_cast<Environment::PassByDelay_Variant**>(*currArg.varargsPassByDelay);
                        ASSERT (_any != nullptr);
                        auto& passByDelayVariantPtr = *_any;
                        if (passByDelayVariantPtr == nullptr) {
                            auto* thunkEnv = currArg.env->rec_copy();
                            auto* delayed = new thunk_with_memoization_t<value_t>{
                                [currArg, thunkEnv]() -> value_t {
                                    auto res = evaluateValue(currArg.expr, thunkEnv);
                                    res = deepcopy(res);
                                    return res;
                                }
                            };
                            passByDelayVariantPtr = new Environment::PassByDelay_Variant{delayed};
                        }
                        parametersBinding[currParam.name] = passByDelayVariantPtr;
                    }
                    else {
                        auto* thunkEnv = currArg.env->rec_copy();
                        auto* delayed = new thunk_with_memoization_t<value_t>{
                            [currArg, thunkEnv]() -> value_t {
                                auto res = evaluateValue(currArg.expr, thunkEnv);
                                res = deepcopy(res);
                                return res;
                            }
                        };
                        parametersBinding[currParam.name] = new Environment::PassByDelay_Variant{delayed};
                    }
                    #endif
                }
            }

            /* binding variadic parameter */
            if (lambda.variadicParameter) {
                auto varargs = Environment::VariadicArguments{};
                for (; i < flattenArgs.size(); ++i) {
                    auto currArg = flattenArgs.at(i);
                    if (!currArg.passByRef && !currArg.varargsPassByDelay) {
                        currArg.varargsPassByDelay = {std::make_any<Environment::PassByDelay_Variant**>(new Environment::PassByDelay_Variant*{nullptr})};
                    }
                    varargs.push_back(currArg);
                }
                parametersBinding[lambda.variadicParameter->name] = varargs;
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

            auto backup_top_level_stmt = top_level_stmt;
            top_level_stmt = false;
            defer {top_level_stmt = backup_top_level_stmt;};

            std::optional<Statement> trailing_stmt;
            for (i = 0; i < lambda.body.statements.size(); ++i) {
                if (!is_empty_expr_stmt(lambda.body.statements.at(i))) {
                    if (trailing_stmt) {
                        performStatement(*trailing_stmt, &lambdaEnv);
                    }
                    trailing_stmt = lambda.body.statements.at(i);
                }
            }

            unless (trailing_stmt) return nil_value_t();

            auto backup_is_tailcallable = is_tailcallable;
            is_tailcallable = check_if_tailcallable(*trailing_stmt);
            defer {is_tailcallable = backup_is_tailcallable;};

            if (!std::holds_alternative<ExpressionStatement*>(*trailing_stmt)) {
                performStatement(*trailing_stmt, &lambdaEnv);
                return nil_value_t();
            }

            auto exprStmt = *std::get<ExpressionStatement*>(*trailing_stmt);
            ASSERT (exprStmt.expression);
            auto res = evaluateValue(*exprStmt.expression, &lambdaEnv);
            // res = deepcopy(res); // TODO: no need ?
            return res;
        }
    };
    return new prim_value_t{lambdaVal};
}

value_t evaluateValue(const BlockExpression& blockExpr, Environment* env) {
    auto backup_top_level_stmt = top_level_stmt;
    top_level_stmt = false;
    defer {top_level_stmt = backup_top_level_stmt;};

    if (blockExpr.statements.size() == 0) {
        return nil_value_t();
    }
    auto newEnv = new Environment{{}, env};

    std::optional<Statement> trailing_stmt;
    for (size_t i = 0; i < blockExpr.statements.size(); ++i) {
        if (!is_empty_expr_stmt(blockExpr.statements.at(i))) {
            if (trailing_stmt) {
                performStatement(*trailing_stmt, newEnv);
            }
            trailing_stmt = blockExpr.statements.at(i);
        }
    }

    unless (trailing_stmt) return nil_value_t();

    auto backup_is_tailcallable = is_tailcallable;
    is_tailcallable = check_if_tailcallable(*trailing_stmt);
    defer {is_tailcallable = backup_is_tailcallable;};

    if (!std::holds_alternative<ExpressionStatement*>(*trailing_stmt)) {
        performStatement(*trailing_stmt, newEnv);
        return nil_value_t();
    }

    auto exprStmt = *std::get<ExpressionStatement*>(*trailing_stmt);
    ASSERT (exprStmt.expression);
    auto res = evaluateValue(*exprStmt.expression, newEnv);
    // res = deepcopy(res); // TODO: no need ?
    return res;
}

value_t evaluateValue(const FieldAccess& fieldAccess, Environment* env) {
    auto object = evaluateValue(fieldAccess.object, env);
    ASSERT (std::holds_alternative<prim_value_t*>(object)); // TODO: tmp
    auto* objPrimValPtr = std::get<prim_value_t*>(object);

    if (objPrimValPtr == nullptr) {
        throw InterpretError("Accessing field on a $nil");
    }

    if (std::holds_alternative<Map>(objPrimValPtr->variant)) {
        const auto& map = std::get<Map>(objPrimValPtr->variant);
        auto key = new prim_value_t{(Str)fieldAccess.field.name};
        unless (map.contains(key)) {
            ::activeCallStack.push_back(const_cast<Symbol*>(&fieldAccess.field));
            throw InterpretError("Field not found `" + fieldAccess.field.name + "`");
        }
        return map.at(key);
    }

    else throw InterpretError("Accessing field on a non-struct");
}

value_t evaluateValue(const Subscript& subscript, Environment* env) {
    auto arrVal = evaluateValue(subscript.array, env);
    // arrVal = deepcopy(arrVal); // TODO: no need ?
    ASSERT (std::holds_alternative<prim_value_t*>(arrVal)); // TODO: tmp
    auto* arrPrimValPtr = std::get<prim_value_t*>(arrVal);
    if (arrPrimValPtr == nullptr) {
        throw InterpretError("Subscripting a $nil");
    }

    return std::visit(overload{
        [&subscript, env](const Str& str) -> value_t {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a Str with a key");
            }

            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                auto nthVal = evaluateValue(variant_cast(index.nth), env);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs63(intVal) <= abs63(str.size())) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? str.size() - abs63(intVal) : size_t(intVal) - 1;
                return new prim_value_t{Byte(str.at(pos))};
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);
                abs63(str.size()); // make sure it's storable in a Int

                /* from */
                ::activeCallStack.push_back(variant_cast(range.from));
                auto fromVal = evaluateValue(variant_cast(range.from), env);
                auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                safe_pop_back(::activeCallStack);

                /* to */
                ::activeCallStack.push_back(variant_cast(range.to));
                auto toVal = evaluateValue(variant_cast(range.to), env);
                auto intToVal = builtin::prim_ctor::Int_(toVal);
                unless (intToVal != 0 || range.exclusive) throw InterpretError("Subscript range 'from' is zero");
                safe_pop_back(::activeCallStack);

                /* 1) handle exlusive range, if present */
                if (range.exclusive) {
                    Int fromPos = intFromVal < 0? str.size() - abs63(intFromVal) : intFromVal - 1;
                    Int toPos = intToVal < 0? str.size() - abs63(intToVal) : intToVal - 1;
                    if (intFromVal == intToVal) {
                        return new prim_value_t{Str()}; // empty range
                    }
                    else if (fromPos < toPos) {
                        intToVal -= 1;
                    }
                    else if (fromPos > toPos) {
                        intToVal += 1;
                    }
                }

                /* 2) transform to pos, with index starting at zero */
                Int fromPos = intFromVal < 0? str.size() - abs63(intFromVal) : intFromVal - 1;
                Int toPos = intToVal < 0? str.size() - abs63(intToVal) : intToVal - 1;

                /* 3) check out of bounds */
                unless (0 <= fromPos && size_t(fromPos) < str.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'from' is out of bounds");
                }
                unless (0 <= toPos && size_t(toPos) < str.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'to' is out of bounds");
                }

                return new prim_value_t{str.substr(fromPos, min0(toPos - fromPos) + 1)};
            }

            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, env](const List& list) -> value_t {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a List with a key");
            }

            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                auto nthVal = evaluateValue(variant_cast(index.nth), env);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs63(intVal) <= abs63(list.size())) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? list.size() - abs63(intVal) : size_t(intVal) - 1;
                return list.at(pos);
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);
                abs63(list.size()); // make sure it's storable in a Int

                /* from */
                ::activeCallStack.push_back(variant_cast(range.from));
                auto fromVal = evaluateValue(variant_cast(range.from), env);
                auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                safe_pop_back(::activeCallStack);

                /* to */
                ::activeCallStack.push_back(variant_cast(range.to));
                auto toVal = evaluateValue(variant_cast(range.to), env);
                auto intToVal = builtin::prim_ctor::Int_(toVal);
                unless (intToVal != 0 || range.exclusive) throw InterpretError("Subscript range 'from' is zero");
                safe_pop_back(::activeCallStack);

                /* 1) handle exlusive range, if present */
                if (range.exclusive) {
                    Int fromPos = intFromVal < 0? list.size() - abs63(intFromVal) : intFromVal - 1;
                    Int toPos = intToVal < 0? list.size() - abs63(intToVal) : intToVal - 1;
                    if (intFromVal == intToVal) {
                        return new prim_value_t{List()}; // empty range
                    }
                    else if (fromPos < toPos) {
                        intToVal -= 1;
                    }
                    else if (fromPos > toPos) {
                        intToVal += 1;
                    }
                }

                /* 2) transform to pos, with index starting at zero */
                Int fromPos = intFromVal < 0? list.size() - abs63(intFromVal) : intFromVal - 1;
                Int toPos = intToVal < 0? list.size() - abs63(intToVal) : intToVal - 1;

                /* 3) check out of bounds */
                unless (0 <= fromPos && size_t(fromPos) < list.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'from' is out of bounds");
                }
                unless (0 <= toPos && size_t(toPos) < list.size()) {
                    ::activeCallStack.push_back(variant_cast(range.from));
                    throw InterpretError("Subscript range 'to' is out of bounds");
                }

                auto res = List(list.begin() + fromPos, list.begin() + std::max(fromPos, toPos) + 1);
                return new prim_value_t{res};
            }

            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, env](const Map& map) -> value_t {
            if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an index");
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an range");
            }

            else if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                auto key = std::get<Subscript::Key>(subscript.argument);
                auto keyVal = evaluateValue(key.expr, env);
                if (subscript.suffix == '?') {
                    return map.contains(keyVal)? BoolConst::TRUE : BoolConst::FALSE;
                }
                ::activeCallStack.push_back(key.expr);
                unless (map.contains(keyVal)) throw InterpretError("Subscript key not found");
                safe_pop_back(::activeCallStack);
                return map.at(keyVal);
            }

            else SHOULD_NOT_HAPPEN();
        },

        [](Bool) -> value_t {throw InterpretError("Cannot subscript a Bool");},
        [](Byte) -> value_t {throw InterpretError("Cannot subscript a Byte");},
        [](Int) -> value_t {throw InterpretError("Cannot subscript an Int");},
        [](Float) -> value_t {throw InterpretError("Cannot subscript a Float");},
        [](const prim_value_t::Lambda&) -> value_t {throw InterpretError("Cannot subscript a Lambda");},
    }, arrPrimValPtr->variant);
}

value_t evaluateValue(const ListLiteral& listLiteral, Environment* env) {
    List res;
    res.reserve(listLiteral.arguments.size());
    for (auto arg: listLiteral.arguments) {
        auto currArgVal = evaluateValue(arg, env);
        res.push_back(currArgVal);
    }
    return new prim_value_t{res};
}

value_t evaluateValue(const MapLiteral& mapLiteral, Environment* env) {
    Map res;
    for (auto [key, val]: mapLiteral.arguments) {
        auto currKeyVal = evaluateValue(key, env);
        auto currValVal = evaluateValue(val, env);
        res[currKeyVal] = currValVal;
    }
    return new prim_value_t{res};
}

static value_t init_ARGS() {
    List strs;
    for (Str arg: SRC_ARGS) {
        strs.push_back(new prim_value_t{arg});
    }
    return new prim_value_t{strs};
}

value_t evaluateValue(const SpecialSymbol& specialSymbol, const Environment* env) {
    static const value_t ARG0 = new prim_value_t{(Str)::ARG0};
    static const value_t SRCNAME = new prim_value_t{(Str)::SRCNAME};
    static const value_t ARGS = init_ARGS();

    if (specialSymbol.name == "$nil") {
        return nil_value_t();
    }

    if (specialSymbol.name == "$true") {
        return BoolConst::TRUE;
    }

    if (specialSymbol.name == "$false") {
        return BoolConst::FALSE;
    }

    if (specialSymbol.name == "$arg0") {
        return ARG0;
    }

    if (specialSymbol.name == "$srcname") {
        return SRCNAME;
    }

    if (specialSymbol.name == "$args") {
        return ARGS;
    }

    if (env->contains(specialSymbol.name)) {
        auto specialSymbolVal = env->at(specialSymbol.name);
        return std::visit(overload{
            [](const Environment::ConstValue& const_) -> value_t {return const_;},
            [](Environment::Variable) -> value_t {SHOULD_NOT_HAPPEN();},
            [](const Environment::LabelToLvalue& /*or PassByRef*/) -> value_t {SHOULD_NOT_HAPPEN();},
            [](const Environment::PassByDelay&) -> value_t {SHOULD_NOT_HAPPEN();},
            [](const Environment::VariadicArguments&) -> value_t {SHOULD_NOT_HAPPEN();},
        }, specialSymbolVal);
    }

    else if (BUILTIN_TABLE.contains(specialSymbol.name)) {
        return BUILTIN_TABLE.at(specialSymbol.name);
    }

    throw InterpretError("Unbound symbol `" + specialSymbol.name + "`");
}

static long long str2llong(const std::string& str, int base = 10) {
    try {
        return std::stoll(str, nullptr, base);
    }
    catch (const std::out_of_range&) {
        throw InterpretError("Numeral out of range");
    }
}

value_t evaluateValue(const Numeral& numeral, const Environment*) {
    if (numeral.type == "int") {
        return new prim_value_t(Int(str2llong(numeral.int1)));
    }

    if (numeral.type == "hex") {
        return new prim_value_t(Int(str2llong(numeral.int1, 16)));
    }
    
    if (numeral.type == "bin") {
        return new prim_value_t(Int(str2llong(numeral.int1, 2)));
    }

    if (numeral.type == "oct") {
        return new prim_value_t(Int(str2llong(numeral.int1, 8)));
    }

    if (numeral.type == "frac") {
        auto numerator = str2llong(numeral.int1);
        auto denominator = str2llong(numeral.int2);
        auto division = (double)numerator / denominator;
        return new prim_value_t(Float(division));
    }

    if (numeral.type == "pow") {
        auto base = str2llong(numeral.int1);
        auto exponent = str2llong(numeral.int2);
        auto power = std::pow(base, exponent);
        return new prim_value_t(Int(power));
    }

    if (numeral.type == "fix_only") {
        auto int_part = str2llong(numeral.int1);
        auto numerator = str2llong(numeral.fixed);
        auto denominator = std::pow(10, numeral.fixed.size());
        auto division = (double)numerator / denominator;
        auto sum = int_part + (int_part < 0? -division : division);
        if (numeral.int1.starts_with("-") && int_part == 0) sum = -sum;
        return new prim_value_t(Float(sum));
    }

    if (numeral.type == "per_only") {
        auto int_part = str2llong(numeral.int1);
        auto numerator = str2llong(numeral.periodic);
        auto denominator = std::pow(10, numeral.periodic.size()) - 1;
        auto division = (double)numerator / denominator;
        auto sum = int_part + (int_part < 0? -division : division);
        if (numeral.int1.starts_with("-") && int_part == 0) sum = -sum;
        return new prim_value_t(Float(sum));
    }

    if (numeral.type == "fix_and_per") {
        auto int_part = str2llong(numeral.int1);
        auto fixed_part_numerator = str2llong(numeral.fixed);
        auto fixed_part_denominator = std::pow(10, numeral.fixed.size());
        auto fixed_part_division = (double)fixed_part_numerator / fixed_part_denominator;
        auto periodic_part_numerator = str2llong(numeral.periodic);
        auto periodic_part_denominator = (std::pow(10, numeral.periodic.size()) - 1) * fixed_part_denominator;
        auto periodic_part_division = (double)periodic_part_numerator / periodic_part_denominator;
        auto sum = int_part < 0? int_part - fixed_part_division - periodic_part_division
                : int_part + fixed_part_division + periodic_part_division;
        if (numeral.int1.starts_with("-") && int_part == 0) sum = -sum;
        return new prim_value_t(Float(sum));
    }

    SHOULD_NOT_HAPPEN(); // BUG unknown numeral type
}

value_t evaluateValue(const StrLiteral& strLiteral, const Environment*) {
    return new prim_value_t((Str)strLiteral.str);
}

value_t evaluateValue(const Symbol& symbol, const Environment* env) {
    if (symbol.name == "_") {
        return nil_value_t();
    }

    if (env->contains(symbol.name)) {
        auto symbolVal = env->at(symbol.name);
        return std::visit(overload{
            [](const Environment::ConstValue&) -> value_t {SHOULD_NOT_HAPPEN();}, // special symbols exclusively
            [](Environment::Variable var) -> value_t {return *var;},
            [](Environment::LabelToLvalue& label_ref /*or PassByRef*/) -> value_t {return label_ref.value();},
            [](const Environment::PassByDelay& delayed) -> value_t {
                if (std::holds_alternative<thunk_with_memoization_t<value_t>*>(*delayed)) {
                    auto* thunk = std::get<thunk_with_memoization_t<value_t>*>(*delayed);
                    return (*thunk)(); // now memoized for all tracking references
                }

                else if (std::holds_alternative<value_t*>(*delayed)) {
                    auto var = std::get<value_t*>(*delayed);
                    return *var;
                }

                else SHOULD_NOT_HAPPEN();
            },
            [](const Environment::VariadicArguments&) -> value_t {
                throw InterpretError("Cannot refer to variadic arguments as symbol");
            },
        }, symbolVal);
    }
    else if (BUILTIN_TABLE.contains(symbol.name)) {
        return BUILTIN_TABLE.at(symbol.name);
    }
    #ifdef TOGGLE_UNBOUND_SYM_AS_STR
    /*
        useful for checking in-code whether a symbol is unbound or not
        (for testing purposes)
    */
    return new prim_value_t((Str)symbol.name);
    #else
    throw InterpretError("Unbound symbol `" + symbol.name + "`");
    #endif
}


//==============================================================
// evaluateLvalue
//==============================================================

value_t* evaluateLvalue(const FieldAccess& fieldAccess, Environment* env) {
    auto* lvalue = evaluateLvalue(fieldAccess.object, env, /*subscripted*/true);
    ASSERT (lvalue != nullptr);

    ASSERT (std::holds_alternative<prim_value_t*>(*lvalue)); // TODO: tmp
    auto* lvaluePrimValPtr = std::get<prim_value_t*>(*lvalue);

    if (lvaluePrimValPtr == nullptr) {
        throw InterpretError("Accessing field on a $nil");
    }

    if (std::holds_alternative<Map>(lvaluePrimValPtr->variant)) {
        auto& map = std::get<Map>(lvaluePrimValPtr->variant);
        auto key = new prim_value_t{(Str)fieldAccess.field.name};
        unless (map.contains(key)) {
            ::activeCallStack.push_back(const_cast<Symbol*>(&fieldAccess.field));
            throw InterpretError("Field not found `" + fieldAccess.field.name + "`");
        }
        return &map[key];
    }

    else throw InterpretError("Accessing field on a non-struct");
}

value_t* evaluateLvalue(const Subscript& subscript, Environment* env) {
    if (subscript.suffix == '?') {
        throw InterpretError("lvaluing a subscript[]?");
    }
    auto* lvalue = evaluateLvalue(subscript.array, env, /*subscripted*/true);
    ASSERT (lvalue != nullptr);
    ASSERT (std::holds_alternative<prim_value_t*>(*lvalue)); // TODO: tmp
    auto& lvaluePrimValPtr = std::get<prim_value_t*>(*lvalue);

    if (*lvalue == SENTINEL_NEW_MAP) {
        lvaluePrimValPtr = new prim_value_t{prim_value_t::Map()};
    }

    if (lvaluePrimValPtr == nullptr) {
        throw InterpretError("lvaluing a $nil subscript array");
    }

    if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
        throw InterpretError("lvaluing a subscript range");
    }

    return std::visit(overload{
        [&subscript, env](Str& str) -> value_t* {

            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a Str with a key");
            }
            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                auto nthVal = evaluateValue(variant_cast(index.nth), env);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs63(intVal) <= abs63(str.size())) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? str.size() - abs63(intVal) : size_t(intVal) - 1;

                return new value_t{&str[pos]};
            }
            else SHOULD_NOT_HAPPEN();
        },
        [&subscript, env](List& list) -> value_t* {
            if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                throw InterpretError("Subscripting a List with a key");
            }
            else if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                auto index = std::get<Subscript::Index>(subscript.argument);
                auto nthVal = evaluateValue(variant_cast(index.nth), env);
                ::activeCallStack.push_back(variant_cast(index.nth));
                defer {safe_pop_back(::activeCallStack);};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs63(intVal) <= abs63(list.size())) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? list.size() - abs63(intVal) : size_t(intVal) - 1;

                return &list[pos];
            }
            else SHOULD_NOT_HAPPEN();
        },

        [&subscript, env](Map& map) -> value_t* {
            if (std::holds_alternative<Subscript::Index>(subscript.argument)) {
                throw InterpretError("Subscripting a Map with an index");
            }
            else if (std::holds_alternative<Subscript::Key>(subscript.argument)) {
                auto key = std::get<Subscript::Key>(subscript.argument);
                auto keyVal = evaluateValue(key.expr, env);
                if (!map.contains(keyVal)) {
                    if (subscript.suffix == '!') {
                        ::activeCallStack.push_back(key.expr);
                        throw InterpretError("Subscript key not found");
                    }
                    map[keyVal] = SENTINEL_NEW_MAP;
                }
                return &map[keyVal];
            }
            else SHOULD_NOT_HAPPEN();
        },

        [](Bool&) -> value_t* {throw InterpretError("Cannot subscript a Bool");},
        [](Byte&) -> value_t* {throw InterpretError("Cannot subscript a Byte");},
        [](Int&) -> value_t* {throw InterpretError("Cannot subscript an Int");},
        [](Float&) -> value_t* {throw InterpretError("Cannot subscript a Float");},
        [](prim_value_t::Lambda&) -> value_t* {throw InterpretError("Cannot subscript a Lambda");},
    }, lvaluePrimValPtr->variant);
}

value_t* evaluateLvalue(const Symbol& symbol, const Environment* env, bool subscripted) {
    static value_t DISPOSABLE_LVALUE;
    if (symbol.name == "_") {
        return &DISPOSABLE_LVALUE;
    }

    if (env->contains(symbol.name)) {
        auto symbolVal = env->at(symbol.name);
        return std::visit(overload{
            [](Environment::Variable var) -> value_t* {return var;},
            [subscripted](Environment::LabelToLvalue& label_ref) -> value_t* {return label_ref.lvalue(subscripted);},
            [subscripted](const Environment::PassByDelay& delayed) -> value_t* {
                if (std::holds_alternative<thunk_with_memoization_t<value_t>*>(*delayed)) {
                    auto* thunk = std::get<thunk_with_memoization_t<value_t>*>(*delayed);
                    if (thunk->memoized || subscripted) {
                        /* init var with evaluated value */
                        auto initVal = (*thunk)();
                        *delayed = new value_t(initVal);
                    }
                    else {
                        *delayed = new value_t(); // discard unevaluated value
                    }
                }

                else if (std::holds_alternative<value_t*>(*delayed)) {
                    ; // nothing to do
                }

                else SHOULD_NOT_HAPPEN();

                return std::get<value_t*>(*delayed);
            },

            [](Environment::ConstValue) -> value_t* {SHOULD_NOT_HAPPEN();},
            [](const Environment::VariadicArguments&) -> value_t* {
                throw InterpretError("Cannot refer to variadic arguments as symbol");
            },
        }, symbolVal);
    }

    else if (BUILTIN_TABLE.contains(symbol.name)) {
        throw InterpretError("Cannot modify the builtins directly");
    }

    else {
        throw InterpretError("Unbound symbol `" + symbol.name + "`");
    }
}
