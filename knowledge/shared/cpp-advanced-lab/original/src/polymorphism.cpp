#include "check.hpp"
#include <array>
#include <memory>
#include <string>
#include <typeinfo>
struct Events {
    std::array<int, 16> data{};
    std::size_t count{};
    void Add(int value) noexcept { if (count < data.size()) data[count++] = value; }
};
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
class Fire final : public Effect {
    Events& events_;
public:
    explicit Fire(Events& events) : Effect(events), events_(events) { events_.Add(Kind()); }
    ~Fire() noexcept override { events_.Add(Kind()); }
    int Kind() const noexcept override { return 2; }
    int Strength(int scale = 3) const noexcept override { return scale * 10; }
};
class Ice final : public Effect { public: using Effect::Effect; };
struct ValueBase { int Kind() const { return 1; } };
struct ValueDerived : ValueBase { int Kind() const { return 2; } };
struct Overloads { int F(int) const { return 1; } };
struct AddOverload : Overloads { using Overloads::F; int F(const std::string&) const { return 2; } };
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
struct Cloneable {
    virtual ~Cloneable() = default;
    virtual std::unique_ptr<Cloneable> Clone() const = 0;
    virtual int Value() const = 0;
};
struct Number final : Cloneable {
    int value;
    explicit Number(int n) : value(n) {}
    std::unique_ptr<Cloneable> Clone() const override { return std::make_unique<Number>(*this); }
    int Value() const override { return value; }
};
int main() { return Run("polymorphism", [](Checks& c) {
    Events events;
    {
        auto concrete = std::make_unique<Fire>(events);
        Effect& base = *concrete;
        c.Expect(events.count == 2 && events.data[0] == 1 && events.data[1] == 2, "construction dispatch");
        c.Expect(base.Kind() == 2, "dynamic dispatch");
        c.Expect(base.Strength() == 20 && concrete->Strength() == 30, "static defaults");
        c.Expect(dynamic_cast<Fire*>(&base) == concrete.get(), "pointer cast success");
        c.Expect(dynamic_cast<Ice*>(&base) == nullptr, "pointer cast failure");
        c.Throws<std::bad_cast>([&] { (void)dynamic_cast<Ice&>(base); }, "reference cast failure");
        std::unique_ptr<Effect> owner = std::move(concrete); owner.reset();
    }
    c.Expect(events.count == 4 && events.data[2] == 2 && events.data[3] == 1, "virtual destruction");
    ValueDerived derived; ValueBase sliced = derived;
    c.Expect(sliced.Kind() == 1 && derived.Kind() == 2, "slicing illustration");
    AddOverload over; c.Expect(over.F(1) == 1 && over.F(std::string("x")) == 2, "using overloads");
    HalfDamage half; c.Expect(half.Apply(10) == 5, "NVI");
    c.Throws<std::invalid_argument>([&] { (void)half.Apply(-1); }, "NVI validation");
    Number number(7); auto clone = number.Clone();
    c.Expect(clone.get() != &number && clone->Value() == 7, "polymorphic clone");
}); }
