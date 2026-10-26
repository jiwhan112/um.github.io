// 초기화 문법이 생성자 선택을 바꾼다. 외부 정수는 enum으로 무조건 캐스팅하지 않는다.
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
enum class Mode : std::uint8_t { Walk = 0, Run = 1 };
std::optional<Mode> decode_mode(unsigned value) {
    if (value == 0) return Mode::Walk;
    if (value == 1) return Mode::Run;
    return std::nullopt;
}
int select(int) {
    return 1;
}
int select(const char*) {
    return 2;
}
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("initialization contract");
    };
    // 괄호는 개수/초깃값, 중괄호는 원소 나열을 선택한다.
    std::vector<int> repeated(3, 7);
    std::vector<int> listed{3, 7};
    check(repeated.size() == 3 && repeated[2] == 7);
    check(listed.size() == 2 && listed[0] == 3);
    auto single{7};
    auto many = {7};
    static_assert(std::is_same_v<decltype(single), int>);
    static_assert(std::is_same_v<decltype(many), std::initializer_list<int>>);
    check(select(0) == 1 && select(nullptr) == 2);
    check(decode_mode(0) == Mode::Walk);
    check(decode_mode(1) == Mode::Run);
    check(!decode_mode(2) && !decode_mode(256));
    // int bad{3.5}; 는 의도적인 컴파일 오류. 실행 가능한 코드에는 넣지 않는다.
    const double speed = 3.5;
    const int truncated = static_cast<int>(speed);
    // 작은 유한 값이라는 전제의 명시적 변환.
    check(truncated == 3);
    std::cout << "parentheses=3-elements braces=2-elements invalid-mode=rejected\nPASS: " << checks << " contract checks\n";
}
