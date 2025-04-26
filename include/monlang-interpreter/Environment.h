
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <string>
#include <map>

struct Environment {
    using SymbolName = std::string;
    using SymbolValue = int;

    std::map<SymbolName, SymbolValue> symbolTable;
    Environment* enclosingEnv;
};

#endif // ENVIRONMENT_H
