// 이름의 선언형과 표현식의 값 범주를 별도로 검사한다.
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
struct Actor {
    int hp;
};
auto copy_hp(Actor& a) {
    return a.hp;
}
decltype(auto) borrow_hp(Actor& a) {
    return (a.hp);
}
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("deduction contract");
    };
    const int level = 9;
    auto copy = level;
    auto& alias = level;
    static_assert(std::is_same_v<decltype(copy), int>);
    static_assert(std::is_same_v<decltype(alias), const int&>);
    static_assert(std::is_same_v<decltype(level), const int>);
    static_assert(std::is_same_v<decltype((level)), const int&>);
    // 값 반환의 복사와 참조 반환의 별칭이 같은 객체를 바꾸는지 비교한다.
    Actor actor{40};
    auto hp = copy_hp(actor);
    hp = 5;
    check(actor.hp == 40 && hp == 5);
    borrow_hp(actor) = 17;
    check(actor.hp == 17);
    auto && named = Actor{22};
    // 이 지역 참조에 바인딩된 임시의 수명은 범위 끝까지.
    static_assert(std::is_same_v<decltype(named), Actor && >);
    static_assert(std::is_same_v<decltype((named)), Actor&>);
    // 이름 붙은 식은 lvalue.
    check(named.hp == 22);
    // 프록시를 쓰는 동안 vector의 크기와 수명을 바꾸지 않는다.
    std::vector<bool> flags{true};
    auto proxy = flags[0];
    // bool이 아니라 비트 프록시 객체일 수 있다.
    bool snapshot = flags[0];
    flags[0] = false;
    check(!static_cast<bool>(proxy) && snapshot);
    std::cout << "copy=5 actor.hp=17 snapshot=true proxy=false\nPASS: " << checks << " contract checks\n";
}
