#include <monlang-interpreter/PathResolution.h>
#include <monlang-interpreter/interpret.h>

namespace {

/* visitors */

class EvaluateValue {
  public:
    PathResolution* pathResolution;
    Environment* envAtResolution;

    value_t operator()(); // doit
    value_t operator()(Subscript*);
    value_t operator()(FieldAccess*);
    value_t operator()(Symbol*);
};

class EvaluateLvalue {
  public:
    PathResolution* pathResolution;
    Environment* envAtResolution;

    value_t* operator()(); // doit
    value_t* operator()(Subscript*);
    value_t* operator()(FieldAccess*);
    value_t* operator()(Symbol*);
};

} // end of anonymous namespace

value_t PathResolution::value(Environment* envAtResolution) {
    auto evaluateValue = EvaluateValue{this, envAtResolution};
    return evaluateValue();
}

value_t* PathResolution::lvalue(Environment* envAtResolution) {
    auto evaluateLvalue = EvaluateLvalue{this, envAtResolution};
    return evaluateLvalue();
}

PathResolution::PathResolution(const Lvalue& path, Environment* pathValuesEnv)
        : path(path), pathValuesEnv(pathValuesEnv){}

//==============================================================
// EvaluateValue
//==============================================================

value_t EvaluateValue::operator()() {
    return std::visit(*this, pathResolution->path.variant);
}

value_t EvaluateValue::operator()(Subscript* subscript) {
    auto arrayVal = std::visit(*this, Lvalue(subscript->array).variant);
    return evaluateValue(*subscript, pathResolution->pathValuesEnv, arrayVal);
}

value_t EvaluateValue::operator()(FieldAccess* fieldAccess) {
    auto objectVal = std::visit(*this, Lvalue(fieldAccess->object).variant);
    // FieldAccess path value is always a literal, so we can..
    // ..simply evaluate FieldAccess with respect with envAtResolution
    return evaluateValue(*fieldAccess, envAtResolution, objectVal);
}

// base case
value_t EvaluateValue::operator()(Symbol* symbol) {
    return evaluateValue(*symbol, envAtResolution);
}

//==============================================================
// EvaluateLvalue
//==============================================================

value_t* EvaluateLvalue::operator()() {
    return std::visit(*this, pathResolution->path.variant);
}

value_t* EvaluateLvalue::operator()(Subscript* subscript) {
    auto arrayVal = std::visit(*this, Lvalue(subscript->array).variant);
    return evaluateLvalue(*subscript, pathResolution->pathValuesEnv, arrayVal);
}

value_t* EvaluateLvalue::operator()(FieldAccess* fieldAccess) {
    auto objectVal = std::visit(*this, Lvalue(fieldAccess->object).variant);
    // FieldAccess path value is always a literal, so we can..
    // ..simply evaluate FieldAccess with respect with envAtResolution
    return evaluateLvalue(*fieldAccess, envAtResolution, objectVal);
}

// base case
value_t* EvaluateLvalue::operator()(Symbol* symbol) {
    return evaluateLvalue(*symbol, envAtResolution);
}
