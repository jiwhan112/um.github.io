#include "check.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <memory>
#include <utility>
void SetValue(int value) { value = 9; (void)value; }
void SetReference(int& value) { value = 9; }
void ReseatCopy(int* p, int* target) { p = target; (void)p; }
void Reseat(int*& p, int* target) { p = target; }
void ReseatOut(int** p, int* target) { if (p) *p = target; }
int main() {
    int hp{100}; int& alias = hp; int* p = &hp;
    CHECK(p == &hp); CHECK(*p == 100);
    alias = 80; CHECK(hp == 80);
    SetValue(hp); CHECK(hp == 80);
    SetReference(hp); CHECK(hp == 9);
    int other{7}; ReseatCopy(p, &other); CHECK(p == &hp);
    Reseat(p, &other); CHECK(p == &other);
    ReseatOut(&p, &hp); CHECK(p == &hp);
    const int* read_only = &hp; CHECK(*read_only == 9);
    int* const fixed = &hp; *fixed = 6; CHECK(hp == 6);
    void* erased = &hp; CHECK(*static_cast<int*>(erased) == 6);
    int values[]{10, 20, 30}; int* end = values + 3;
    CHECK(*(values + 1) == values[1]); CHECK(end - values == 3);
    // Never dereference end: it is a one-past pointer.
    auto owner = std::make_unique<int>(42);
    int* observer = owner.get(); CHECK(*observer == 42);
    auto new_owner = std::move(owner);
    CHECK(!owner); CHECK(new_owner.get() == observer);
    new_owner.reset(); observer = nullptr; CHECK(observer == nullptr);
    // Explicit byte decoding avoids aliasing and host-endianness assumptions.
    std::array<std::uint8_t, 2> bytes{0x01, 0x02};
    const auto decoded = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[0]) |
        (static_cast<std::uint16_t>(bytes[1]) << 8));
    CHECK(decoded == 513);
    const std::uint32_t source{0x12345678u};
    const auto representation = std::bit_cast<std::array<unsigned char, sizeof(source)>>(source);
    CHECK(std::bit_cast<std::uint32_t>(representation) == source);
    Report();
}
