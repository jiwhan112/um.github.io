#include "check.hpp"
#include <cstdint>
#include <limits>
#include <optional>
#include <sstream>
std::optional<std::uint64_t> Factorial(int n) {
    if (n < 0 || n > 20) return std::nullopt;
    if (n == 0) return std::uint64_t{1};
    const auto previous = Factorial(n - 1);
    const auto factor = static_cast<std::uint64_t>(n);
    if (!previous || *previous > std::numeric_limits<std::uint64_t>::max() / factor)
        return std::nullopt;
    return *previous * factor;
}
int Add(int a, int b) { return a + b; } // Precondition: sum representable in int.
int Scale(int value, int factor = 2) { return value * factor; }
using Operation = int (*)(int, int);
int main() {
    CHECK(Factorial(0) == 1); CHECK(Factorial(5) == 120);
    CHECK(Factorial(20) == 2432902008176640000ULL);
    CHECK(!Factorial(-1)); CHECK(!Factorial(21)); CHECK(!Factorial(1000000));
    int count{};
    for (int i = 0; i < 5; ++i) { if (i == 2) continue; ++count; }
    CHECK(count == 4);
    enum class State { Idle, Running }; const State state = State::Running;
    switch (state) { case State::Idle: count = 0; break; case State::Running: count = 7; break; }
    CHECK(count == 7);
    int once{}; do { ++once; } while (false); CHECK(once == 1);
    Operation operation = &Add; CHECK(operation(3, 4) == 7);
    operation = [](int a, int b) { return a - b; }; CHECK(operation(7, 3) == 4);
    CHECK(Scale(3) == 6); CHECK(Scale(3, 4) == 12);
    std::istringstream good{"42"}, bad{"oops"}; int value{};
    CHECK(static_cast<bool>(good >> value) && value == 42);
    CHECK(!(bad >> value));
    Report();
}
