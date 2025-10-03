#ifndef PATH_RESOLUTION_H
#define PATH_RESOLUTION_H

#include <monlang-interpreter/types.h>
#include <monlang-LV2/ast/Lvalue.h>

#include <vector>

class PathResolution {
  public:
    Lvalue path;
    Environment* pathValuesEnv = nullptr;

    using PathValues = std::vector<value_t>;
    PathValues pathValues;

    PathResolution() = default;
    PathResolution(const Lvalue& path, Environment* pathValuesEnv);

    value_t value(Environment* envAtResolution);
    value_t* lvalue(Environment* envAtResolution);
};

#endif // PATH_RESOLUTION_H
