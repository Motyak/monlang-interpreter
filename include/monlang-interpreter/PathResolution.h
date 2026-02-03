#ifndef PATH_RESOLUTION_H
#define PATH_RESOLUTION_H

#include <monlang-interpreter/types.h>
#include <monlang-LV2/ast/Lvalue.h>

#include <vector>
#include <any>

class PathResolution {
  private:
    Lvalue path;
    Environment* pathValuesEnv = nullptr;

    using PathValues = std::vector<std::any>;
    PathValues pathValues;

    int nthSubscript = int(); // start at zero

    value_t evaluateValue(const Lvalue&, Environment* envAtResolution);
    value_t evaluateValue(const Subscript&, Environment* envAtResolution);
    value_t evaluateValue(const FieldAccess&, Environment* envAtResolution);
    value_t evaluateValue(const Symbol&, Environment* envAtResolution);

    value_t* evaluateLvalue(const Lvalue&, Environment* envAtResolution, bool subscripted = false);
    value_t* evaluateLvalue(const Subscript&, Environment* envAtResolution);
    value_t* evaluateLvalue(const FieldAccess&, Environment* envAtResolution);
    value_t* evaluateLvalue(const Symbol&, Environment* envAtResolution, bool subscripted = false);

    value_t createPaths(const Lvalue&, Environment* envAtResolution);
    value_t createPaths(const Subscript&, Environment* envAtResolution);
    value_t createPaths(const FieldAccess&, Environment* envAtResolution);

  public:
    PathResolution() = default;
    PathResolution(const Lvalue& path, Environment* pathValuesEnv);

    value_t value(Environment* envAtResolution);
    value_t* lvalue(Environment* envAtResolution);
    value_t createPaths(Environment* envAtResolution);
};

#endif // PATH_RESOLUTION_H
