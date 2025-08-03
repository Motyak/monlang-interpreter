#include <monlang-interpreter/Environment.h>

#include <utils/assert-utils.h>

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

Environment* Environment::deepcopy() {
    auto* newEnv = new Environment{*this};
    auto* currEnv = newEnv;
    while (currEnv->enclosingEnv) {
        currEnv->enclosingEnv = new Environment{*currEnv->enclosingEnv};
        currEnv = currEnv->enclosingEnv;
    }
    return newEnv;
}
