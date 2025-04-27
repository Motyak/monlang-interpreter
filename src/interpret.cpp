#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/InterpretVisitor.h>

void interpret(const Program& prog, Environment& env) {
    auto interpret = InterpretVisitor(/*OUT*/env);
    interpret(prog);
}

void interpret(const Program& prog) {
    Environment env;
    interpret(prog, env);
}
