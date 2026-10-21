#include "check.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// span은 원소를 소유하지 않는 연속 구간이다. 호출 중 원본의 수명과 주소가 유효해야 한다.
// 이 합계 예제는 int로 표현 가능한 작은 값만 사용한다.
int Sum(std::span<const int> values) {
    int result{};
    for (int value : values) result += value;
    return result;
}

// name은 문자열을 직접 소유한다. sizeof(Item)을 파일 포맷의 크기로 사용하면 안 된다.
struct Item {
    int id;
    std::string name;
    int quantity;
};

int main() {
    // array는 고정 개수의 원소를 소유하고 span으로 그 구간을 빌려 줄 수 있다.
    std::array<int, 3> a{10, 20, 30};
    CHECK(Sum(a) == 60);

    std::vector<int> v;
    v.reserve(4);
    // reserve는 저장 공간만 확보한다. 아직 접근 가능한 원소는 하나도 없다.
    CHECK(v.empty());
    CHECK(v.capacity() >= 4);

    // resize가 세 원소를 만들며, 이 int 원소들의 초기값은 0이다.
    v.resize(3);
    CHECK(v.size() == 3);
    CHECK(v[0] == 0);
    v[1] = 8;
    CHECK(Sum(v) == 8);

    // 유효한 인덱스는 0, 1, 2이다. at은 범위 밖 접근을 예외로 알린다.
    bool rejected = false;
    try { (void)v.at(3); }
    catch (const std::out_of_range&) { rejected = true; }
    CHECK(rejected);

    const auto old_capacity = v.capacity();
    // 현재 capacity보다 크게 요청해 재할당을 일으킨다.
    // 이전 원소 포인터·참조·반복자는 보관하거나 검사하지 않는다.
    v.reserve(old_capacity + 1);
    CHECK(v.capacity() > old_capacity);
    CHECK(v[1] == 8);

    // grid는 길이 3인 행 배열 두 개다. row는 int**가 아니라 행 배열 포인터다.
    int grid[2][3]{{1,2,3},{4,5,6}};
    int (*row)[3] = grid;
    CHECK(row[1][2] == 6);
    // 평탄한 vector에서는 행 * 열 개수 + 열로 인덱스를 계산한다.
    std::vector<int> flat{1,2,3,4,5,6};
    CHECK(flat.at(1 * 3 + 2) == 6);

    // string은 길이를 보관하므로 내부 널도 원소다. C 문자열 함수는 첫 널에서 멈춘다.
    std::string embedded{"a\0b", 3};
    CHECK(embedded.size() == 3);
    CHECK(std::strlen(embedded.c_str()) == 1);
    std::string name{"Potion"};
    const std::string_view view{name};
    // view는 name을 소유하지 않는다. 이 예제에서는 name의 수명과 내용을 유지한다.
    CHECK(view == "Potion");
    // strcmp는 음수·0·양수의 의미만 사용한다. 정확히 -1이나 1일 필요는 없다.
    CHECK(std::strcmp("a", "b") < 0);
    CHECK(std::strcmp("b", "b") == 0);

    std::vector<Item> items{{2,"Mana",1},{1,"Health",3},{3,"Shield",1}};
    std::sort(items.begin(), items.end(), [](const Item& x, const Item& y) {
        // 수량 내림차순, 같은 수량에서는 ID 오름차순으로 비교한다.
        return x.quantity != y.quantity ? x.quantity > y.quantity : x.id < y.id;
    });
    CHECK(items[0].id == 1);
    CHECK(items[1].id == 2);
    CHECK(items[2].id == 3);

    // variant는 현재 활성 타입을 추적한다. 활성 타입에 맞춰 읽는다.
    std::variant<int, std::string> data{7};
    CHECK(std::get<int>(data) == 7);
    data = std::string{"text"};
    CHECK(std::holds_alternative<std::string>(data));

    // 같은 엔진과 시드는 같은 엔진 출력을 만든다. 분포 변환과는 구별한다.
    std::mt19937 engine_a{1234u}, engine_b{1234u};
    for (int i = 0; i < 8; ++i) CHECK(engine_a() == engine_b());
    std::uniform_int_distribution<int> die{1, 6};
    // 이 실험은 20번의 결과 범위만 확인한다. 통계적 균등성 검증은 아니다.
    for (int i = 0; i < 20; ++i) {
        int roll = die(engine_a);
        CHECK(roll >= 1 && roll <= 6);
    }
    // 크기와 정렬 요구는 현재 구현의 관찰값이며 모든 플랫폼의 고정값이 아니다.
    std::cout << "sizeof(Item)=" << sizeof(Item) << " alignof(Item)=" << alignof(Item) << '\n';
    Report();
}
