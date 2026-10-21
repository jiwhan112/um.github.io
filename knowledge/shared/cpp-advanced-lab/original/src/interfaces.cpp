#include "check.hpp"
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
class Seconds {
    double value_;
public:
    explicit Seconds(double value) : value_(value) {
        if (!(value >= 0.0) || value > 3600.0) throw std::invalid_argument("seconds");
    }
    double Value() const noexcept { return value_; }
};
static_assert(!std::is_convertible_v<double, Seconds>);
class Label {
    std::string text_;
public:
    explicit Label(std::string text) : text_(std::move(text)) {}
    std::string_view View() const & noexcept { return text_; }
    std::string_view View() && = delete;
    std::string_view View() const && = delete;
    std::string Copy() const { return text_; }
};
// Small integers only: arithmetic-range validation is not this experiment's purpose.
class Counter {
    int value_{};
public:
    Counter(int value = 0) : value_(value) {}
    int Value() const noexcept { return value_; }
    Counter& operator+=(const Counter& rhs) { value_ += rhs.value_; return *this; }
    friend Counter operator+(Counter lhs, const Counter& rhs) { lhs += rhs; return lhs; }
};
template<class T> class Quantity {
    T value_;
public:
    Quantity(T value) : value_(value) {}
    T Value() const { return value_; }
    friend Quantity operator+(Quantity a, Quantity b) { return Quantity(a.value_ + b.value_); }
};
int main() { return Run("interfaces", [](Checks& c) {
    Seconds dt(0.016); c.Expect(dt.Value() == 0.016, "explicit valid duration");
    c.Throws<std::invalid_argument>([] { Seconds invalid(-1.0); }, "negative duration");
    std::vector<int> data{1,2}; const auto fixed = data.begin(); *fixed = 7;
    auto readonly = data.cbegin(); ++readonly;
    c.Expect(data.front() == 7 && *readonly == 2, "const iterator vs const_iterator");
    Label label("player"); c.Expect(label.View() == "player", "borrow lvalue");
    const auto copy = Label("temporary").Copy(); c.Expect(copy == "temporary", "owned result");
    Counter count(3); c.Expect((count+2).Value() == 5 && (2+count).Value() == 5, "symmetric conversion");
    Quantity<int> q(4); c.Expect((q+3).Value() == 7 && (3+q).Value() == 7, "hidden friend");
}); }
