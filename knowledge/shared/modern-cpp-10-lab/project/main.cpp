// 값 없음, 여러 상태 중 하나, 설명 가능한 실패를 서로 다른 타입으로 표현한다.
#include <charconv>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <variant>
#include <type_traits>
#include <version>
#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
#include <expected>
#define LESSON_HAS_EXPECTED 1
#else
#define LESSON_HAS_EXPECTED 0
#endif
struct Waiting {
};
struct Ready {
    int texture;
};
struct Failed {
    std::string reason;
};
using State = std::variant<Waiting, Ready, Failed>;
std::optional<int> lookup(int key) {
    if (key == 7) return 100;
    return std::nullopt;
}
#if LESSON_HAS_EXPECTED
std::expected<int, std::string> parse_count(std::string_view text) {
    if (text.empty()) return std::unexpected("empty");
    int count = 0;
    auto [end, ec] = std::from_chars(text.data(), text.data()+text.size(), count);
    if (ec != std::errc{} || end != text.data()+text.size()) return std::unexpected("invalid integer");
    if (count<0) return std::unexpected("negative");
    return count;
}
#endif
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("result contract");
    };
    check(lookup(7) == 100 && !lookup(8));
    // 현재 활성 대안을 확인한 뒤에만 해당 멤버를 읽는다.
    State state = Waiting{};
    check(std::holds_alternative<Waiting>(state));
    state = Ready{23};
    auto ready = std::get_if<Ready>(&state);
    check(ready && ready->texture == 23);
    state = Failed{"missing asset"};
    check(std::get_if<Ready>(&state) == nullptr);
    const auto message = std::visit([](const auto& item)->std::string {
    using T = std::decay_t<decltype(item)>;
    if constexpr(std::is_same_v<T, Waiting>) return "waiting";
    else if constexpr(std::is_same_v<T, Ready>) return "ready";
    else return item.reason;
    }, state);
    check(message == "missing asset");
#if LESSON_HAS_EXPECTED
    check(parse_count("15") == 15);
    auto negative = parse_count("-1");
    check(!negative && negative.error() == "negative");
    check(!parse_count("99999999999999999999999") && !parse_count(""));
    std::cout << "expected=executed\n";
#else
    std::cout << "expected=not-compiled (requires C++23 library)\n";
#endif
    std::cout << "variant=failed optional=missing\nPASS: " << checks << " contract checks\n";
}
