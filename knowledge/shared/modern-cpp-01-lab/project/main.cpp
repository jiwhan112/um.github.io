// 언어 모드와 라이브러리 기능을 따로 확인하는 최소 진단 프로그램.
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <version>
#if __cplusplus < 202002L
#error This lesson requires C++20 mode
#endif
#ifndef __cpp_lib_span
#error This standard library does not advertise span
#endif
#include <span>
#include <array>
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("feature contract");
    };
    static_assert(__cplusplus >= 202002L);
    static_assert(__cpp_lib_span >= 202002L);
    // 소유 저장소는 frame이다. view는 같은 세 원소를 빌린다.
    std::array<int, 3> frame{3, 5, 8};
    std::span<int> view{frame};
    check(view.size() == 3);
    view[1] = 7;
    check(frame[1] == 7);
    // 헤더 존재뿐 아니라 실제 사용하는 식도 빌드/실행한다.
    check(std::is_same_v<decltype(view)::element_type, int>);
    std::cout << "mode=C++20-or-later span=available\nPASS: " << checks << " contract checks\n";
}
