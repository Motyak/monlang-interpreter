#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <monlang-interpreter/types.h>

#include <functional>
#include <string>
#include <variant>
#include <map>

class Environment {
  public:
    using ConstValue = owned_value_t;
    using Variable = std::shared_ptr<owned_value_t>;
    struct LabelToLvalue {
        thunk_t<value_t> value;
        thunk_t<lvalue_t> lvalue;
    };

    // argument passed by delayed value (lazy pass by value)
    using PassByDelay_Variant = std::variant<
        thunk_with_memoization_t<value_t>*, // ptr is necessary here
        owned_value_t // once transformed
    >;
    using PassByDelay = std::shared_ptr<PassByDelay_Variant>;
    using PassByRef = LabelToLvalue;

    using VariadicArguments = std::vector<FlattenArg>;

    using SymbolName = std::string_view;
    using SymbolValue = std::variant<
        ConstValue,
        Variable,
        LabelToLvalue /*or PassByRef*/,
        PassByDelay,
        VariadicArguments
    >;
    std::map<SymbolName, SymbolValue> symbolTable = {};

    std::shared_ptr<Environment> enclosingEnv = nullptr;

    bool contains(const SymbolName&) const;
    const SymbolValue& at(const SymbolName&) const;
    SymbolValue& at(const SymbolName&);
    std::shared_ptr<Environment> rec_copy(); // fork symbols (shallow copy on variables)
    std::shared_ptr<Environment> rec_deepcopy(); // fork symbols AND variables
};

#endif // ENVIRONMENT_H
