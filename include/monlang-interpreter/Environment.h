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
    using PassByDelayed = thunk_with_memoization_t<value_t>* const;
    struct PassByRef {
        std::function<value_t()> value;
        std::function<value_t*()> lvalue;
    };

    using SymbolName = std::string;
    using SymbolValue = std::variant<
        ConstValue /*or LabelToConst*/,
        Variable,
        LabelToNonConst,
        LabelToLvalue,
        PassByDelayed,
        PassByRef
    >;
    std::map<SymbolName, SymbolValue> symbolTable = {};

    Environment* enclosingEnv = nullptr;

    bool contains(const SymbolName&) const;
    const SymbolValue& at(const SymbolName&) const;
    SymbolValue& at(const SymbolName&);
};

#endif // ENVIRONMENT_H
