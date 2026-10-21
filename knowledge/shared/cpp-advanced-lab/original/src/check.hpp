#ifndef STUDY_CHECK_HPP
#define STUDY_CHECK_HPP
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>
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
