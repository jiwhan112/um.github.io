// view는 소유권이 아니라 주소와 범위다. 유효한 동안만 빌린다.
#include <array>
#include <charconv>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
std::optional<int> parse_id(std::string_view text) {
    if (text.empty()) return std::nullopt;
    int id = 0;
    auto [end, error] = std::from_chars(text.data(), text.data()+text.size(), id);
    if (error != std::errc{} || end != text.data()+text.size() || id<0) return std::nullopt;
    return id;
}
void damage(std::span<int> health, int amount) {
    if (amount<0) throw std::invalid_argument("negative damage");
    for (auto& hp:health) {
        if (hp<0) throw std::invalid_argument("negative health");
    }
    for (auto& hp:health) hp = hp>amount ? hp-amount : 0;
}
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("view contract");
    };
    std::string message = "123,ready";
    std::string_view field{message.data(), 3};
    check(parse_id(field) == 123);
    check(!parse_id("12x") && !parse_id("") && !parse_id("-1"));
    const std::string embedded("12\0x", 4);
    check(!parse_id(std::string_view(embedded.data(), embedded.size())));
    std::array<int, 3> hp{10, 4, 0};
    damage(hp, 5);
    check((hp == std::array<int, 3>{5, 0, 0}));
    damage(std::span<int>{}, 2);
    check(true);
    // 빈 범위도 유효한 입력이다.
    bool rejected = false;
    try {
        damage(hp, -1);
    } catch(const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected && hp[0] == 5);
    // 장기 보관이 필요하면 빌린 범위가 아니라 소유 문자열을 만든다.
    std::string saved(field);
    // 소유 복사 후에는 원본 문자열이 재할당되어도 saved가 유지된다.
    field = {};
    message.assign(1000, 'x');
    check(saved == "123");
    std::cout << "id=123 hp=5,0,0 owned-copy=123\nPASS: " << checks << " contract checks\n";
}
