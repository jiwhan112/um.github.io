// C++ 기초 01 · 원본 types.cpp의 실행 토큰을 유지한 한국어 주석판입니다.
// 공통 검사 함수는 original/src/check.hpp에 있습니다. C++20으로 함께 빌드합니다.
#include "check.hpp"
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

int main() {
    // 아래 네 조건은 컴파일할 때 검사합니다. Report()의 런타임 15회와 따로 셉니다.
    // sizeof의 단위는 char 한 개이고, 한 바이트의 비트 수는 CHAR_BIT로 확인합니다.
    static_assert(sizeof(char) == 1);
    static_assert(CHAR_BIT >= 8);
    static_assert(std::is_same_v<decltype(10.0), double>);
    static_assert(std::is_same_v<decltype(10.0f), float>);

    // 빈 중괄호는 int를 0으로 초기화합니다. 선언 뒤의 '='는 기존 객체에 새 값을 대입합니다.
    int zero{};
    int hp{100};
    hp = 80;
    CHECK(zero == 0);
    CHECK(hp == 80);

    // 0b 접두사는 2진수, 앞의 0은 8진수입니다. 비트 XOR는 서로 다른 비트만 1로 만듭니다.
    CHECK(0b00000010 == 2);
    CHECK((0b0100 ^ 0b0011) == 7);
    CHECK(0132 == 90);

    // 두 피연산자가 정수이면 정수 나눗셈입니다. 7.0을 쓰면 double 나눗셈이 됩니다.
    CHECK(7 / 2 == 3);
    CHECK(7.0 / 2 == 3.5);

    // 서로 다른 문장으로 분리해 증가 순서를 분명히 합니다. 후위는 이전 값, 전위는 증가한 값입니다.
    int i{5};
    const int old = i++;
    const int now = ++i;
    CHECK(old == 5);
    CHECK(now == 7);
    CHECK(i == 7);

    // unsigned의 모듈러 산술을 확인합니다. signed 정수의 넘침에도 같은 규칙이 적용되지는 않습니다.
    unsigned int n = std::numeric_limits<unsigned int>::max();
    CHECK(n + 1u == 0u);

    // 2번 비트 하나를 상태 플래그로 정합니다. OR로 켜고, 반전한 마스크와 AND하여 끕니다.
    std::uint32_t flags{0};
    constexpr std::uint32_t burning = std::uint32_t{1} << 2;
    flags |= burning;
    CHECK((flags & burning) != 0);
    flags &= ~burning;
    CHECK((flags & burning) == 0);

    // &&의 왼쪽이 false이면 오른쪽은 평가하지 않습니다. evaluated에 대입할 일도 없습니다.
    bool evaluated = false;
    const bool result = false && (evaluated = true);
    CHECK(!result && !evaluated);

    // 이진 부동소수점의 근삿값을 작은 절대 오차로 비교합니다. 모든 수치 문제의 공통 허용 오차는 아닙니다.
    CHECK(std::abs((0.1 + 0.2) - 0.3) < 1e-12);

    // 타입 크기는 실행 환경에서 관찰한 값입니다. 다른 플랫폼에서도 같은 크기라고 일반화하지 않습니다.
    std::cout << "CHAR_BIT=" << CHAR_BIT << " sizeof(int)=" << sizeof(int)
              << " sizeof(long)=" << sizeof(long) << " sizeof(void*)=" << sizeof(void*)
              << " sizeof(wchar_t)=" << sizeof(wchar_t) << '\n';
    Report();
}
