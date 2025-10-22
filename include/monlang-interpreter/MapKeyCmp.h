#ifndef MAP_KEY_CMP_H
#define MAP_KEY_CMP_H

#include <variant>
#include <memory>

// forward declare value_t ////////////////////

struct prim_value_t;
struct type_value_t;
struct struct_value_t;
struct enum_value_t;

using value_t = std::variant<
    std::unique_ptr<prim_value_t>, // primitive
    /* user-defined */
    type_value_t*,
    struct_value_t*,
    enum_value_t*,
    /* for evaluating Str subscript as lvalue */
    char*
>;

///////////////////////////////////////////////

class MapKeyCmp {
  public:
    bool operator()(const value_t&, const value_t&) const;
};

#endif // MAP_KEY_CMP_H
