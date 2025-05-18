
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

static void reportCallStack(
    const std::vector<Expression>& callStack,
    const Tokens&,
    const std::string& filename = "<str>"
);

int repl_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    Environment env; // persists between top-level statements

    INTERACTIVE_MODE = true;

    Read:
    auto text = slurp_stdin(/*repeatable*/true);

    Eval:
    Print:
    Program prog;
    try {
        prog = parse(text); // we don't reconstruct tokens in REPL
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
        std::cerr << "Runtime error: " << e.what() << "\n";

        // we can't report the call stack because each new..
        // ..user prompt is interpreted as an individual program..
        // ..therefore we can't point to a previous prompt token

        ::activeCallStack = {}; // discard it
        goto Read;
    }

    Loop: goto Read
    ;
}

int stdinput_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    auto text = slurp_stdin(/*repeatable*/false);
    Program prog;
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
        reportCallStack(e.callStack, tokens);
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
    Program prog;
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
        reportCallStack(e.callStack, tokens, filename);
        return 1;
    }

    return 0;
}

static void reportCallStack(
    const std::vector<Expression>& callStack,
    const Tokens& tokens,
    const std::string& filename
)
{
    // maps fnName to token location in source
    std::vector<std::pair<std::string, std::string>>
    fixedCallStack; // for reporting

    auto prevIterFnName = std::string("<program>");
    for (auto expr: callStack) {
        auto token = tokens.get(token_id(expr));
        auto fnName = prevIterFnName;
        auto tokenLocation = filename
                + ":" + std::to_string(token.start.line)
                + ":" + std::to_string(token.start.column);
        fixedCallStack.push_back({fnName, tokenLocation});

        // setup prevIterFnName for next iteration
        if (std::holds_alternative<FunctionCall*>(expr)) {
            auto fnCall = *std::get<FunctionCall*>(expr);
            auto fnName = std::get<Symbol*>(std::get<FunctionCall*>(expr)->function)->name;
            prevIterFnName = fnName;
        }
        else {
            prevIterFnName = "<block>";
        }
    }

    for (int i = fixedCallStack.size() - 1; i >= 0; --i) {
        auto [fnName, tokenLocation] = fixedCallStack[i];
        std::cerr << "    at " << fnName << "(" << tokenLocation << ")\n";
    }
}
