#ifndef STUDY_CHECK_HPP
#define STUDY_CHECK_HPP
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>

// 한 프로그램에서 실제로 확인한 조건의 수를 센다.
// 이 카운터는 경로 커버리지나 독립 기능 수를 측정하지 않는다.
class Checks {
    int count_{};
public:
    void Expect(bool condition, const char* label) {
        ++count_;
        if (!condition) throw std::runtime_error(label);
    }
    template<class E, class F> void Throws(F&& f, const char* label) {
        bool matched = false;
        try { std::invoke(std::forward<F>(f)); }
        catch (const E&) { matched = true; }
        // 예외를 예상하는 실험도 Expect를 한 번 호출한다.
        // E와 무관한 예외는 여기서 잡지 않고 바깥 실행 경계로 전달한다.
        Expect(matched, label);
    }
    int Count() const noexcept { return count_; }
};

template<class F> int Run(const char* name, F&& test) {
    try {
        Checks checks;
        std::invoke(std::forward<F>(test), checks);
        std::cout << "PASS " << name << ": " << checks.Count() << " checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL " << name << ": " << e.what() << '\n';
        return 1;
    }
}
#endif
