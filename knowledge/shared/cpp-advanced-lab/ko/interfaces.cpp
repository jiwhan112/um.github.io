#include "check.hpp"
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
// 원본 동작을 보존하고 API 계약을 한국어로 설명한 주석판이다.
// explicit은 double에서 Seconds로의 암시적 변환을 막고, 직접 생성은 허용한다.
// 조건상 음수·NaN·3600 초과를 거부하지만 아래 실제 입력은 0.016과 -1.0뿐이다.
class Seconds {
    double value_;
public:
    explicit Seconds(double value) : value_(value) {
        if (!(value >= 0.0) || value > 3600.0) throw std::invalid_argument("seconds");
    }
    double Value() const noexcept { return value_; }
};
// 컴파일 시점 검사 한 개다. 런타임 7회와 별도로 집계한다.
static_assert(!std::is_convertible_v<double, Seconds>);
// View는 소유하지 않는 string_view다. 반환된 뷰의 수명은 Label 내부 문자열에 달려 있다.
// 두 rvalue 오버로드 삭제로 임시 Label에서 View를 직접 얻는 호출을 막는다.
// lvalue Label을 나중에 파괴하거나 문자열을 바꾸는 상황까지 자동으로 안전해지는 것은 아니다.
class Label {
    std::string text_;
public:
    explicit Label(std::string text) : text_(std::move(text)) {}
    std::string_view View() const & noexcept { return text_; }
    std::string_view View() && = delete;
    std::string_view View() const && = delete;
    // 소유 문자열을 값으로 반환하므로 임시 Label이 사라진 뒤에도 결과를 사용할 수 있다.
    std::string Copy() const { return text_; }
};
// 아래 연산자는 작은 정수로만 실행한다. 산술 범위 검증은 이 실험의 목적이 아니다.
// Small integers only: arithmetic-range validation is not this experiment's purpose.
class Counter {
    int value_{};
public:
    Counter(int value = 0) : value_(value) {}
    int Value() const noexcept { return value_; }
    Counter& operator+=(const Counter& rhs) { value_ += rhs.value_; return *this; }
    // 왼쪽 피연산자는 값으로 복사하고 +=를 재사용한다.
    // 비멤버 함수이므로 Counter가 포함된 표현식에서 양쪽 피연산자의 암시적 변환을 사용할 수 있다.
    friend Counter operator+(Counter lhs, const Counter& rhs) { lhs += rhs; return lhs; }
};
// 각 Quantity<T>에 대응하는 비멤버 friend를 클래스 안에 정의한다.
// 이 hidden friend는 Quantity가 인자로 등장할 때 ADL로 찾으며, int 인자의 Quantity 변환도 가능하다.
template<class T> class Quantity {
    T value_;
public:
    Quantity(T value) : value_(value) {}
    T Value() const { return value_; }
    friend Quantity operator+(Quantity a, Quantity b) { return Quantity(a.value_ + b.value_); }
};
// 정상 경로의 호출은 Expect 6회 + Throws 1회, 총 7회다.
int main() { return Run("interfaces", [](Checks& c) {
    Seconds dt(0.016); c.Expect(dt.Value() == 0.016, "explicit valid duration");
    c.Throws<std::invalid_argument>([] { Seconds invalid(-1.0); }, "negative duration");
    // fixed는 iterator 객체가 const인 경우다. 위치는 못 바꾸지만 가리킨 int는 수정할 수 있다.
    // readonly는 const_iterator이므로 원소를 수정할 수 없고, iterator 객체 자체는 이동할 수 있다.
    std::vector<int> data{1,2}; const auto fixed = data.begin(); *fixed = 7;
    auto readonly = data.cbegin(); ++readonly;
    c.Expect(data.front() == 7 && *readonly == 2, "const iterator vs const_iterator");
    Label label("player"); c.Expect(label.View() == "player", "borrow lvalue");
    // 임시 Label의 View 대신 소유한 Copy 결과를 받는다. 댕글링 뷰를 실제로 역참조하지 않는다.
    const auto copy = Label("temporary").Copy(); c.Expect(copy == "temporary", "owned result");
    // 양쪽 방향을 작은 값으로 확인한다. signed overflow 같은 위험 입력은 실행하지 않는다.
    Counter count(3); c.Expect((count+2).Value() == 5 && (2+count).Value() == 5, "symmetric conversion");
    Quantity<int> q(4); c.Expect((q+3).Value() == 7 && (3+q).Value() == 7, "hidden friend");
}); }
