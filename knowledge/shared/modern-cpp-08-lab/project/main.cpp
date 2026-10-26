// 쉼표 fold로 왼쪽부터 부작용 순서를 정하고, 각 더하기 전에 오버플로를 막는다.
#include <concepts>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
template<std::same_as<std::uint32_t>... Ts>
std::optional<std::uint32_t> checked_sum(Ts... values) {
    std::uint32_t total = 0;
    bool ok = true;
    // 빈 인자 팩에서는 append가 호출되지 않으므로 이 경우만 미사용이 의도된다.
    [[maybe_unused]] const auto append = [&](std::uint32_t x) {
        if (!ok) return;
        if (x>std::numeric_limits<std::uint32_t>::max()-total) {
            ok = false;
            return;
        }
        total+=x;
    };
    (append(values), ...);
    // 빈 팩도 허용: total=0, ok=true.
    if (!ok) return std::nullopt;
    return total;
}
template<class Tuple, std::size_t... I>
std::uint32_t from_tuple(const Tuple& items, std::index_sequence<I...>) {
    return checked_sum(std::get<I>(items)...).value();
    // 본 예제는 합이 작은 입력만 준다.
}
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("pack contract");
    };
    check(checked_sum() == 0u);
    check(checked_sum(std::uint32_t{2}, std::uint32_t{3}, std::uint32_t{4}) == 9u);
    check(checked_sum(std::uint32_t{UINT32_MAX}, std::uint32_t{0}) == UINT32_MAX);
    check(!checked_sum(std::uint32_t{UINT32_MAX}, std::uint32_t{1}));
    auto costs = std::tuple{std::uint32_t{5}, std::uint32_t{6}};
    check(from_tuple(costs, std::make_index_sequence<2>{}) == 11);
    std::cout << "empty=0 pack=9 tuple=11 overflow=rejected\nPASS: " << checks << " contract checks\n";
}
