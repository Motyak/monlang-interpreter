
#include <monlang_parser.h>
#include <monlang-interpreter/interpret.h>
#include <monlang-interpreter/ParseError.h>
#include <monlang-interpreter/InterpretError.h>

#include <utils/iostream-utils.h>
#include <utils/file-utils.h>
#include <utils/str-utils.h>
#include <utils/vec-utils.h>
#include <utils/env-utils.h>
#include <utils/args-utils.h>

#include <cstring>

#define unless(x) if(!(x))

static std::string STDIN_SRCNAME = env_or_default("STDIN_SRCNAME", "<stdin>");

[[noreturn]] int repl_main(int argc, char* argv[]);
int stdinput_main(int argc, char* argv[]);
int fileinput_main(int argc, char* argv[]);
int selfinput_main(int argc, char* argv[]);

class SelfInputException : public std::exception{};

int main(int argc, char* argv[])
{
    if (auto options = second(split_in_two(argv[0], " -"))) {
        if (options->contains("i")) {
            INTERACTIVE_MODE = true;
        }
    }

    ARG0 = argv[0];
    auto ARGS = split_args_in_two(argc, argv, "--").first;
    SRC_ARGS = split_args_in_two(argc, argv, "--").second;

    /* delegate main based on execution mode */

    if (ARGS.size() == 0) {
        if (INTERACTIVE_MODE) {
            return repl_main(argc, argv);
        }
        try {
            return selfinput_main(argc, argv);
        }
        catch (const SelfInputException&) {
            return repl_main(argc, argv);
        }
    }

    if (ARGS[0] == "-") {
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
        return 102;
    }
    try {
        interpretProgram(prog);
    }
    catch (const InterpretError& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        reportCallStack(e.callStack, tokens, STDIN_SRCNAME);
        return 101;
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
        return 103;
    }
    Program prog;
    Tokens tokens;
    try {
        prog = parse(text, &tokens);
    }
    catch (const ParseError&) {
        std::cerr << "Parsing error" << std::endl;
        return 102;
    }
    try {
        interpretProgram(prog);
    }
    catch (const InterpretError& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        reportCallStack(e.callStack, tokens, filename);
        return 101;
    }

    return 0;
}

int selfinput_main(int argc, char* argv[]) {
    (void)argc;
    const auto self_file = argv[0];

    char magic_sig[7];
    uint32_t src_size;
    char* src = nullptr;

    auto ifs = std::ifstream(self_file, std::ios::binary);
    unless (ifs.is_open()) throw SelfInputException(); // could be permissions issue, ..
    ifs.seekg(-(sizeof(magic_sig) + sizeof(src_size)), std::ios::end);
    unless (!ifs.fail()) throw SelfInputException(); // file is too short to conform
    ifs.read(magic_sig, sizeof(magic_sig));
    unless (ifs.gcount() == sizeof(magic_sig)) throw SelfInputException(); // failed to read file (magic_sig)
    unless (std::strncmp("monlang", magic_sig, sizeof(magic_sig)/sizeof(magic_sig[0])) == 0) throw SelfInputException(); // no magic sig
    ifs.read((char*)&src_size, sizeof(src_size));
    unless (ifs.gcount() == sizeof(src_size)) throw SelfInputException(); // failed to read file (src_size)
    ifs.seekg(-(src_size + sizeof(magic_sig) + sizeof(src_size)), std::ios::end);
    unless (!ifs.fail()) throw SelfInputException(); // // incorrect src size
    src = new char[src_size];
    ifs.read(src, src_size);
    unless (ifs.gcount() == src_size) throw SelfInputException(); // failed to read file (src)

    Program prog;
    Tokens tokens;
    try {
        prog = parse(src, &tokens);
    }
    catch (const ParseError&) {
        std::cerr << "Parsing error" << std::endl;
        return 102;
    }
    try {
        interpretProgram(prog);
    }
    catch (const InterpretError& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        reportCallStack(e.callStack, tokens);
        return 101;
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
            if (std::holds_alternative<Symbol*>(fnCall.function)) {
                prevIterFnName = std::get<Symbol*>(fnCall.function)->name;
            }
        }
    }

    for (int i = fixedCallStack.size() - 1; i >= 0; --i) {
        auto [fnName, tokenLocation] = fixedCallStack[i];
        std::cerr << "    at " << fnName << "(" << tokenLocation << ")\n";
    }
}
