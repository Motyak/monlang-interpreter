#include <monlang-interpreter/Environment.h>

#include <utils/assert-utils.h>

Environment::DelayedPassedByRef::DelayedPassedByRef(const std::function<value_t()>& pull)
        : pull(pull){}


value_t Environment::DelayedPassedByRef::value() {
    if (this->_variable == nullptr) {
        this->_variable = new value_t{this->pull()};
    }
    return *this->_variable;
}
    
value_t* Environment::DelayedPassedByRef::lvalue() {
    if (this->_variable == nullptr) {
        this->_variable = new value_t{};
    }
    return this->_variable;
}

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
