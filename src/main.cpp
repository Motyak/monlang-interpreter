
#include <monlang_parser.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/ParseError.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/iostream-utils.h>
#include <utils/file-utils.h>
#include <utils/str-utils.h>
#include <utils/vec-utils.h>

#define unless(x) if(!(x))

[[noreturn]] int repl_main(int argc, char* argv[]);
int stdinput_main(int argc, char* argv[]);
int fileinput_main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    if (auto options = second(split_in_two(argv[0], " -"))) {
        if (options->contains("i")) {
            INTERACTIVE_MODE = true;
        }
    }

    /* delegate main based on execution mode */

    if (argc == 1) {
        return repl_main(argc, argv);
    }

    if (*argv[1] == '-') {
        return stdinput_main(argc, argv);
    }

    return fileinput_main(argc, argv);
}

int repl_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    Environment env; // persists between top-level statements

    INTERACTIVE_MODE = true;

    Read:
    auto text = slurp_stdin(/*repeatable*/true);

    Eval:
    Print:
    LV2::Program prog;
    Tokens tokens;
    try {
        prog = parse(text, &tokens);
    }
    catch (const ParseError&) {
        std::cerr << "Parsing error" << std::endl;
        goto Read;
    }
    try {
        for (auto stmt: prog.statements) {
            performStatement(stmt, &env);
        }
    }
    catch (const InterpretError& e) {
        // TODO: careful, might need rollback ?

        std::cerr << "Runtime error: " << e.what() << "\n";
        std::cerr << "Call stack:\n";
        for (auto entity: e.callStack) {
            auto token = tokens[token_id(entity)];
            // TODO: tmp
            auto fnName = std::get<Symbol*>(std::get<FunctionCall*>(std::get<Expression>(entity))->function)->name;
            std::cerr << "  " << fnName << "(line " << token.start.line;
            std::cerr << " column " << token.start.column << ")\n";
        }
        goto Read;
    }

    Loop: goto Read
    ;
}

int stdinput_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    auto text = slurp_stdin(/*repeatable*/false);
    LV2::Program prog;
    Tokens tokens;
    try {
        prog = parse(text, &tokens);
    }
    catch (const ParseError&) {
        std::cerr << "Parsing error" << std::endl;
        return 2;
    }
    try {
        interpretProgram(prog);
    }
    catch (const InterpretError& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        std::cerr << "Call stack:\n";
        for (auto entity: e.callStack) {
            auto token = tokens[token_id(entity)];
            // TODO: tmp
            auto fnName = std::get<Symbol*>(std::get<FunctionCall*>(std::get<Expression>(entity))->function)->name;
            std::cerr << "  " << fnName << "(line " << token.start.line;
            std::cerr << " column " << token.start.column << ")\n";
        }
        return 1;
    }

    return 0;
}

int fileinput_main(int argc, char* argv[]) {
    (void)argc;
    const auto filename = argv[1];

    std::string text;
    try {
        text = slurp_file(filename);
    }
    catch (const CantOpenFileException&) {
        std::cerr << "Failed to open file `" << filename << "`" << std::endl;
        return 3;
    }
    LV2::Program prog;
    Tokens tokens;
    try {
        prog = parse(text, &tokens);
    }
    catch (const ParseError&) {
        std::cerr << "Parsing error" << std::endl;
        return 2;
    }
    try {
        interpretProgram(prog);
    }
    catch (const InterpretError& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        std::cerr << "Call stack:\n";
        for (auto entity: e.callStack) {
            auto token = tokens[token_id(entity)];
            // TODO: tmp
            auto fnName = std::get<Symbol*>(std::get<FunctionCall*>(std::get<Expression>(entity))->function)->name;
            std::cerr << "  " << fnName << "(";
            std::cerr << filename << ":" << token.start.line << ":" << token.start.column;
            std::cerr << ")\n";
        }
        return 1;
    }

    return 0;
}
