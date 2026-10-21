#ifndef CPP_BASICS_CHECK_HPP
#define CPP_BASICS_CHECK_HPP
#include <cstdlib>
#include <iostream>
inline int checks = 0;
inline void Check(bool condition, const char* expression, int line) {
    ++checks;
    if (!condition) { std::cerr << "FAIL line " << line << ": " << expression << '\n'; std::exit(1); }
}
#define CHECK(expression) Check(static_cast<bool>(expression), #expression, __LINE__)
inline void Report() { std::cout << "PASS " << checks << " checks\n"; }
#endif
