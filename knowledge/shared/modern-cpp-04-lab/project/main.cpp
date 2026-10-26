// 이동 허용 표현식과 실제 이동 생성자 호출을 구별한다.
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
struct Resource {
    std::unique_ptr<int> value;
    explicit Resource(int n):value(std::make_unique<int>(n)) {
    }
    Resource(Resource&&) noexcept = default;
    Resource& operator=(Resource&&) noexcept = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
};
struct CopyOrMove {
    int value;
    inline static unsigned copies = 0, moves = 0;
    explicit CopyOrMove(int n):value(n) {
    }
    CopyOrMove(const CopyOrMove& x):value(x.value) {
        ++copies;
    }
    CopyOrMove(CopyOrMove&& x) noexcept(false):value(x.value) {
        ++moves;
        x.value = -1;
    }
};
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("move contract");
    };
    static_assert(std::is_nothrow_move_constructible_v<Resource>);
    Resource source{42};
    Resource destination = std::move(source);
    check(!source.value && *destination.value == 42);
    // unique_ptr의 이동 후 빈 상태는 보장된다.
    // 대입 대상이 이미 가진 자원7도 안전하게 정리되어야 한다.
    Resource replacement{7};
    replacement = std::move(destination);
    check(!destination.value && *replacement.value == 42);
    // 던질 수 있는 이동 대신 복사 경로가 선택되는지 횟수로 관찰한다.
    CopyOrMove risky{9};
    CopyOrMove safe = std::move_if_noexcept(risky);
    check(safe.value == 9 && risky.value == 9);
    check(CopyOrMove::copies == 1 && CopyOrMove::moves == 0);
    const CopyOrMove immutable{11};
    CopyOrMove from_const = std::move(immutable);
    // const T&&는 비const 이동 생성자에 맞지 않아 복사된다.
    check(from_const.value == 11 && CopyOrMove::copies == 2);
    std::cout << "unique-owner-transferred copies=2 moves=0\nPASS: " << checks << " contract checks\n";
}
