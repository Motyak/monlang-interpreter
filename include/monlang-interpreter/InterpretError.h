/*
    standalone header, no .cpp

    activeCallStack global variable is defined in src/interpret.cpp
*/
#ifndef INTERPRET_ERROR_H
#define INTERPRET_ERROR_H

#include <monlang-LV2/ast/visitors/visitor.h>

#include <exception>
#include <vector>

extern thread_local std::vector<LV2::Ast> activeCallStack;

#define InterpretError(...) InterpretError{__FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__}

class InterpretError : public std::exception {
  public:
    std::vector<LV2::Ast> callStack;
    std::string msg;

    (InterpretError)(const char* _cpp_file, int _cpp_line, const char* msg = "")
            : callStack(activeCallStack)
    {
        this->msg += msg;
        if (*msg) this->msg += " (";
        this->msg += _cpp_file;
        this->msg += ":";
        this->msg += std::to_string(_cpp_line);
        if (*msg) this->msg += ")";
    }

    const char* what() const noexcept {
        return this->msg.c_str();
    }
};

#endif // INTERPRET_ERROR_H
