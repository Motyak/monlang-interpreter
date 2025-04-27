#ifndef INTERPRET_H
#define INTERPRET_H

#include <monlang-interpreter/Environment.h>

#include <monlang-LV2/ast/Program.h>

using namespace LV2;

void interpret(const Program&, Environment&);
void interpret(const Program&);

#endif // INTERPRET_H
