/*
    standalone header, no .cpp
*/
#ifndef EARLY_EXIT_H
#define EARLY_EXIT_H

#include <exception>

class EarlyExit : public std::exception {
  public:
    int exitCode;

    EarlyExit(int exitCode = 0) : exitCode(exitCode){}
};

#endif // EARLY_EXIT_H
