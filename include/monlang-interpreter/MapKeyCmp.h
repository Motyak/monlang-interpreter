#ifndef MAP_KEY_CMP_H
#define MAP_KEY_CMP_H

#include <variant>
#include <memory>

// forward declare owned_value_t ////////////////////

struct prim_value_t;
struct type_value_t;
struct struct_value_t;
struct enum_value_t;

using owned_value_t = std::variant<
    std::unique_ptr<prim_value_t>,
    std::unique_ptr<type_value_t>,
    std::unique_ptr<struct_value_t>,
    std::unique_ptr<enum_value_t>
>;

///////////////////////////////////////////////

class MapKeyCmp {
  public:
    bool operator()(const owned_value_t&, const owned_value_t&) const;
};

#endif // MAP_KEY_CMP_H
