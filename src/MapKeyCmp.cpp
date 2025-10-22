#include <monlang-interpreter/MapKeyCmp.h>
#include <monlang-interpreter/types.h>

#include <utils/variant-utils.h>
#include <utils/assert-utils.h>

using Bool = prim_value_t::Bool;
using Byte = prim_value_t::Byte;
using Int = prim_value_t::Int;
using Float = prim_value_t::Float;
using Char = prim_value_t::Char;
using Str = prim_value_t::Str;
using List = prim_value_t::List;
using Map = prim_value_t::Map;

static int cmp(const owned_value_t& lhs, const owned_value_t& rhs);
static int cmp(prim_value_t* lhs, prim_value_t* rhs);
static int cmp(type_value_t* lhs, type_value_t* rhs);
static int cmp(struct_value_t* lhs, struct_value_t* rhs);
static int cmp(enum_value_t* lhs, enum_value_t* rhs);

bool MapKeyCmp::operator()(const owned_value_t& lhs, const owned_value_t& rhs) const {
    if (is_nil(lhs) && is_nil(rhs)) return false;
    if (is_nil(lhs) && !is_nil(rhs)) return true;
    if (!is_nil(lhs) && is_nil(rhs)) return false;
    auto lhsIndex = lhs.index();
    auto rhsIndex = rhs.index();
    if (lhsIndex != rhsIndex) {
        return lhsIndex < rhsIndex;
    }
    return std::visit(overload{
        [&rhs](const std::unique_ptr<prim_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<prim_value_t>>(rhs).get()) < 0;
        },

        [&rhs](const std::unique_ptr<type_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<type_value_t>>(rhs).get()) < 0;
        },

        [&rhs](const std::unique_ptr<struct_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<struct_value_t>>(rhs).get()) < 0;
        },

        [&rhs](const std::unique_ptr<enum_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<enum_value_t>>(rhs).get()) < 0;
        },

        [](char*) -> bool {SHOULD_NOT_HAPPEN();},
    }, lhs);
}

static int cmp(const owned_value_t& lhs, const owned_value_t& rhs) {
    if (is_nil(lhs) && is_nil(rhs)) return 0;
    if (is_nil(lhs) && !is_nil(rhs)) return -1;
    if (!is_nil(lhs) && is_nil(rhs)) return 1;
    auto lhsIndex = lhs.index();
    auto rhsIndex = rhs.index();
    if (lhsIndex != rhsIndex) {
        return lhsIndex < rhsIndex? -1 : 1;
    }
    return std::visit(overload{
        [&rhs](const std::unique_ptr<prim_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<prim_value_t>>(rhs).get()) < 0;
        },

        [&rhs](const std::unique_ptr<type_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<type_value_t>>(rhs).get()) < 0;
        },

        [&rhs](const std::unique_ptr<struct_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<struct_value_t>>(rhs).get()) < 0;
        },

        [&rhs](const std::unique_ptr<enum_value_t>& lhs) -> bool {
            return cmp(lhs.get(), std::get<std::unique_ptr<enum_value_t>>(rhs).get()) < 0;
        },

        [](char*) -> bool {SHOULD_NOT_HAPPEN();},
    }, lhs);
}

static int cmp(prim_value_t* lhs, prim_value_t* rhs) {
    auto lhsIndex = lhs->variant.index();
    auto rhsIndex = rhs->variant.index();
    if (lhsIndex != rhsIndex) {
        return lhsIndex < rhsIndex? -1 : 1;
    }
    return std::visit(overload{
        [rhs](Bool lhs) -> int {
            auto rhsAsBool = std::get<Bool>(rhs->variant);
            return lhs < rhsAsBool? -1 : lhs > rhsAsBool? 1 : 0;
        },

        [rhs](Byte lhs) -> int {
            auto rhsAsByte = std::get<Byte>(rhs->variant);
            return lhs < rhsAsByte? -1 : lhs > rhsAsByte? 1 : 0;
        },

        [rhs](Int lhs) -> int {
            auto rhsAsInt = std::get<Int>(rhs->variant);
            return lhs < rhsAsInt? -1 : lhs > rhsAsInt? 1 : 0;
        },

        [rhs](Float lhs) -> int {
            auto rhsAsFloat = std::get<Float>(rhs->variant);
            return lhs < rhsAsFloat? -1 : lhs > rhsAsFloat? 1 : 0;
        },

        [rhs](Char lhs) -> int {
            auto rhsAsChar = std::get<Char>(rhs->variant);
            return lhs < rhsAsChar? -1 : lhs > rhsAsChar? 1 : 0;
        },

        [rhs](const Str& lhs) -> int {
            const auto& rhsAsStr = std::get<Str>(rhs->variant);
            return lhs < rhsAsStr? -1 : lhs > rhsAsStr? 1 : 0;
        },

        [rhs](const List& lhs) -> int {
            const auto& rhsList = std::get<List>(rhs->variant);
            auto lhsSize = lhs.size();
            auto rhsSize = rhsList.size();
            if (lhsSize != rhsSize) {
                return lhsSize < rhsSize? -1 : 1;
            }
            for (size_t i = 0; i < lhsSize; ++i) {
                auto cmpRes = cmp(lhs.at(i), rhsList.at(i));
                if (cmpRes != 0) {
                    return cmpRes;
                }
            }
            return 0;
        },

        [rhs](const Map& lhs) -> int {
            const auto& rhsMap = std::get<Map>(rhs->variant);
            auto lhsSize = lhs.size();
            auto rhsSize = rhsMap.size();
            if (lhsSize != rhsSize) {
                return lhsSize < rhsSize? -1 : 1;
            }
            auto lhsIt = lhs.begin();
            auto rhsIt = rhsMap.begin();
            for (; lhsIt != lhs.end(); ++lhsIt, ++rhsIt) {
                auto keyCmpRes = cmp(lhsIt->first, rhsIt->first);
                if (keyCmpRes != 0) {
                    return keyCmpRes;
                }
                auto valCmpRes = cmp(lhsIt->second, rhsIt->second);
                if (valCmpRes != 0) {
                    return valCmpRes;
                }
            }
            return 0;
        },

        [rhs](const prim_value_t::Lambda& lhs) -> int {
            const auto& rhsAsLambda = std::get<prim_value_t::Lambda>(rhs->variant);
            return lhs.id < rhsAsLambda.id? -1 : lhs.id > rhsAsLambda.id? 1 : 0;
        },

    }, lhs->variant);
}

static int cmp(type_value_t* lhs, type_value_t* rhs) {
    (void)lhs;
    (void)rhs;
    TODO();
}

static int cmp(struct_value_t* lhs, struct_value_t* rhs) {
    (void)lhs;
    (void)rhs;
    TODO();
}

static int cmp(enum_value_t* lhs, enum_value_t* rhs) {
    (void)lhs;
    (void)rhs;
    TODO();
}
