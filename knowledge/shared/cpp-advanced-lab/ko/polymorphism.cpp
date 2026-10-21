#include "check.hpp"
#include <array>
#include <memory>
#include <string>
#include <typeinfo>
// 원본 프로그램의 토큰은 그대로 두고 한국어 설명만 추가한 학습용 주석판이다.
// 기록 버퍼는 16칸이다. 넘친 이벤트는 조용히 버리므로 일반적인 추적 시스템의 완전성을 보장하지 않는다.
struct Events {
    std::array<int, 16> data{};
    std::size_t count{};
    void Add(int value) noexcept { if (count < data.size()) data[count++] = value; }
};
// 생성자·소멸자 안의 가상 호출은 현재 생성·소멸 중인 클래스 단계에서 결정된다.
// Effect 단계의 Kind()는 1이며, 가장 파생된 Fire의 구현까지 호출하지 않는다.
class Effect {
    Events& events_;
public:
    explicit Effect(Events& events) : events_(events) { events_.Add(Kind()); }
    virtual ~Effect() noexcept { events_.Add(Kind()); }
    virtual int Kind() const noexcept { return 1; }
    virtual int Strength(int scale = 2) const noexcept { return scale; }
    Effect(const Effect&) = delete;
    Effect& operator=(const Effect&) = delete;
};
// Fire 생성자 본문과 소멸자 본문에서는 Kind()가 2를 반환한다.
// 생성 기록은 1, 2이고 가상 소멸을 마친 기록은 1, 2, 2, 1이 된다.
// Strength는 기본값 2·3으로만 실행한다. 임의의 큰 scale에 대한 곱셈 범위 검사는 없다.
class Fire final : public Effect {
    Events& events_;
public:
    explicit Fire(Events& events) : Effect(events), events_(events) { events_.Add(Kind()); }
    ~Fire() noexcept override { events_.Add(Kind()); }
    int Kind() const noexcept override { return 2; }
    int Strength(int scale = 3) const noexcept override { return scale * 10; }
};
// Ice는 실패하는 RTTI 변환의 목표 타입이다. 이 테스트에서 실제 Ice 객체를 만들지는 않는다.
class Ice final : public Effect { public: using Effect::Effect; };
// 이 최소 예시는 비가상 Kind()와 값 복사를 사용한다. 파생 데이터 멤버는 없다.
// 기반 부분만 값으로 만드는 표현과 정적 타입에 따른 호출을 보여 주며, 파생 데이터 손실까지 검사하지 않는다.
struct ValueBase { int Kind() const { return 1; } };
struct ValueDerived : ValueBase { int Kind() const { return 2; } };
// 파생 클래스에서 같은 이름을 선언하면 기반 이름이 가려진다. using으로 기반 오버로드도 후보에 넣는다.
struct Overloads { int F(int) const { return 1; } };
struct AddOverload : Overloads { using Overloads::F; int F(const std::string&) const { return 2; } };
// NVI: 공개 비가상 Apply가 입력 계약을 확인하고, private 가상 훅에 계산을 위임한다.
// 허용 범위는 0부터 1000까지다. 아래 실행은 정상값 10과 거부값 -1만 검사한다.
class Damage {
public:
    int Apply(int amount) const {
        if (amount < 0 || amount > 1000) throw std::invalid_argument("amount");
        return ApplyImpl(amount);
    }
    virtual ~Damage() = default;
private:
    virtual int ApplyImpl(int amount) const = 0;
};
class HalfDamage final : public Damage { int ApplyImpl(int amount) const override { return amount / 2; } };
// 가상 Clone은 소유권을 unique_ptr<Cloneable>로 돌려준다. 기반 소멸자도 가상이다.
// 스마트 포인터 자체에 가상 반환형 공변성을 적용한 것이 아니라 두 선언의 반환형을 같게 두었다.
struct Cloneable {
    virtual ~Cloneable() = default;
    virtual std::unique_ptr<Cloneable> Clone() const = 0;
    virtual int Value() const = 0;
};
// 정수 하나를 가진 실제 Number를 새로 만든다. 복잡한 객체 그래프의 깊은 복사까지 다루지는 않는다.
struct Number final : Cloneable {
    int value;
    explicit Number(int n) : value(n) {}
    std::unique_ptr<Cloneable> Clone() const override { return std::make_unique<Number>(*this); }
    int Value() const override { return value; }
};
// 정상 경로의 검사 호출은 Expect 10회 + Throws 2회, 총 12회다.
int main() { return Run("polymorphism", [](Checks& c) {
    Events events;
    {
        auto concrete = std::make_unique<Fire>(events);
        Effect& base = *concrete;
        c.Expect(events.count == 2 && events.data[0] == 1 && events.data[1] == 2, "construction dispatch");
        c.Expect(base.Kind() == 2, "dynamic dispatch");
        // 가상 함수 본체는 Fire로 가지만 기본 인자는 호출식의 정적 타입에서 고른다.
        // Effect&의 기본값 2는 20, Fire*의 기본값 3은 30을 만든다.
        c.Expect(base.Strength() == 20 && concrete->Strength() == 30, "static defaults");
        // 포인터 dynamic_cast 실패는 nullptr, 참조 dynamic_cast 실패는 bad_cast다.
        c.Expect(dynamic_cast<Fire*>(&base) == concrete.get(), "pointer cast success");
        c.Expect(dynamic_cast<Ice*>(&base) == nullptr, "pointer cast failure");
        c.Throws<std::bad_cast>([&] { (void)dynamic_cast<Ice&>(base); }, "reference cast failure");
        // unique_ptr의 이동은 소유 핸들을 넘긴다. Fire 객체를 값으로 복사·이동하지 않는다.
        // 기반 포인터로 삭제해도 가상 소멸자 덕분에 Fire 다음 Effect 순서로 정리된다.
        std::unique_ptr<Effect> owner = std::move(concrete); owner.reset();
    }
    // 위 스코프가 끝나기 전에 owner.reset()이 이미 객체를 파괴했다.
    c.Expect(events.count == 4 && events.data[2] == 2 && events.data[3] == 1, "virtual destruction");
    ValueDerived derived; ValueBase sliced = derived;
    c.Expect(sliced.Kind() == 1 && derived.Kind() == 2, "slicing illustration");
    AddOverload over; c.Expect(over.F(1) == 1 && over.F(std::string("x")) == 2, "using overloads");
    HalfDamage half; c.Expect(half.Apply(10) == 5, "NVI");
    c.Throws<std::invalid_argument>([&] { (void)half.Apply(-1); }, "NVI validation");
    // 복제본은 다른 객체이며 값 7을 보존한다. 이후 두 객체를 변경하는 독립성 테스트는 별도로 없다.
    Number number(7); auto clone = number.Clone();
    c.Expect(clone.get() != &number && clone->Value() == 7, "polymorphic clone");
}); }
