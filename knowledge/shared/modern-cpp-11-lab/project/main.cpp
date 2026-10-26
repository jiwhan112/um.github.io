// 문법 제약과 런타임 의미 계약은 별개다. view의 지연 평가는 소유 복사가 아니다.
#include <algorithm>
#include <concepts>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <vector>
template<class T> concept HasHealth = requires(const T& value) {
    { value.hp } -> std::convertible_to<int>;
};
template<HasHealth T> bool alive(const T& value) {
    return value.hp>0;
}
struct Actor {
    int hp;
};
struct Tag {
    const char* name;
};
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("range contract");
    };
    static_assert(HasHealth<Actor> && !HasHealth<Tag>);
    std::vector<Actor> actors{{4}, {0}, {7}};
    auto health = actors | std::views::filter([](const Actor& a) {
        return alive(a);
    })
    | std::views::transform([](const Actor& a) {
        return a.hp;
    });
    // 결과가 필요한 순간에 별도 소유 컨테이너로 구체화한다.
    std::vector<int> snapshot;
    for (int hp:health) snapshot.push_back(hp);
    check((snapshot == std::vector<int>{4, 7}));
    actors[0].hp = 5;
    // 필터 포함 여부는 그대로 두고 값만 변경. 반복자 무효화 없음.
    check(*health.begin() == 5 && snapshot[0] == 4);
    check(std::ranges::all_of(health, [](int hp) {
        return hp>0;
    }));
    auto found = std::ranges::find(snapshot, 7);
    check(found != snapshot.end() && *found == 7);
    using TemporaryResult = decltype(std::ranges::find(std::vector<int>{1, 2}, 2));
    static_assert(std::is_same_v<TemporaryResult, std::ranges::dangling>);
    check(std::ranges::distance(health) == 2);
    std::cout << "snapshot=4,7 live-first=5 matching=2\nPASS: " << checks << " contract checks\n";
}
