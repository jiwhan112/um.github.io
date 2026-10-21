#include "check.hpp"
#include <concepts>
#include <cstddef>
#include <functional>
#include <list>
#include <string>
#include <type_traits>
#include <vector>
// 원본의 타입·표현식·검사 수를 보존하고 설명 주석만 더했다.
// 이 concept는 const 객체의 size() 표현식과 size_t 변환 가능성을 요구한다.
// 반환값이 실제 크기인지, 음수가 아닌지, 연산 비용이 얼마인지는 이 제약만으로 검증하지 않는다.
template<class T> concept Sized = requires(const T& value) {
    { value.size() } -> std::convertible_to<std::size_t>;
};
template<Sized T> std::size_t SizeOf(const T& value) { return static_cast<std::size_t>(value.size()); }
// 다음 세 오버로드로 변경 가능한 lvalue, const lvalue, rvalue가 선택되는지 관찰한다.
std::string Category(std::string&) { return "mutable lvalue"; }
std::string Category(const std::string&) { return "const lvalue"; }
std::string Category(std::string&&) { return "rvalue"; }
// T가 이 호출에서 추론되는 T&&는 forwarding reference다. 이름이 있는 value 자체는 lvalue다.
// std::forward<T>로 원래 범주를 전달한다. 수명을 연장하거나 소유권을 자동으로 이전하지는 않는다.
template<class T> std::string ForwardCategory(T&& value) { return Category(std::forward<T>(value)); }
// Base<T>는 종속 기반 클래스다. this->는 종속 이름 탐색으로 Read를 찾게 한다.
template<class T> struct Base { int Read() const { return 42; } };
template<class T> struct Derived : Base<T> { int ReadWithLog() const { return this->Read(); } };
// Container::value_type이 타입임을 typename으로 명시한다. 빈 범위를 먼저 거부한 뒤 첫 값을 복사한다.
// 이 함수 자체에는 Sized 제약이 없으며 필요한 empty/begin/value_type은 인스턴스화 과정에서 요구된다.
template<class Container> typename Container::value_type FirstCopy(const Container& values) {
    if (values.empty()) throw std::out_of_range("empty");
    return *values.begin();
}
// Borrow는 소유하지 않는 포인터 래퍼다. 가리키는 객체의 수명과 null 여부는 호출자가 관리한다.
// 변환 생성자는 U*에서 T*로 가능한 변환만 허용하여 업캐스트·const 추가와 역방향 변환을 구분한다.
template<class T> struct Borrow {
    T* pointer;
    explicit Borrow(T* p) : pointer(p) {}
    template<class U> requires std::convertible_to<U*, T*>
    Borrow(const Borrow<U>& other) : pointer(other.pointer) {}
};
struct Animal { virtual ~Animal() = default; }; struct Cat : Animal {};
// offset을 값으로 캡처하므로 함수가 끝난 뒤에도 콜백 안에 복사본이 남는다.
// 이번 입력은 5와 7이며, 임의의 int 합산 오버플로를 막는 구현은 아니다.
std::function<int(int)> AddLater(int offset) { return [offset](int value) { return value + offset; }; }
// constexpr 함수도 런타임에 호출할 수 있고 그 경로에서 예외를 던질 수 있다.
// 31은 unsigned의 비트 폭을 조회한 값이 아니라 원본의 고정 제한이다.
// 현재 검증 환경의 unsigned 폭과 선택한 입력을 확인하며, 이 코드를 16비트 unsigned 환경까지 일반화하지 않는다.
constexpr unsigned PowerOfTwo(unsigned exponent) {
    if (exponent >= 31) throw std::out_of_range("exponent");
    return 1u << exponent;
}
// 컴파일 시점 검사 세 개다. 아래 런타임 10회에 더하지 않는다.
static_assert(PowerOfTwo(10) == 1024);
static_assert(std::constructible_from<Borrow<const Animal>, Borrow<Cat>>);
static_assert(!std::constructible_from<Borrow<Cat>, Borrow<Animal>>);
// 정상 실행의 호출은 Expect 8회 + Throws 2회, 총 10회다.
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
    // parent는 cat을 소유하지 않는다. 이 스코프에서는 cat이 관찰 포인터보다 오래 살아 있다.
    Cat cat; Borrow<Cat> child(&cat); Borrow<const Animal> parent(child);
    c.Expect(parent.pointer == &cat, "converting member template");
    // 31은 시프트 전에 거부한다. 런타임에서 큰 시프트나 UB를 일부러 실행하지 않는다.
    c.Throws<std::out_of_range>([] { (void)PowerOfTwo(31); }, "constexpr runtime error");
}); }
