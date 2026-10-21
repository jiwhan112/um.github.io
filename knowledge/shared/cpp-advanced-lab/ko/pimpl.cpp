#include "pimpl.hpp"
#include <stdexcept>
#include <utility>

// 실제 데이터 정의는 이 번역 단위에서만 보인다.
struct Settings::Impl { std::string name; };

Settings::Settings(std::string name) : impl_(std::make_unique<Impl>(Impl{std::move(name)})) {}

// 이 지점에서는 Impl이 완전하므로 unique_ptr의 삭제 동작을 구성할 수 있다.
Settings::~Settings() = default;
// 기본 이동은 unique_ptr의 소유권 이동이다. 정상적인 다른 객체로의 이동 뒤에는
// 원본 핸들이 비게 된다. 테스트는 이동 생성 경로를 실행한다.
Settings::Settings(Settings&&) noexcept = default;
Settings& Settings::operator=(Settings&&) noexcept = default;

// 원본이 차 있으면 별도 Impl과 문자열 값을 복사하고, 비었으면 비어 있게 복사한다.
// 현재 정상 테스트가 확인하는 것은 차 있는 원본을 복사한 뒤 각 이름을 바꾸는 경로다.
Settings::Settings(const Settings& other) : impl_(other.impl_ ? std::make_unique<Impl>(*other.impl_) : nullptr) {}

Settings& Settings::operator=(const Settings& other) {
    if (this != &other) {
        // 후보 복사가 실패하면 아직 *this를 변경하지 않았다.
        // 복사가 끝난 뒤 noexcept swap으로 교체하고, copy가 이전 상태를 정리한다.
        // 이 구조의 강한 예외 보장을 설명하되, 해당 테스트에서 할당 실패를 주입하지는 않는다.
        Settings copy(other);
        swap(*this, copy);
    }
    return *this;
}

void Settings::Rename(std::string name) {
    // 인자 준비와 후보 생성이 완료될 때까지 기존 impl_는 그대로다.
    auto candidate = std::make_unique<Impl>(Impl{std::move(name)});
    impl_.swap(candidate);
    // candidate가 이전 Impl을 소유한 채 범위를 벗어나 정리한다.
    // 파일·네트워크 부수 효과나 동시 접근을 다루는 트랜잭션은 아니다.
}

std::string Settings::Name() const {
    // 이동 후 비어 있는 상태를 유효한 상태로 인정하되, Name 접근은 명시적으로 거부한다.
    if (!impl_) throw std::logic_error("empty moved-from settings");
    // 값 복사 반환이므로 호출자는 Settings와 별도의 문자열 수명을 갖는다.
    return impl_->name;
}

bool Settings::Empty() const noexcept { return !impl_; }
// Impl의 문자열을 하나씩 교환하지 않고, 두 소유 포인터를 교환한다.
void swap(Settings& a, Settings& b) noexcept { a.impl_.swap(b.impl_); }
