/*
    standalone header, no .cpp

    activeCallStack global variable is defined in src/interpret.cpp
*/
#ifndef INTERPRET_ERROR_H
#define INTERPRET_ERROR_H

#include <monlang-LV2/ast/Expression.h>

#include <exception>
#include <vector>

extern thread_local std::vector<Expression> activeCallStack;

#define InterpretError(msg) InterpretError{__FILE__, __LINE__, new std::string(msg)}

#define WrongNbOfArgsError(params_vec, args_vec) \
    InterpretError{__FILE__, __LINE__, new std::string(\
            "Lambda defined with " \
            + std::to_string(params_vec.size()) \
            + (params_vec.size() <= 1? " param" : " params") \
            + ", but called with " \
            + std::to_string(args_vec.size()) \
            + (args_vec.size() <= 1? " arg" : " args") \
    )}

#define DuplicateParamError(param_name) \
    InterpretError{__FILE__, __LINE__, new std::string("Duplicate Lambda parameter `" + param_name + "`")}

#define SymbolRedefinitionError(symbol_name) \
    InterpretError{__FILE__, __LINE__, new std::string("Redefinition of symbol `" + symbol_name + "`")}

#define StructWrongNbOfArgsError(ctorTypes, args) \
    InterpretError{__FILE__, __LINE__, new std::string(\
            "ctor defined with " \
            + std::to_string(ctorTypes.size()) \
            + (ctorTypes.size() <= 1? " param" : " params") \
            + ", but called with " \
            + std::to_string(args.size()) \
            + (args.size() <= 1? " arg" : " args") \
    )}

#define StructFieldTypeError(argType, fieldType) \
    InterpretError{__FILE__, __LINE__, new std::string(\
            "struct field of type `" \
            + fieldType + "` but assigned with arg of type `" \
            + argType + "`" \
    )}

class InterpretError : public std::exception {
  public:
    std::vector<Expression> callStack;
    std::string msg;

    // we use std::string* to avoid allocating strings on the stack where we throw err
    (InterpretError)(const char* _cpp_file, int _cpp_line, const std::string* msg_ptr)
            : callStack(::activeCallStack)
    {
        std::string msg = *msg_ptr;
        this->msg += msg;
        if (msg.size() > 0) this->msg += " (";
        this->msg += _cpp_file;
        this->msg += ":";
        this->msg += std::to_string(_cpp_line);
        if (msg.size() > 0) this->msg += ")";
    }

    const char* what() const noexcept {
        return this->msg.c_str();
    }
};

#endif // INTERPRET_ERROR_H
