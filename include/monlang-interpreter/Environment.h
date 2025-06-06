#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <monlang-interpreter/types.h>

#include <functional>
#include <string>
#include <variant>
#include <map>

struct Environment {
    using ConstValue = value_t;

    using Variable = value_t*;

    using LabelToConst = ConstValue;
    using LabelToNonConst = thunk_t<value_t>;
    using LabelToLvalue = thunk_t<value_t*>;

    // argument passed by delayed value (lazy pass by value)
    using PassByDelay_Variant = std::variant<
        thunk_with_memoization_t<value_t>*,
        value_t* // once transformed
    >;
    using PassByDelay = PassByDelay_Variant*;
    struct PassByRef {
        thunk_t<value_t> value;
        thunk_t<value_t*> lvalue;
    };

    using VariadicArguments = std::vector<std::pair<FunctionCall::Argument, Environment*>>;

    using SymbolName = std::string;
    using SymbolValue = std::variant<
        ConstValue /*or LabelToConst*/,
        Variable,
        LabelToNonConst,
        LabelToLvalue,
        PassByDelay,
        PassByRef,
        VariadicArguments
    >;
    std::map<SymbolName, SymbolValue> symbolTable = {};

    Environment* enclosingEnv = nullptr;

    bool contains(const SymbolName&) const;
    const SymbolValue& at(const SymbolName&) const;
    SymbolValue& at(const SymbolName&);
};

#endif // ENVIRONMENT_H
