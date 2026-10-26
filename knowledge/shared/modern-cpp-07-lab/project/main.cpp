// 상수 평가 가능성과 반드시 상수 평가해야 하는 맥락을 구별한다.
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
constexpr std::uint32_t capped_add(std::uint32_t a, std::uint32_t b) {
    const auto max = std::numeric_limits<std::uint32_t>::max();
    return b>max-a ? max : a+b;
    // 더하기 전에 검사한다.
}
consteval std::array<unsigned, 4> make_costs(unsigned base) {
    return {base, base+1, base+2, base+3};
    // 이 수업은 base<=1000만 호출한다.
}
template<class T> constexpr unsigned weight(const T& value) {
    if constexpr(std::is_integral_v<T>) return value == 0 ? 0u : 1u;
    else return static_cast<unsigned>(value.size());
    // 정수 인스턴스에는 이 식이 만들어지지 않는다.
}
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("constant evaluation contract");
    };
    // 상수 식이 필요한 위치와 일반 함수 호출 위치를 분리한다.
    constexpr auto costs = make_costs(10);
    static_assert(costs[3] == 13);
    static_assert(capped_add(4, 5) == 9);
    static_assert(capped_add(UINT32_MAX, 1) == UINT32_MAX);
    static_assert(weight(0) == 0);
    std::uint32_t runtime = 7;
    check(capped_add(runtime, 5) == 12);
    // constexpr 함수도 일반 실행 맥락에서 호출 가능.
    check(capped_add(UINT32_MAX-2, 3) == UINT32_MAX);
    check(weight(std::array<int, 3>{}) == 3);
    check(weight(9) == 1 && costs[0] == 10);
    std::cout << "compile-time-table=10,11,12,13 saturated=UINT32_MAX\nPASS: " << checks << " contract checks\n";
}
