
#include <monlang_parser.h>
#include <monlang-interpreter/interpret.h>

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
    Program prog;
    try {
        prog = parse(text);
    }
    catch (const std::bad_variant_access& e) {
        std::cerr << e.what() << std::endl;
        goto Read;
    }
    for (auto stmt: prog.statements) {
        performStatement(stmt, /*OUT*/env);
    }

    Loop: goto Read
    ;
}

int stdinput_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    auto text = slurp_stdin(/*repeatable*/false);
    auto prog = parse(text);
    interpretProgram(prog);

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
        return 1;
    }
    auto prog = parse(text);
    interpretProgram(prog);

    return 0;
}
