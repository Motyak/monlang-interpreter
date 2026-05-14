/*
    standalone header, no .cpp
*/

#ifndef INTERP_DEEPCOPY_H
#define INTERP_DEEPCOPY_H

#include <monlang-interpreter/types.h>

#include <utils/variant-utils.h>
#include <utils/assert-utils.h>

static value_t deepcopy(prim_value_t*);
static value_t deepcopy(type_value_t*);
static value_t deepcopy(struct_value_t*);
static value_t deepcopy(enum_value_t*);

inline value_t deepcopy(const value_t& val) {
    if (is_nil(val)) {
        return val;
    }
    return std::visit(overload{
        [](char*) -> value_t {SHOULD_NOT_HAPPEN();},
        [](FieldLvalue*) -> value_t {SHOULD_NOT_HAPPEN();},
        [](auto* val) -> value_t {return deepcopy(val);}
    }, val);
}

static value_t deepcopy(prim_value_t* primValPtr) {
    ASSERT (primValPtr != nullptr);
    return std::visit(overload{
        [](const prim_value_t::Str& str) -> value_t {
            return new prim_value_t{str};
        },
        [](const prim_value_t::List& list) -> value_t {
            prim_value_t::List newlist;
            for (auto val: list) {
                newlist.push_back(deepcopy(val));
            }
            return new prim_value_t{newlist};
        },
        [](const prim_value_t::Map& map) -> value_t {
            prim_value_t::Map newmap;
            for (auto [key, val]: map) {
                newmap[deepcopy(key)] = deepcopy(val);
            }
            return new prim_value_t{newmap};
        },
        [primValPtr](prim_value_t::Byte) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Bool) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Int) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Float) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Lambda) -> value_t {return primValPtr;},
    }, primValPtr->variant);
}

static value_t deepcopy(type_value_t* type_val) {
    return new type_value_t{
        type_val->typeTag,
        deepcopy(type_val->underlyingVal)
    };
}

static value_t deepcopy(struct_value_t* struct_val) {
    auto fields = std::vector<struct_value_t::Field>();
    for (const auto& field: struct_val->fields) {
        fields.push_back(
            struct_value_t::Field{
                field.type,
                field.name,
                deepcopy(field.val)
            }
        );
    }
    return new struct_value_t{
        struct_val->type,
        fields
    };
}

static value_t deepcopy(enum_value_t* enum_val) {
    return new enum_value_t{
        enum_val->type,
        enum_val->ordinal,
        enum_val->enumerator,
        deepcopy(enum_val->enumerate)
    };
}

#endif // INTERP_DEEPCOPY_H
