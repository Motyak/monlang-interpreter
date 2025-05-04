#include <monlang-interpreter/Environment.h>

#include <utils/assert-utils.h>

bool Environment::contains(const SymbolName& symbolName) const {
    const Environment* curEnv = this;
    while (curEnv) {
        if (curEnv->symbolTable.contains(symbolName)) {
            return true;
        }
        curEnv = curEnv->enclosingEnv;
    }
    return false;

    // return this->symbolTable.contains(symbolName); //
}

Environment::SymbolValue&
Environment::at(const SymbolName& symbolName) {
    Environment* curEnv = this;
    while (curEnv) {
        if (curEnv->symbolTable.contains(symbolName)) {
            return curEnv->symbolTable.at(symbolName);
        }
        curEnv = curEnv->enclosingEnv;
    }
    SHOULD_NOT_HAPPEN(); // should call ::contains before calling ::at
}

Environment::SymbolValue
Environment::at(const SymbolName& symbolName) const {
    const Environment* curEnv = this;
    while (curEnv) {
        if (curEnv->symbolTable.contains(symbolName)) {
            return curEnv->symbolTable.at(symbolName);
        }
        curEnv = curEnv->enclosingEnv;
    }
    SHOULD_NOT_HAPPEN(); // should call ::contains before calling ::at
}
