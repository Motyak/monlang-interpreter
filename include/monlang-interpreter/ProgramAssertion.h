/*
    standalone header, no .cpp

    activeCallStack global variable is defined in src/interpret.cpp
*/
#ifndef PROGRAM_ASSERTION_H
#define PROGRAM_ASSERTION_H

#include <monlang-LV2/ast/Expression.h>

#include <exception>
#include <vector>

extern thread_local std::vector<Expression> activeCallStack;

class ProgramAssertion : public std::exception {
  public:
    std::vector<Expression> callStack;
    std::string msg;

    ProgramAssertion(std::string msg = "")
            : callStack(::activeCallStack), msg(msg){}

    const char* what() const noexcept {
        return this->msg.c_str();
    }
};

#endif // PROGRAM_ASSERTION_H
