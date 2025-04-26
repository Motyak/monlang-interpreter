#ifndef INTERPRET_VISITOR_H
#define INTERPRET_VISITOR_H

#include <monlang-interpreter/Environment.h>

#include <monlang-LV2/ast/Program.h>

using Program = LV2::Program;

class InterpretVisitor {
  public:
    InterpretVisitor(Environment&);

    void operator()(const Program&);
};

#endif // INTERPRET_VISITOR_H
