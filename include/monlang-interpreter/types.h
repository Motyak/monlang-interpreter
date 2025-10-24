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

#include <monlang-interpreter/MapKeyCmp.h>
#include <monlang-LV2/ast/expr/FunctionCall.h>

#include <utils/assert-utils.h>
#include <utils/variant-utils.h>

#include <variant>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <optional>
#include <memory>

#define nil_value_t() ((prim_value_t*)nullptr)

struct Environment;

// flatten / normalized argument
struct FlattenArg : public FunctionCall::Argument {
    std::shared_ptr<Environment> env = nullptr;
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
    char* // TODO: no longer used, should be removed + clean everywhere else
>;

using lvalue_t = std::variant<
    owned_value_t*,
    /* for evaluating Str subscript as lvalue */
    char*
>;

using owned_value_t = std::variant<
    std::unique_ptr<prim_value_t>,
    std::unique_ptr<type_value_t>,
    std::unique_ptr<struct_value_t>,
    std::unique_ptr<enum_value_t>
>;

struct prim_value_t {
    using Bool = bool;
    using Byte = uint8_t;
    using Int = int64_t;
    using Float = double;
    using Char = char;
    using Str = std::string;
    using List = std::vector<owned_value_t>;
    using Map = std::map<owned_value_t, owned_value_t, MapKeyCmp>;

    struct Lambda {
        uint64_t id;
        owned_value_t requiredArgs;
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

// TODO
struct type_value_t {
    std::string_view type;
    value_t value;
};

// TODO
struct struct_value_t {
    std::string_view
    type;

    std::vector<std::pair<std::string_view, value_t>>
    fields;
};

// TODO
struct enum_value_t {
    std::string_view type;
    std::string_view enumerate_name; // TODO: no need ?
    value_t enumerate_value;
};

template <typename R>
class thunk_t {
  protected:
    std::function<R()> fn;

  public:
    thunk_t(const std::function<R()>& fn) : fn(fn){}
    virtual ~thunk_t(){}
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

template <typename... Targs>
bool is_nil(const std::variant<Targs...>& variantPtrs) {
    return std::visit(
        [](const auto& value){return value == nullptr;}
        , variantPtrs
    );
}

inline owned_value_t own(const value_t& val) {
    return std::visit(overload{
        [](prim_value_t* val) -> owned_value_t {return std::unique_ptr<prim_value_t>(val);},
        [](type_value_t* val) -> owned_value_t {return std::unique_ptr<type_value_t>(val);},
        [](struct_value_t* val) -> owned_value_t {return std::unique_ptr<struct_value_t>(val);},
        [](enum_value_t* val) -> owned_value_t {return std::unique_ptr<enum_value_t>(val);},
        [](char*) -> owned_value_t {SHOULD_NOT_HAPPEN();},
    }, val);
}

inline owned_value_t copy_own(const owned_value_t& val) {
    return std::visit(overload{
        [](const std::unique_ptr<prim_value_t>& val) -> owned_value_t {
            return std::visit(overload{
                [](prim_value_t::Bool bool_){return std::make_unique<prim_value_t>(bool_);},
                [](prim_value_t::Byte byte){return std::make_unique<prim_value_t>(byte);},
                [](prim_value_t::Int int_){return std::make_unique<prim_value_t>(int_);},
                [](prim_value_t::Float float_){return std::make_unique<prim_value_t>(float_);},
                [](prim_value_t::Char char_){return std::make_unique<prim_value_t>(char_);},
                [](const prim_value_t::Str& str){return std::make_unique<prim_value_t>(str);},
                [](const prim_value_t::List& list){
                    auto res = std::make_unique<prim_value_t>(prim_value_t::List());
                    auto& res_list = std::get<prim_value_t::List>(res->variant);
                    for (const auto& item: list) {
                        res_list.push_back(copy_own(item));
                    }
                    return res;
                },
                [](const prim_value_t::Map& map){
                    auto res = std::make_unique<prim_value_t>(prim_value_t::Map());
                    auto& res_map = std::get<prim_value_t::Map>(res->variant);
                    for (const auto& [key, val]: map) {
                        res_map[copy_own(key)] = copy_own(val);
                    }
                    return res;
                },
                [](const prim_value_t::Lambda& lambda){
                    return std::make_unique<prim_value_t>(prim_value_t::Lambda{
                        lambda.id,
                        std::make_unique<prim_value_t>(
                            std::get<prim_value_t::Int>(
                                std::get<std::unique_ptr<prim_value_t>>(lambda.requiredArgs)->variant
                            )
                        ),
                        lambda.stdfunc
                    });
                },
            }, val->variant);
        },

        [](const std::unique_ptr<type_value_t>& val) -> owned_value_t {return std::make_unique<type_value_t>(*val);},
        [](const std::unique_ptr<struct_value_t>& val) -> owned_value_t {return std::make_unique<struct_value_t>(*val);},
        [](const std::unique_ptr<enum_value_t>& val) -> owned_value_t {return std::make_unique<enum_value_t>(*val);},
    }, val);
}

inline value_t copy_own_(const owned_value_t& val) {
    return std::visit(overload{
        [](const std::unique_ptr<prim_value_t>& val) -> value_t {
            return std::visit(overload{
                [](prim_value_t::Bool bool_){return new prim_value_t{bool_};},
                [](prim_value_t::Byte byte){return new prim_value_t{byte};},
                [](prim_value_t::Int int_){return new prim_value_t{int_};},
                [](prim_value_t::Float float_){return new prim_value_t{float_};},
                [](prim_value_t::Char char_){return new prim_value_t{char_};},
                [](const prim_value_t::Str& str){return new prim_value_t{str};},
                [](const prim_value_t::List& list){
                    auto res = new prim_value_t{prim_value_t::List()};
                    auto& res_list = std::get<prim_value_t::List>(res->variant);
                    for (const auto& item: list) {
                        res_list.push_back(copy_own(item));
                    }
                    return res;
                },
                [](const prim_value_t::Map& map){
                    auto res = new prim_value_t{prim_value_t::Map()};
                    auto& res_map = std::get<prim_value_t::Map>(res->variant);
                    for (const auto& [key, val]: map) {
                        res_map[copy_own(key)] = copy_own(val);
                    }
                    return res;
                },
                [](const prim_value_t::Lambda& lambda){
                    return new prim_value_t{prim_value_t::Lambda{
                        lambda.id,
                        std::make_unique<prim_value_t>(
                            std::get<prim_value_t::Int>(
                                std::get<std::unique_ptr<prim_value_t>>(lambda.requiredArgs)->variant
                            )
                        ),
                        lambda.stdfunc
                    }};
                },
            }, val->variant);
        },

        [](const std::unique_ptr<type_value_t>& val) -> value_t {return new type_value_t{*val};},
        [](const std::unique_ptr<struct_value_t>& val) -> value_t {return new struct_value_t{*val};},
        [](const std::unique_ptr<enum_value_t>& val) -> value_t {return new enum_value_t{*val};},
    }, val);
}

#endif // TYPES_H
