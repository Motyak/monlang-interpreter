#ifndef MAP_KEY_CMP_H
#define MAP_KEY_CMP_H

#include <variant>

// forward declare value_t ////////////////////

struct prim_value_t;
struct type_value_t;
struct struct_value_t;
struct enum_value_t;
struct FieldLvalue;

using value_t = std::variant<
    prim_value_t*, // primitive
    /* user-defined */
    type_value_t*,
    struct_value_t*,
    enum_value_t*,

    char*, // for evaluating Str subscript as lvalue
    FieldLvalue* // for evaluating FieldAccess as lvalue
>;

///////////////////////////////////////////////

class MapKeyCmp {
  public:
    bool operator()(const value_t&, const value_t&) const;
};

#endif // MAP_KEY_CMP_H
