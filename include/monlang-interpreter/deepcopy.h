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

inline value_t deepcopy(value_t val) {
    if (is_nil(val)) {
        return val;
    }
    return std::visit(overload{
        [](char*) -> value_t {SHOULD_NOT_HAPPEN();},
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
                newmap[key] = deepcopy(val);
            }
            return new prim_value_t{newmap};
        },
        [primValPtr](prim_value_t::Byte) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Bool) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Int) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Float) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Char) -> value_t {return primValPtr;},
        [primValPtr](prim_value_t::Lambda) -> value_t {return primValPtr;},
    }, primValPtr->variant);
}

static value_t deepcopy(type_value_t*) {
    TODO();
}

static value_t deepcopy(struct_value_t*) {
    TODO();
}

static value_t deepcopy(enum_value_t*) {
    TODO();
}

#endif // INTERP_DEEPCOPY_H
