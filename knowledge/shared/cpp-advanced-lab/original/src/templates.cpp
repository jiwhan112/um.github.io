#include "check.hpp"
#include <concepts>
#include <cstddef>
#include <functional>
#include <list>
#include <string>
#include <type_traits>
#include <vector>
template<class T> concept Sized = requires(const T& value) {
    { value.size() } -> std::convertible_to<std::size_t>;
};
template<Sized T> std::size_t SizeOf(const T& value) { return static_cast<std::size_t>(value.size()); }
std::string Category(std::string&) { return "mutable lvalue"; }
std::string Category(const std::string&) { return "const lvalue"; }
std::string Category(std::string&&) { return "rvalue"; }
template<class T> std::string ForwardCategory(T&& value) { return Category(std::forward<T>(value)); }
template<class T> struct Base { int Read() const { return 42; } };
template<class T> struct Derived : Base<T> { int ReadWithLog() const { return this->Read(); } };
template<class Container> typename Container::value_type FirstCopy(const Container& values) {
    if (values.empty()) throw std::out_of_range("empty");
    return *values.begin();
}
template<class T> struct Borrow {
    T* pointer;
    explicit Borrow(T* p) : pointer(p) {}
    template<class U> requires std::convertible_to<U*, T*>
    Borrow(const Borrow<U>& other) : pointer(other.pointer) {}
};
struct Animal { virtual ~Animal() = default; }; struct Cat : Animal {};
std::function<int(int)> AddLater(int offset) { return [offset](int value) { return value + offset; }; }
constexpr unsigned PowerOfTwo(unsigned exponent) {
    if (exponent >= 31) throw std::out_of_range("exponent");
    return 1u << exponent;
}
static_assert(PowerOfTwo(10) == 1024);
static_assert(std::constructible_from<Borrow<const Animal>, Borrow<Cat>>);
static_assert(!std::constructible_from<Borrow<Cat>, Borrow<Animal>>);
int main() { return Run("templates", [](Checks& c) {
    std::vector<int> values{7,8,9}; c.Expect(SizeOf(values) == 3, "concept");
    c.Expect(FirstCopy(values) == 7, "dependent type");
    std::list<int> empty; c.Throws<std::out_of_range>([&] { (void)FirstCopy(empty); }, "empty");
    Derived<int> derived; c.Expect(derived.ReadWithLog() == 42, "dependent base");
    std::string text = "x"; const std::string fixed = "y";
    c.Expect(ForwardCategory(text) == "mutable lvalue", "forward lvalue");
    c.Expect(ForwardCategory(fixed) == "const lvalue", "forward const");
    c.Expect(ForwardCategory(std::string("z")) == "rvalue", "forward rvalue");
    auto callback = AddLater(5); c.Expect(callback(7) == 12, "capture by value");
    Cat cat; Borrow<Cat> child(&cat); Borrow<const Animal> parent(child);
    c.Expect(parent.pointer == &cat, "converting member template");
    c.Throws<std::out_of_range>([] { (void)PowerOfTwo(31); }, "constexpr runtime error");
}); }
