#include "check.hpp"
#include <array>
#include <cstddef>
#include <initializer_list>

// 소멸자에서도 예외 없이 순서를 기록할 수 있도록 고정 크기 배열을 사용한다.
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
        // 양수 기록은 생성자 본문에 들어왔다는 뜻이다. 생성 완료와는 구별한다.
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
    // 멤버는 초기화 목록을 읽는 순서가 아니라 이 선언 순서로 생성된다.
    Part first;
    Part second;
    Whole(Trace& t, bool fail = false) : Base(t), first(t, 2), second(t, 3, fail) {
        trace.Add(4);
    }
    // 본문 실행 후 멤버 second, first와 기반 Base가 역순으로 파괴된다.
    ~Whole() noexcept { trace.Add(-4); }
};

class Health {
    int maximum_;
    int current_;
public:
    explicit Health(int maximum) : maximum_(maximum), current_(maximum) {
        // 정상적으로 생성된 객체는 0 <= current <= maximum, maximum > 0을 만족한다.
        if (maximum <= 0) throw std::invalid_argument("maximum must be positive");
    }
    int Current() const noexcept { return current_; }
    void Damage(int amount) {
        // 잘못된 입력을 상태 변경 전에 거부한다.
        if (amount < 0) throw std::invalid_argument("negative damage");
        current_ = amount >= current_ ? 0 : current_ - amount;
    }
    void Heal(int amount) {
        if (amount < 0) throw std::invalid_argument("negative heal");
        // 먼저 남은 공간과 비교해 current + amount의 불필요한 범위 초과를 피한다.
        const int room = maximum_ - current_;
        current_ += amount >= room ? room : amount;
    }
};

int main() {
    return Run("lifetime", [](Checks& c) {
        Trace trace;
        {
            Whole whole(trace);
            c.Expect(trace.Equals({1,2,3,4}), "construction order");
        }
        c.Expect(trace.Equals({1,2,3,4,-4,-3,-2,-1}), "destruction order");

        Trace failed;
        c.Throws<std::runtime_error>([&] { Whole whole(failed, true); }, "constructor failure");
        // second는 본문 진입 뒤 실패하여 자신의 소멸자가 실행되지 않는다.
        // Whole도 완성되지 않았다. 이미 완성한 first와 Base만 정리된다.
        c.Expect(failed.Equals({1,2,3,-2,-1}), "only completed subobjects destroyed");

        Health hp(100);
        hp.Damage(30);
        c.Expect(hp.Current() == 70, "damage");
        hp.Heal(200);
        c.Expect(hp.Current() == 100, "heal clamp");
        hp.Damage(200);
        c.Expect(hp.Current() == 0, "damage clamp");
        c.Throws<std::invalid_argument>([&] { hp.Damage(-1); }, "negative rejected");
        c.Expect(hp.Current() == 0, "failure preserves state");
        c.Throws<std::invalid_argument>([] { Health invalid(0); }, "invalid construction");
    });
}
