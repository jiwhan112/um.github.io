// 클로저의 복사 값, 빌린 참조, 이동 소유 상태를 비교한다.
#include <functional>
#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("capture contract");
    };
    int score = 10;
    auto snapshot = [score] {
        return score;
    };
    auto borrowed = [&score] {
        return score;
    };
    score = 30;
    check(snapshot() == 10 && borrowed() == 30);
    auto counter = [value = score]() mutable { return ++value;
    };
    check(counter() == 31 && counter() == 32 && score == 30);
    auto resource = std::make_unique<int>(7);
    auto owned = [p = std::move(resource)](int delta) mutable { *p+=delta;
    return *p;
    };
    static_assert(!std::is_copy_constructible_v<decltype(owned)>);
    check(!resource && owned(2) == 9);
    // 클로저가 보유한 unique 소유권도 함께 이전된다. 이전 closure는 재호출하지 않는다.
    auto moved = std::move(owned);
    check(moved(1) == 10);
    // moved-from owned는 더 호출하지 않는다.
    std::function<int(int)> copyable = [base = 5](int x) {
        return base+x;
    };
    check(copyable(4) == 9);
    auto generic = [](const auto& value) {
        return value.size();
    };
    check(generic(std::string("frame")) == 5);
    std::cout << "snapshot=10 borrowed=30 owned=10\nPASS: " << checks << " contract checks\n";
}
