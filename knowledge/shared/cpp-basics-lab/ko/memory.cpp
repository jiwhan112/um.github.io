// C++ 기초 03 · 원본 memory.cpp의 실행 토큰을 유지한 한국어 주석판입니다.
// 공통 검사 함수는 original/src/check.hpp에 있습니다. 이 파일에는 19개의 런타임 검사가 있습니다.
#include "check.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <memory>
#include <utility>

// 값으로 받은 int는 복사본입니다. 함수 안의 대입은 호출자의 int를 바꾸지 않습니다.
void SetValue(int value) {
    value = 9;
    (void)value;
}

// 참조는 호출자가 넘긴 객체를 가리키므로 같은 대입이 호출자의 값을 바꿉니다.
void SetReference(int& value) {
    value = 9;
}

// 포인터도 값으로 받으면 주소의 복사본입니다. 복사본에 다른 주소를 넣어도 호출자의 포인터는 그대로입니다.
void ReseatCopy(int* p, int* target) {
    p = target;
    (void)p;
}

// 포인터 변수 자체를 참조로 받아 그 변수에 저장된 주소를 바꿉니다.
void Reseat(int*& p, int* target) {
    p = target;
}

// 포인터 변수의 주소를 받는 방법입니다. p가 null이 아닐 때 호출자의 포인터에 target을 씁니다.
void ReseatOut(int** p, int* target) {
    if (p)
        *p = target;
}

int main() {
    // alias는 hp의 다른 이름이고, p는 hp의 주소를 보관하는 별도의 객체입니다.
    int hp{100};
    int& alias = hp;
    int* p = &hp;
    CHECK(p == &hp);
    CHECK(*p == 100);
    alias = 80;
    CHECK(hp == 80);

    SetValue(hp);
    CHECK(hp == 80);
    SetReference(hp);
    CHECK(hp == 9);

    // 세 함수가 '가리키는 값'과 '포인터에 저장한 주소' 중 무엇을 바꾸는지 비교합니다.
    int other{7};
    ReseatCopy(p, &other);
    CHECK(p == &hp);
    Reseat(p, &other);
    CHECK(p == &other);
    ReseatOut(&p, &hp);
    CHECK(p == &hp);

    // const int*는 이 경로로 값을 바꾸지 못합니다. int* const는 포인터 주소가 고정되고 값은 바꿀 수 있습니다.
    const int* read_only = &hp;
    CHECK(*read_only == 9);
    int* const fixed = &hp;
    *fixed = 6;
    CHECK(hp == 6);

    // void* 자체로는 int를 읽을 수 없습니다. 원래 int 객체였다는 전제에서 int*로 되돌립니다.
    void* erased = &hp;
    CHECK(*static_cast<int*>(erased) == 6);

    // 배열 안의 포인터 산술과 한 칸 뒤 포인터를 확인합니다. end는 비교·차이 계산에 쓰며 역참조하지 않습니다.
    int values[]{10, 20, 30};
    int* end = values + 3;
    CHECK(*(values + 1) == values[1]);
    CHECK(end - values == 3);

    // unique_ptr는 메모리를 해제할 책임을 가집니다. get()으로 얻은 observer는 그 책임을 갖지 않습니다.
    auto owner = std::make_unique<int>(42);
    int* observer = owner.get();
    CHECK(*observer == 42);

    // 소유권 이동은 같은 할당을 새 소유자에게 넘깁니다. 이동 전 소유자는 비워집니다.
    auto new_owner = std::move(owner);
    CHECK(!owner);
    CHECK(new_owner.get() == observer);

    // reset 뒤 observer를 역참조하면 안 됩니다. 이 예제는 즉시 nullptr로 바꿔 더 이상 쓰지 않습니다.
    new_owner.reset();
    observer = nullptr;
    CHECK(observer == nullptr);

    // 두 바이트를 낮은 자리·높은 자리로 명시적으로 해석합니다. 포인터 재해석이나 호스트 바이트 순서에 기대지 않습니다.
    std::array<std::uint8_t, 2> bytes{0x01, 0x02};
    const auto decoded = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[0]) |
        (static_cast<std::uint16_t>(bytes[1]) << 8));
    CHECK(decoded == 513);

    // C++20 bit_cast로 객체 표현을 복사한 뒤 원래 타입으로 되돌립니다. 특정 바이트 순서를 가정하지 않습니다.
    const std::uint32_t source{0x12345678u};
    const auto representation = std::bit_cast<std::array<unsigned char, sizeof(source)>>(source);
    CHECK(std::bit_cast<std::uint32_t>(representation) == source);
    Report();
}
