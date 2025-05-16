/*
    standalone header, no .cpp
*/
#ifndef PARSE_ERROR_H
#define PARSE_ERROR_H

#include <exception>

class ParseError : public std::exception {};

#endif // PARSE_ERROR_H
