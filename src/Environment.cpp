#include <monlang-interpreter/Environment.h>

#include <utils/assert-utils.h>

#define unless(x) if(!(x))

bool Environment::contains(const SymbolName& symbolName) const {
    auto* currEnv = this;
    while (currEnv) {
        if (currEnv->symbolTable.contains(symbolName)) {
            return true;
        }
        currEnv = currEnv->enclosingEnv.get();
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
        currEnv = currEnv->enclosingEnv.get();
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
        currEnv = currEnv->enclosingEnv.get();
    }
    SHOULD_NOT_HAPPEN(); // should call ::contains before calling ::at
}

std::shared_ptr<Environment> Environment::rec_copy() {
    auto newEnv = std::make_shared<Environment>(std::move(*this));
    auto* currEnv = newEnv.get();
    while (currEnv->enclosingEnv) {
        currEnv->enclosingEnv = std::make_shared<Environment>(
            std::move(*currEnv->enclosingEnv)
        );
        currEnv = currEnv->enclosingEnv.get();
    }
    return newEnv;
}

static void fork_variables(Environment* env) {
    using Variable = Environment::Variable;
    for (auto& [symName, symVal]: env->symbolTable) {
        unless (std::holds_alternative<Variable>(symVal)) continue;
        auto& var = std::get<Variable>(symVal);
        var = std::make_shared<owned_value_t>(copy_own(*var));
    }
}

std::shared_ptr<Environment> Environment::rec_deepcopy() {
    auto newEnv = std::make_shared<Environment>(std::move(*this));
    auto* currEnv = newEnv.get();
    fork_variables(currEnv);
    while (currEnv->enclosingEnv) {
        currEnv->enclosingEnv = std::make_shared<Environment>(
            std::move(*currEnv->enclosingEnv)
        );
        currEnv = currEnv->enclosingEnv.get();
        fork_variables(currEnv);
    }
    return newEnv;
}
