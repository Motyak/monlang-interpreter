
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <map>

using value_t = int; // TODO: tmp

template <typename R>
class thunk_t {
  protected:
    std::function<R()> fn;

  public:
    thunk_t(const std::function<R()>& fn) : fn(fn){}
    virtual R operator()();
};

template <typename R>
class thunk_with_memoization_t : public thunk_t<R> {
  private:
    std::optional<R> memoized;

  public:
    thunk_with_memoization_t(const std::function<R()>& fn) : thunk_t<R>(fn){}
    R operator()() override;
};

struct Environment {
    using ConstValue = value_t;

    using Variable = value_t*;

    using LabelToConst = ConstValue;
    using LabelToNonConst = thunk_t<value_t>;
    using LabelToLvalue = thunk_t<value_t*>;

    // argument passed by delayed value (lazy pass by value)
    using PassByDelayed = thunk_with_memoization_t<value_t>;
    // argument passed by reference
    using PassByRef = LabelToLvalue;

    using SymbolName = std::string;
    using SymbolValue = std::variant<
        ConstValue /*or LabelToConst*/,
        Variable,
        LabelToNonConst,
        LabelToLvalue /*or PassByRef*/,
        PassByDelayed
    >;
    std::map<SymbolName, SymbolValue> symbolTable;

    Environment* enclosingEnv;
};

template <typename R>
R thunk_t<R>::operator()() {
    return this->fn();
}

template <typename R>
R thunk_with_memoization_t<R>::operator()() {
    if (!this->memoized) {
        this->memoized = this->fn();
    }
    return *this->memoized;
}

#endif // ENVIRONMENT_H
