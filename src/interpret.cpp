#include <monlang-interpreter/interpret.h>
// #include <monlang-interpreter/visitors/Interpret.h>

void interpret(const Program& prog, Environment& env) {
    // auto interpret = Interpret(/*OUT*/env);
    // interpret(prog);
}

void interpret(const Program& prog) {
    Environment env;
    interpret(prog, env);
}
