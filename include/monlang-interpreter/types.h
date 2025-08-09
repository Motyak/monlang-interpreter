/*
    standalone header, no .cpp

    defines types:
    value_t
    nil_value_t
    prim_value_t
    type_value_t
    struct_value_t
    enum_value_t
    thunk_t
    thunk_with_memoization_t
*/

#ifndef TYPES_H
#define TYPES_H

#include <monlang-LV2/ast/expr/FunctionCall.h>

#include <variant>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <optional>

#define nil_value_t() ((prim_value_t*)nullptr)

struct Environment;

// flatten / normalized argument
struct FlattenArg : public FunctionCall::Argument {
    Environment* env = nullptr;
};

struct prim_value_t;
struct type_value_t;
struct struct_value_t;
struct enum_value_t;

using value_t = std::variant<
    prim_value_t*, // primitive
    /* user-defined */
    type_value_t*,
    struct_value_t*,
    enum_value_t*,
    /* for evaluating Str subscript as lvalue */
    char*
>;

struct prim_value_t {
    using Bool = bool;
    using Byte = uint8_t;
    using Int = int64_t;
    using Float = double;
    using Char = char;
    using Str = std::string;
    using List = std::vector<value_t>;
    using Map = std::map<value_t, value_t, std::greater<value_t>>;
    //  -> std::greater allows us to keep insertion order
    //     (C++/std impl dependendant but well...)

    struct Lambda {
        uint64_t id;
        value_t requiredParams;
        std::function<value_t(const std::vector<FlattenArg>&)> stdfunc;
    };

    using Variant = std::variant<
        Bool,
        Byte,
        Int,
        Float,
        Char,
        Str,
        List,
        Map,
        Lambda
    >;
    Variant variant;
};

inline prim_value_t::Bool asBool(const prim_value_t& val) {return std::get<prim_value_t::Bool>(val.variant);}
inline prim_value_t::Byte asByte(const prim_value_t& val) {return std::get<prim_value_t::Byte>(val.variant);}
inline prim_value_t::Int asInt(const prim_value_t& val) {return std::get<prim_value_t::Int>(val.variant);}
inline prim_value_t::Float asFloat(const prim_value_t& val) {return std::get<prim_value_t::Float>(val.variant);}
inline prim_value_t::Char asChar(const prim_value_t& val) {return std::get<prim_value_t::Char>(val.variant);}
inline prim_value_t::Str asStr(const prim_value_t& val) {return std::get<prim_value_t::Str>(val.variant);}
inline prim_value_t::List asList(const prim_value_t& val) {return std::get<prim_value_t::List>(val.variant);}
inline prim_value_t::Map asMap(const prim_value_t& val) {return std::get<prim_value_t::Map>(val.variant);}
inline prim_value_t::Lambda asLambda(const prim_value_t& val) {return std::get<prim_value_t::Lambda>(val.variant);}

struct type_value_t {
    std::string_view type;
    value_t value;
};

struct struct_value_t {
    std::string_view
    type;

    std::vector<std::pair<std::string_view, value_t>>
    fields;
};

struct enum_value_t {
    std::string_view type;
    std::string_view enumerate_name;
    value_t enumerate_value;
};

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
  public:
    std::optional<R> memoized;

    thunk_with_memoization_t(const std::function<R()>& fn) : thunk_t<R>(fn){}
    R operator()() override;
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

inline bool is_nil(const value_t& value) {
    return std::visit(
        [](auto* value){return value == nullptr;}
        , value
    );
}

#endif // TYPES_H
