#ifndef STUDY_PIMPL_HPP
#define STUDY_PIMPL_HPP
#include <memory>
#include <string>

// 공개 선언은 Impl의 존재만 알리고, 구체적인 데이터 배치는 pimpl.cpp로 숨긴다.
// 헤더 이용자가 Impl 정의 없이 Settings를 선언·사용하는 경계를 보여 준다.
// 이 예제만으로 배포 라이브러리의 ABI 호환성까지 검증한 것은 아니다.
class Settings {
    struct Impl;
    // unique_ptr 멤버의 선언에는 불완전 타입을 사용할 수 있다.
    // Impl의 삭제가 필요한 연산은 완전한 정의를 볼 수 있는 .cpp에 둔다.
    std::unique_ptr<Impl> impl_;
public:
    explicit Settings(std::string name);
    // 이 헤더 안에서 = default로 정의하지 않고, Impl 정의 뒤에서 정의한다.
    ~Settings();
    Settings(Settings&&) noexcept;
    Settings& operator=(Settings&&) noexcept;

    // 소유 핸들만 복사하지 않고 별도 Impl을 만드는 값 복사 계약이다.
    // unique_ptr 자체는 복사할 수 없으므로 구현에서 이 동작을 직접 정의한다.
    Settings(const Settings&);
    Settings& operator=(const Settings&);

    // 새 Impl을 준비한 뒤 교체한다. 비어 있는 이동 후 객체도 다시 채울 수 있다.
    void Rename(std::string name);
    // 내부 문자열의 참조나 뷰가 아니라 소유하는 std::string 값을 반환한다.
    // impl_가 없을 때는 logic_error를 던지는 계약을 구현에 둔다.
    std::string Name() const;
    bool Empty() const noexcept;
    // ADL로 찾는 비멤버 swap은 두 소유 핸들만 교환한다.
    friend void swap(Settings&, Settings&) noexcept;
};
#endif
