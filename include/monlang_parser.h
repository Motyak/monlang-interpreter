#ifndef MONLANG_PARSER_H
#define MONLANG_PARSER_H

#include <monlang-LV2/ast/Program.h>

#include <string>

using Program = LV2::Program;
Program parse(const std::string&);

#endif // MONLANG_PARSER_H
