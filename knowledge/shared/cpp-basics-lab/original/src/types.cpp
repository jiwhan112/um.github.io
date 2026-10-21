#include "check.hpp"
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
int main() {
    static_assert(sizeof(char) == 1);
    static_assert(CHAR_BIT >= 8);
    static_assert(std::is_same_v<decltype(10.0), double>);
    static_assert(std::is_same_v<decltype(10.0f), float>);
    int zero{};
    int hp{100}; hp = 80;
    CHECK(zero == 0); CHECK(hp == 80);
    CHECK(0b00000010 == 2); CHECK((0b0100 ^ 0b0011) == 7); CHECK(0132 == 90);
    CHECK(7 / 2 == 3); CHECK(7.0 / 2 == 3.5);
    int i{5}; const int old = i++; const int now = ++i;
    CHECK(old == 5); CHECK(now == 7); CHECK(i == 7);
    unsigned int n = std::numeric_limits<unsigned int>::max();
    CHECK(n + 1u == 0u); // Unsigned arithmetic, not signed overflow.
    std::uint32_t flags{0}; constexpr std::uint32_t burning = std::uint32_t{1} << 2;
    flags |= burning; CHECK((flags & burning) != 0);
    flags &= ~burning; CHECK((flags & burning) == 0);
    bool evaluated = false;
    const bool result = false && (evaluated = true);
    CHECK(!result && !evaluated);
    CHECK(std::abs((0.1 + 0.2) - 0.3) < 1e-12);
    std::cout << "CHAR_BIT=" << CHAR_BIT << " sizeof(int)=" << sizeof(int)
              << " sizeof(long)=" << sizeof(long) << " sizeof(void*)=" << sizeof(void*)
              << " sizeof(wchar_t)=" << sizeof(wchar_t) << '\n';
    Report();
}
