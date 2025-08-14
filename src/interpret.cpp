#include <monlang-interpreter/interpret.h>

/* impl only */

#include <monlang-interpreter/InterpretError.h>
#include <monlang-interpreter/builtin.h>
#include <monlang-interpreter/deepcopy.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>
#include <utils/str-utils.h>
#include <utils/defer-util.h>

#include <cmath>
#include <vector>
#include <csetjmp>

#define unless(x) if(!(x))

/* set by a "main.cpp", otherwise default values */
std::string ARG0;
std::vector<std::string> SRC_ARGS;
bool INTERACTIVE_MODE = false;

thread_local std::vector<Expression> activeCallStack;
static bool top_level_stmt = true;
static bool is_tailcallable = false;
uint64_t builtin_lambda_id = 0;

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Char = prim_value_t::Char;
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

value_t* evaluateLvalue(const Lvalue& lvalue, Environment* env, bool subscripted) {
    ::activeCallStack.push_back(lvalue);
    defer {::activeCallStack.pop_back();};
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
        defer {::activeCallStack.pop_back();};
        auto newChar = builtin::prim_ctor::Char_(new_value);
        auto old_value = value_t(new prim_value_t{Char(*c)});
        *c = newChar;
        env->symbolTable["$old"] = Environment::ConstValue{old_value};
        return;
    }

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
    if (!env->symbolTable.contains(leftmostSymbol.name)) {
        ::activeCallStack.push_back(letStmt.variable);
        throw InterpretError("Unbound symbol `" + leftmostSymbol.name + "`");
    }

    env->symbolTable[letStmt.alias.name] = Environment::LabelToLvalue{
        (thunk_t<value_t>)[&letStmt, env]() -> value_t {
            return evaluateValue(letStmt.variable, env);
        },
        (thunk_t<value_t*>)[&letStmt, env]() -> value_t* {
            return evaluateLvalue(letStmt.variable, env);
        }
    };
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
    auto fnCallPtr = new FunctionCall{(Expression)opPtr, {lhs, rhs}};
    fnCallPtr->_tokenId = operation.operator_._tokenId;

    auto res = evaluateValue((Expression)fnCallPtr, env);
    res = deepcopy(res);

    delete opPtr;
    delete fnCallPtr;

    return res;
}

value_t evaluateValue(const FunctionCall& fnCall, Environment* env) {
    auto fnVal = evaluateValue(fnCall.function, env);
    ASSERT (std::holds_alternative<prim_value_t*>(fnVal)); // TODO: tmp
    auto* fnPrimValPtr = std::get<prim_value_t*>(fnVal);
    if (fnPrimValPtr == nullptr) {
        throw InterpretError("Calling a $nil");
    }
    if (!std::holds_alternative<prim_value_t::Lambda>(fnPrimValPtr->variant)) {
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
            unless (env->contains(symbol->name)) break;
            auto symbolVal = env->at(symbol->name);
            unless (std::holds_alternative<Environment::VariadicArguments>(symbolVal)) break;
            auto varargs = std::get<Environment::VariadicArguments>(symbolVal);

            flattenArgs.insert(flattenArgs.end(), varargs.begin(), varargs.end());

            goto CONTINUE;
        }
        
        /* handle "single" argument */
        flattenArgs.push_back({arg, env});

        CONTINUE:
    }

    auto function = std::get<prim_value_t::Lambda>(fnPrimValPtr->variant);

    /* recursive tail call elimination */
    static auto savedCalledFns = std::map<uint64_t, BackupStack>{};
    if (savedCalledFns.contains(function.id)) {
        if (is_tailcallable && fnCall.arguments.size() == 0) {
            activeCallStack = savedCalledFns.at(function.id).activeCallStack;
            longjmp(savedCalledFns.at(function.id).jmpBuf, 1);
        }
    }
    else if (fnCall.arguments.size() == 0) {
        // map auto-vivification creates a default jmpBuf here
        savedCalledFns[function.id].activeCallStack = activeCallStack;
        if (setjmp(savedCalledFns.at(function.id).jmpBuf)) {
            ;
        }
    }

    auto res = function.stdfunc(flattenArgs);
    savedCalledFns.erase(function.id);
    return res;
}

static bool check_if_tailcallable(const Statement& stmt) {
    #ifdef TOGGLE_TAILCALL
    unless (std::holds_alternative<Assignment*>(stmt)) return false;
    auto assignment = std::get<Assignment*>(stmt);
    unless (std::holds_alternative<Symbol*>(assignment->variable.variant)) return false;
    auto varSymbol = std::get<Symbol*>(assignment->variable.variant);
    return varSymbol->name == "_";
    #else
    (void)stmt;
    return false;
    #endif
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

    Environment* envAtCreation = env->deepcopy();
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
            if (!lambda.variadicParameters && flattenArgs.size() != lambda.parameters.size()) {
                ::activeCallStack.push_back(const_cast<LV2::Lambda*>(&lambda));
                throw WrongNbOfArgsError(lambda.parameters, flattenArgs);
            }
            if (lambda.variadicParameters && flattenArgs.size() < lambda.parameters.size()) {
                ::activeCallStack.push_back(const_cast<LV2::Lambda*>(&lambda));
                throw WrongNbOfArgsError(lambda.parameters, flattenArgs);
            }
            
            size_t i = 0;

            /* binding required parameters (<> variadic parameters) */
            for (; i < lambda.parameters.size(); ++i) {
                auto currParam = lambda.parameters.at(i);
                auto currArg = flattenArgs.at(i);

                if (parametersBinding.contains(currParam.name)
                        && currParam.name != "_") {
                    SHOULD_NOT_HAPPEN(); // BUG: Lambda had two parameters with the same name
                }

                if (currArg.passByRef) {
                    // we should immediatly evaluate any subscript index/range..
                    // (if they refer to symbols instead of numerals)
                    parametersBinding[currParam.name] = Environment::PassByRef{
                        (thunk_t<value_t>)[currArg]() -> value_t {
                            return evaluateValue(currArg.expr, currArg.env);
                        },
                        (thunk_t<value_t*>)[currArg]() -> value_t* {
                            return evaluateLvalue(currArg.expr, currArg.env);
                        }
                    };
                }
                else {
                    #ifdef TOGGLE_PASS_BY_VALUE
                    auto var = new value_t{evaluateValue(currArg.expr, currArg.env)}; // TODO: leak
                    *var = deepcopy(*var);
                    parametersBinding[currParam.name] = Environment::Variable{var};
                    #else // lazy passing a.k.a pass by delayed
                    auto* thunkEnv = new Environment{*currArg.env}; // no deep copy needed apparently ?
                                                                    // .. = currArg.env->deepcopy();
                    auto* delayed = new thunk_with_memoization_t<value_t>{
                        [currArg, thunkEnv]() -> value_t {
                            auto res = evaluateValue(currArg.expr, thunkEnv);
                            res = deepcopy(res);
                            return res;

                        }
                    };
                    parametersBinding[currParam.name] = new Environment::PassByDelay_Variant{delayed};
                    #endif
                }
            }

            /* binding variadic parameters */
            if (lambda.variadicParameters) {
                auto varargs = Environment::VariadicArguments{
                    flattenArgs.begin() + i,
                    flattenArgs.end()
                };
                parametersBinding[lambda.variadicParameters->name] = varargs;
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

            i = 0;
            for (; i < lambda.body.statements.size() - 1; ++i) {
                performStatement(lambda.body.statements.at(i), &lambdaEnv);
            }

                auto backup_is_tailcallable = is_tailcallable;
                is_tailcallable = check_if_tailcallable(lambda.body.statements.at(i));
                defer {is_tailcallable = backup_is_tailcallable;};

            if (std::holds_alternative<ExpressionStatement*>(lambda.body.statements.at(i))) {
                auto exprStmt = *std::get<ExpressionStatement*>(lambda.body.statements.at(i));
                if (exprStmt.expression) {
                    auto res = evaluateValue(*exprStmt.expression, &lambdaEnv);
                    res = deepcopy(res);
                    return res;
                }
                return nil_value_t();
            }
            performStatement(lambda.body.statements.at(i), &lambdaEnv);
            return nil_value_t();
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

    size_t i = 0;
    for (; i < blockExpr.statements.size() - 1; ++i) {
        performStatement(blockExpr.statements.at(i), newEnv);
    }

    auto backup_is_tailcallable = is_tailcallable;
    is_tailcallable = check_if_tailcallable(blockExpr.statements.at(i));
    defer {is_tailcallable = backup_is_tailcallable;};

    if (std::holds_alternative<ExpressionStatement*>(blockExpr.statements.at(i))) {
        auto exprStmt = *std::get<ExpressionStatement*>(blockExpr.statements.at(i));
        if (exprStmt.expression) {
            auto res = evaluateValue(*exprStmt.expression, newEnv);
            res = deepcopy(res);
            return res;
        }
        return nil_value_t();
    }
    performStatement(blockExpr.statements.at(i), newEnv);
    return nil_value_t();
}

value_t evaluateValue(const FieldAccess&, const Environment*) {
    TODO();
}

value_t evaluateValue(const Subscript& subscript, Environment* env) {
    auto arrVal = evaluateValue(subscript.array, env);
    arrVal = deepcopy(arrVal);
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
                defer {::activeCallStack.pop_back();};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs(intVal) <= str.size()) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? str.size() - abs(intVal) : size_t(intVal) - 1;
                return new prim_value_t{Char(str.at(pos))};
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);

                /* from */
                ::activeCallStack.push_back(variant_cast(range.from));
                auto fromVal = evaluateValue(variant_cast(range.from), env);
                auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                Int fromPos = intFromVal < 0? Int(str.size()) - abs(intFromVal) : intFromVal - 1;
                unless (fromPos < Int(str.size())) throw InterpretError("Subscript range 'from' is out of bounds");
                ::activeCallStack.pop_back(); // variant_cast(range.from)

                /* to */
                ::activeCallStack.push_back(variant_cast(range.to));
                auto toVal = evaluateValue(variant_cast(range.to), env);
                auto intToVal = builtin::prim_ctor::Int_(toVal);
                unless (intToVal != 0) throw InterpretError("Subscript range 'to' is zero");
                Int toPos = intToVal < 0? Int(str.size()) - abs(intToVal) : intToVal - 1;
                if (range.exclusive) {
                    toPos -= fromPos <= toPos? 1 : -1;
                }
                unless (toPos < Int(str.size())) throw InterpretError("Subscript range 'to' is out of bounds");
                ::activeCallStack.pop_back(); // variant_cast(range.to)

                return new prim_value_t{str.substr(fromPos, toPos - fromPos + 1)};
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
                defer {::activeCallStack.pop_back();};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs(intVal) <= list.size()) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? list.size() - abs(intVal) : size_t(intVal) - 1;
                return list.at(pos);
            }

            else if (std::holds_alternative<Subscript::Range>(subscript.argument)) {
                auto range = std::get<Subscript::Range>(subscript.argument);

                /* from */
                ::activeCallStack.push_back(variant_cast(range.from));
                auto fromVal = evaluateValue(variant_cast(range.from), env);
                auto intFromVal = builtin::prim_ctor::Int_(fromVal);
                unless (intFromVal != 0) throw InterpretError("Subscript range 'from' is zero");
                Int fromPos = intFromVal < 0? Int(list.size()) - abs(intFromVal) : intFromVal - 1;
                unless (fromPos < Int(list.size())) throw InterpretError("Subscript range 'from' is out of bounds");
                ::activeCallStack.pop_back(); // variant_cast(range.from)

                /* to */
                ::activeCallStack.push_back(variant_cast(range.to));
                auto toVal = evaluateValue(variant_cast(range.to), env);
                auto intToVal = builtin::prim_ctor::Int_(toVal);
                unless (intToVal != 0) throw InterpretError("Subscript range 'to' is zero");
                Int toPos = intToVal < 0? Int(list.size()) - abs(intToVal) : intToVal - 1;
                if (range.exclusive) {
                    toPos -= fromPos <= toPos? 1 : -1;
                }
                unless (toPos < Int(list.size())) throw InterpretError("Subscript range 'to' is out of bounds");
                ::activeCallStack.pop_back(); // variant_cast(range.to)

                auto res = List(list.begin() + fromPos, list.begin() + + toPos + 1);
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
                unless (map.contains(keyVal)) throw InterpretError("Subscript key not found");
                return map.at(keyVal);
            }

            else SHOULD_NOT_HAPPEN();
        },

        [](Bool) -> value_t {throw InterpretError("Cannot subscript a Bool");},
        [](Byte) -> value_t {throw InterpretError("Cannot subscript a Byte");},
        [](Int) -> value_t {throw InterpretError("Cannot subscript an Int");},
        [](Float) -> value_t {throw InterpretError("Cannot subscript a Float");},
        [](Char) -> value_t {throw InterpretError("Cannot subscript a Char");},
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

    if (specialSymbol.name == "$args") {
        return ARGS;
    }

    if (!env->contains(specialSymbol.name)) {
        throw InterpretError("Unbound symbol `" + specialSymbol.name + "`");
    }

    auto specialSymbolVal = env->at(specialSymbol.name);
    return std::visit(overload{
        [](const Environment::ConstValue& const_) -> value_t {return const_;},
        [](Environment::Variable) -> value_t {SHOULD_NOT_HAPPEN();},
        [](const Environment::LabelToLvalue& /*or PassByRef*/) -> value_t {SHOULD_NOT_HAPPEN();},
        [](const Environment::PassByDelay&) -> value_t {SHOULD_NOT_HAPPEN();},
        [](const Environment::VariadicArguments&) -> value_t {SHOULD_NOT_HAPPEN();},
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
        auto power = std::pow(base, exponent);
        return new prim_value_t(Int(power));
    }

    if (numeral.type == "fix_only") {
        auto int_part = std::stoll(numeral.int1);
        auto numerator = std::stoll(numeral.fixed);
        auto denominator = std::pow(10, numeral.fixed.size());
        auto division = (double)numerator / denominator;
        auto sum = int_part + (int_part < 0? -division : division);
        if (numeral.int1.starts_with("-") && int_part == 0.0) sum = 0 - sum;
        return new prim_value_t(Float(sum));
    }

    if (numeral.type == "per_only") {
        auto int_part = std::stoll(numeral.int1);
        auto numerator = std::stoll(numeral.periodic);
        auto denominator = std::pow(10, numeral.periodic.size()) - 1;
        auto division = (double)numerator / denominator;
        auto sum = int_part + (int_part < 0? -division : division);
        if (numeral.int1.starts_with("-") && int_part == 0.0) sum = 0 - sum;
        return new prim_value_t(Float(sum));
    }

    if (numeral.type == "fix_and_per") {
        auto int_part = std::stoll(numeral.int1);
        auto fixed_part_numerator = std::stoll(numeral.fixed);
        auto fixed_part_denominator = std::pow(10, numeral.fixed.size());
        auto fixed_part_division = (double)fixed_part_numerator / fixed_part_denominator;
        auto periodic_part_numerator = std::stoll(numeral.periodic);
        auto periodic_part_denominator = (std::pow(10, numeral.periodic.size()) - 1) * fixed_part_denominator;
        auto periodic_part_division = (double)periodic_part_numerator / periodic_part_denominator;
        auto sum = int_part < 0? int_part - fixed_part_division - periodic_part_division
                : int_part + fixed_part_division + periodic_part_division;
        if (numeral.int1.starts_with("-") && int_part == 0.0) sum = 0 - sum;
        return new prim_value_t(Float(sum));
    }

    SHOULD_NOT_HAPPEN(); // BUG unknown numeral type
}

value_t evaluateValue(const StrLiteral& strLiteral, const Environment*) {
    return new prim_value_t((Str)strLiteral.str);
}

value_t evaluateValue(const Symbol& symbol, Environment* env) {
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

value_t* evaluateLvalue(const FieldAccess&, Environment*) {
    TODO();
}

value_t* evaluateLvalue(const Subscript& subscript, Environment* env) {
    //TODO: this is catch during syntax analysis, so should remove ?
    if (!is_lvalue(subscript.array)) {
        throw InterpretError("lvaluing a non-lvalue subscript array");
    }

    auto* lvalue = evaluateLvalue(subscript.array, env, /*subscripted*/true);
    ASSERT (lvalue != nullptr);
    ASSERT (std::holds_alternative<prim_value_t*>(*lvalue)); // TODO: tmp
    auto* lvaluePrimValPtr = std::get<prim_value_t*>(*lvalue);
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
                defer {::activeCallStack.pop_back();};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs(intVal) <= str.size()) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? str.size() - abs(intVal) : size_t(intVal) - 1;

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
                defer {::activeCallStack.pop_back();};
                auto intVal = builtin::prim_ctor::Int_(nthVal);

                unless (intVal != 0) throw InterpretError("Subscript index is zero");
                unless (abs(intVal) <= list.size()) throw InterpretError("Subscript index is out of bounds");
                auto pos = intVal < 0? list.size() - abs(intVal) : size_t(intVal) - 1;

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
                if (subscript.suffix == '!' && !map.contains(keyVal)) {
                    throw InterpretError("Subscript key not found");
                }
                return &map[keyVal];
            }
            else SHOULD_NOT_HAPPEN();
        },

        [](Bool&) -> value_t* {throw InterpretError("Cannot subscript a Bool");},
        [](Byte&) -> value_t* {throw InterpretError("Cannot subscript a Byte");},
        [](Int&) -> value_t* {throw InterpretError("Cannot subscript an Int");},
        [](Float&) -> value_t* {throw InterpretError("Cannot subscript a Float");},
        [](Char&) -> value_t* {throw InterpretError("Cannot subscript a Char");},
        [](prim_value_t::Lambda&) -> value_t* {throw InterpretError("Cannot subscript a Lambda");},
    }, lvaluePrimValPtr->variant);
}

value_t* evaluateLvalue(const Symbol& symbol, Environment* env, bool subscripted) {
    static value_t DISPOSABLE_LVALUE;
    if (symbol.name == "_") {
        return &DISPOSABLE_LVALUE;
    }

    if (!env->contains(symbol.name)) {
        throw InterpretError("Unbound symbol `" + symbol.name + "`");
    }

    auto symbolVal = env->at(symbol.name);
    return std::visit(overload{
        [](Environment::Variable var) -> value_t* {return var;},
        [](Environment::LabelToLvalue& label_ref) -> value_t* {return label_ref.lvalue();},
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

        /*
            TODO: runtime error: Not an lvalue
              -> when non-lvalue is passed by ref, or assigned a value
                -> at this time, we can't trigger it, we need the let stmt
        */
        [](Environment::ConstValue) -> value_t* {SHOULD_NOT_HAPPEN();},
        [](const Environment::VariadicArguments&) -> value_t* {
            throw InterpretError("Cannot refer to variadic arguments as symbol");
        },
    }, symbolVal);

}
