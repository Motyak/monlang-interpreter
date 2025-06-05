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
}

const Environment::SymbolValue&
Environment::at(const SymbolName& symbolName) const {
    auto* curEnv = this;
    while (curEnv) {
        if (curEnv->symbolTable.contains(symbolName)) {
            return curEnv->symbolTable.at(symbolName);
        }
        curEnv = curEnv->enclosingEnv;
    }
    SHOULD_NOT_HAPPEN(); // should call ::contains before calling ::at
}

Environment::SymbolValue&
Environment::at(const SymbolName& symbolName) {
    auto* curEnv = this;
    while (curEnv) {
        if (curEnv->symbolTable.contains(symbolName)) {
            return curEnv->symbolTable.at(symbolName);
        }
        curEnv = curEnv->enclosingEnv;
    }
    SHOULD_NOT_HAPPEN(); // should call ::contains before calling ::at
}

// Environment* Environment::root() {
    // Environment* curEnv = this;
    // while (curEnv->enclosingEnv) {
    //     curEnv = curEnv->enclosingEnv;
    // }
//     return curEnv;
// }
