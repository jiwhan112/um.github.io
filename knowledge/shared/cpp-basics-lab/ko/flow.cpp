// C++ 기초 02 · 원본 flow.cpp의 실행 토큰을 유지한 한국어 주석판입니다.
// 공통 검사 함수는 original/src/check.hpp에 있습니다. 이 파일에는 15개의 런타임 검사가 있습니다.
#include "check.hpp"
#include <cstdint>
#include <limits>
#include <optional>
#include <sstream>

// 값이 있거나 없는 결과를 optional로 표현합니다. 입력 범위를 먼저 제한해 재귀가 과도하게 깊어지지 않습니다.
std::optional<std::uint64_t> Factorial(int n) {
    if (n < 0 || n > 20)
        return std::nullopt;
    if (n == 0)
        return std::uint64_t{1};

    // 더 작은 입력의 답을 먼저 구합니다. n == 0은 위에서 반환했으므로 여기의 factor는 0이 아닙니다.
    const auto previous = Factorial(n - 1);
    const auto factor = static_cast<std::uint64_t>(n);

    // 곱한 뒤 검사하면 이미 범위를 넘었을 수 있습니다. 최댓값을 나눈 경계와 먼저 비교합니다.
    // ||는 previous가 비었으면 오른쪽 역참조를 평가하지 않습니다.
    if (!previous || *previous > std::numeric_limits<std::uint64_t>::max() / factor)
        return std::nullopt;
    return *previous * factor;
}

// 합이 int로 표현될 수 있는 작은 입력을 전제로 합니다. 범용 오버플로 방어 함수가 아닙니다.
int Add(int a, int b) {
    return a + b;
}

// 호출자가 두 번째 인수를 생략하면 2를 사용합니다. 곱도 int 범위 안에 있어야 합니다.
int Scale(int value, int factor = 2) {
    return value * factor;
}

// int 두 개를 받아 int를 돌려주는 함수의 주소를 담는 타입입니다.
using Operation = int (*)(int, int);

int main() {
    // 정상 경계와 입력 거부를 함께 확인합니다. 20!까지가 이 예제에서 허용한 범위입니다.
    CHECK(Factorial(0) == 1);
    CHECK(Factorial(5) == 120);
    CHECK(Factorial(20) == 2432902008176640000ULL);
    CHECK(!Factorial(-1));
    CHECK(!Factorial(21));
    CHECK(!Factorial(1000000));

    // continue는 이번 반복의 나머지를 건너뜁니다. i == 2일 때만 count를 증가시키지 않습니다.
    int count{};
    for (int i = 0; i < 5; ++i) {
        if (i == 2)
            continue;
        ++count;
    }
    CHECK(count == 4);

    // 상태별 분기를 나누고 break로 switch를 끝냅니다.
    enum class State { Idle, Running };
    const State state = State::Running;
    switch (state) {
        case State::Idle:
            count = 0;
            break;
        case State::Running:
            count = 7;
            break;
    }
    CHECK(count == 7);

    // do-while은 본문을 실행한 다음 조건을 봅니다. false여도 본문은 한 번 실행됩니다.
    int once{};
    do {
        ++once;
    } while (false);
    CHECK(once == 1);

    // 같은 함수 포인터 변수로 덧셈 함수와 캡처 없는 람다를 차례로 호출합니다.
    Operation operation = &Add;
    CHECK(operation(3, 4) == 7);
    operation = [](int a, int b) { return a - b; };
    CHECK(operation(7, 3) == 4);
    CHECK(Scale(3) == 6);
    CHECK(Scale(3, 4) == 12);

    // 입력이 성공했는지 확인한 뒤 값을 사용합니다. 숫자가 아닌 입력은 스트림 실패 상태로 구별합니다.
    std::istringstream good{"42"}, bad{"oops"};
    int value{};
    CHECK(static_cast<bool>(good >> value) && value == 42);
    CHECK(!(bad >> value));
    Report();
}
