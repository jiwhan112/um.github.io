#include "check.hpp"
#include <array>
#include <cstddef>
#include <initializer_list>
struct Trace {
    std::array<int, 32> values{};
    std::size_t used{};
    bool overflow{};
    void Add(int value) noexcept {
        if (used < values.size()) values[used++] = value;
        else overflow = true;
    }
    bool Equals(std::initializer_list<int> expected) const {
        if (overflow || used != expected.size()) return false;
        std::size_t i{};
        for (int value : expected) if (values[i++] != value) return false;
        return true;
    }
};
struct Part {
    Trace& trace;
    int id;
    Part(Trace& t, int value, bool fail = false) : trace(t), id(value) {
        trace.Add(id);
        if (fail) throw std::runtime_error("part construction failed");
    }
    ~Part() noexcept { trace.Add(-id); }
};
struct Base {
    Trace& trace;
    explicit Base(Trace& t) : trace(t) { trace.Add(1); }
    ~Base() noexcept { trace.Add(-1); }
};
struct Whole : Base {
    Part first;
    Part second;
    Whole(Trace& t, bool fail = false) : Base(t), first(t, 2), second(t, 3, fail) { trace.Add(4); }
    ~Whole() noexcept { trace.Add(-4); }
};
class Health {
    int maximum_;
    int current_;
public:
    explicit Health(int maximum) : maximum_(maximum), current_(maximum) {
        if (maximum <= 0) throw std::invalid_argument("maximum must be positive");
    }
    int Current() const noexcept { return current_; }
    void Damage(int amount) {
        if (amount < 0) throw std::invalid_argument("negative damage");
        current_ = amount >= current_ ? 0 : current_ - amount;
    }
    void Heal(int amount) {
        if (amount < 0) throw std::invalid_argument("negative heal");
        const int room = maximum_ - current_;
        current_ += amount >= room ? room : amount;
    }
};
int main() { return Run("lifetime", [](Checks& c) {
    Trace trace;
    { Whole whole(trace); c.Expect(trace.Equals({1,2,3,4}), "construction order"); }
    c.Expect(trace.Equals({1,2,3,4,-4,-3,-2,-1}), "destruction order");
    Trace failed;
    c.Throws<std::runtime_error>([&] { Whole whole(failed, true); }, "constructor failure");
    c.Expect(failed.Equals({1,2,3,-2,-1}), "only completed subobjects destroyed");
    Health hp(100);
    hp.Damage(30); c.Expect(hp.Current() == 70, "damage");
    hp.Heal(200); c.Expect(hp.Current() == 100, "heal clamp");
    hp.Damage(200); c.Expect(hp.Current() == 0, "damage clamp");
    c.Throws<std::invalid_argument>([&] { hp.Damage(-1); }, "negative rejected");
    c.Expect(hp.Current() == 0, "failure preserves state");
    c.Throws<std::invalid_argument>([] { Health invalid(0); }, "invalid construction");
}); }
