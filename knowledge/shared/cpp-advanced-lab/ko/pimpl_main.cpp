#include "check.hpp"
#include "pimpl.hpp"
#include <type_traits>

// 실행 시 검사 여섯 회와 별개인 컴파일 시점 성질 확인이다.
// 이 번역 단위는 Impl의 데이터 정의를 보지 않고 공개 헤더만 사용한다.
static_assert(std::is_nothrow_move_constructible_v<Settings>);

int main() { return Run("pimpl", [](Checks& c) {
    Settings a("old");
    Settings b = a;
    b.Rename("new");
    // 1. Rename 뒤 b의 새 이름과 a의 기존 이름을 관찰한다.
    // 복사 시 별도 Impl을 생성한다는 근거는 복사 생성자의 make_unique 코드다.
    // Rename 자체도 후보 Impl로 교체하므로 이 결과만으로 포인터 분리를 증명하지 않는다.
    c.Expect(a.Name() == "old" && b.Name() == "new", "deep pimpl copy");

    // std::move 자체는 변환이다. 여기서는 선택된 이동 생성자가 소유권을 옮긴다.
    Settings moved = std::move(a);
    // 2. 이동 원본의 Empty와 대상의 이름을 함께 확인한다.
    c.Expect(a.Empty() && moved.Name() == "old", "move contract");
    // 3. 빈 객체에 Name을 호출하는 계약 위반은 logic_error로 보고한다.
    c.Throws<std::logic_error>([&] { (void)a.Name(); }, "empty contract");

    a.Rename("restored");
    // 4. 이동 후 객체도 새 이름으로 복구할 수 있다.
    c.Expect(a.Name() == "restored", "restore moved-from");
    // std::swap 후보를 열어 두고, 인자 타입과 연관된 비멤버 swap을 ADL로 찾는다.
    using std::swap;
    swap(a,b);
    // 5. 두 객체의 이름이 바뀌었는지 확인한다.
    c.Expect(a.Name() == "new" && b.Name() == "restored", "ADL swap");

    b = moved;
    // 6. 복사 대입의 정상 경로를 확인한다. 자기 대입·이동 대입·빈 원본 복사나
    // 후보 복사 실패를 이 여섯 회에서 각각 실행했다고 해석하지 않는다.
    c.Expect(b.Name() == "old", "assignment");
}); }
