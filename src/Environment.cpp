#include <monlang-interpreter/Environment.h>
#include <monlang-interpreter/deepcopy.h>

#include <utils/assert-utils.h>

#define unless(x) if(!(x))

bool Environment::contains(const SymbolName& symbolName) const {
    auto* currEnv = this;
    while (currEnv) {
        if (currEnv->symbolTable.contains(symbolName)) {
            return true;
        }
        currEnv = currEnv->enclosingEnv;
    }
    return false;
}

const Environment::SymbolValue&
Environment::at(const SymbolName& symbolName) const {
    auto* currEnv = this;
    while (currEnv) {
        if (currEnv->symbolTable.contains(symbolName)) {
            return currEnv->symbolTable.at(symbolName);
        }
        currEnv = currEnv->enclosingEnv;
    }
    SHOULD_NOT_HAPPEN(); // should call ::contains before calling ::at
}

Environment::SymbolValue&
Environment::at(const SymbolName& symbolName) {
    auto* currEnv = this;
    while (currEnv) {
        if (currEnv->symbolTable.contains(symbolName)) {
            return currEnv->symbolTable.at(symbolName);
        }
        currEnv = currEnv->enclosingEnv;
    }
    SHOULD_NOT_HAPPEN(); // should call ::contains before calling ::at
}

Environment* Environment::rec_copy() {
    auto* newEnv = new Environment{*this};
    auto* currEnv = newEnv;
    while (currEnv->enclosingEnv) {
        currEnv->enclosingEnv = new Environment{*currEnv->enclosingEnv};
        currEnv = currEnv->enclosingEnv;
    }
    return newEnv;
}

static void fork_variables(Environment* env) {
    using Variable = Environment::Variable;
    for (auto& [symName, symVal]: env->symbolTable) {
        unless (std::holds_alternative<Variable>(symVal)) continue;
        auto& var = std::get<Variable>(symVal);
        var = new value_t(deepcopy(*var));
    }
}

Environment* Environment::rec_deepcopy() {
    auto* newEnv = new Environment{*this};
    auto* currEnv = newEnv;
    fork_variables(currEnv);
    while (currEnv->enclosingEnv) {
        currEnv->enclosingEnv = new Environment{*currEnv->enclosingEnv};
        currEnv = currEnv->enclosingEnv;
        fork_variables(currEnv);
    }
    return newEnv;
}
