<callout icon="🔧" color="blue_bg">
<b>구현을 바꿀 때에는 성능 추측보다 수명·실패·의존성부터 확인합니다.</b>

C++20 기준 · 개정일 <date start="2026-10-13" />.
</callout>

<mention url="https://app.notion.com/p/8178c17f593b4f4e83a0b42544b8fe2b">Effective C++ — 55항목 개정판 v1.0</mention>

<table-of-contents color="gray" />

## 항목 26. 필요한 지점에서 유효한 값으로 정의하기

변수를 사용하는 위치 가까이에서 초기화하면 이름의 유효 범위가 작아지고 초기화되지 않은 상태를 줄일 수 있습니다. 실패할 수 있는 검사가 끝나기 전에 비싼 객체를 만들어 놓고 곧 버리는 낭비도 줄일 수 있습니다. 그러나 ‘모든 객체를 루프 안에서 새로 만들어라’라는 규칙은 아닙니다.

예를 들어 반복마다 긴 문자열 버퍼를 재사용하면 유리한 경우와, 이전 상태가 다음 반복에 남아 버그를 만드는 경우가 모두 있습니다. 생성·대입·할당 비용을 실제로 비교하되 객체의 의미가 바뀌는 지점을 명확히 합니다. 변수를 위에 한꺼번에 선언해두는 오래된 코딩 습관을 따르기보다 읽는 사람이 그 값의 시작 상태를 알게 합니다.

```c++
if (!input_is_valid) return Error::InvalidInput;
Result result = BuildResult();
return result;
```

위는 선언 위치를 보여주는 의사코드입니다. Error와 Result가 포함된 완성 API를 뜻하지 않습니다. 실제 실패 인터페이스는 프로젝트가 예외·결과 객체 중 무엇을 사용하는지에 맞춰 구성합니다.

## 항목 27. 캐스팅의 필요와 전제를 드러내기

캐스트가 자주 등장하면 타입 설계나 인터페이스가 실제 사용과 맞는지 먼저 확인합니다. 숫자 변환에는 범위·정밀도, 계층 변환에는 동적 타입·접근성, 포인터 변환에는 수명·정렬·접근 가능한 타입 규칙이 중요합니다. 캐스트를 C++ 스타일로 바꾼다고 조건이 자동으로 검증되는 것은 아닙니다.

static\_cast 다운캐스트에는 실제 타입에 대한 전제가 필요합니다. dynamic\_cast의 검사도 이미 수명이 끝난 포인터를 정상 포인터로 만들어 주지는 않습니다. const\_cast로 원래 const인 객체를 수정하는 것은 허용되지 않습니다. reinterpret\_cast는 ‘어떤 비트든 원하는 타입으로 안전하게 읽기’ 기능이 아닙니다.

일반적인 바이트 표현을 관찰하려는 목적에는 허용된 바이트 접근이나 memcpy·bit\_cast의 요구사항을 검토합니다. 외부 파일·네트워크에서는 엔디안·길이·인코딩을 정한 파싱이 필요합니다. 단순한 cast만으로 직렬화가 완성되지 않습니다.

근거: [N4861 — static\_cast](https://timsong-cpp.github.io/cppwp/n4861/expr.static.cast), [dynamic\_cast](https://timsong-cpp.github.io/cppwp/n4861/expr.dynamic.cast), [reinterpret\_cast](https://timsong-cpp.github.io/cppwp/n4861/expr.reinterpret.cast).

## 항목 28. 내부 데이터를 빌려주는 API의 수명

멤버의 포인터·참조·iterator·span·string\_view를 반환하면 호출자는 원본의 수명과 변경 규칙에 의존합니다. 불필요한 복사를 줄일 수 있지만 객체 소멸, vector 재할당, string 변경 뒤에도 보관한 뷰를 쓰는 문제가 생길 수 있습니다.

이번 실습 Label은 살아 있는 lvalue 객체에서 View를 빌려주고, 임시 객체에서 직접 View를 얻는 호출은 삭제합니다. 필요하면 Copy로 독립된 문자열을 반환합니다.

```c++
std::string_view View() const & noexcept { return text_; }
std::string_view View() && = delete;
std::string_view View() const && = delete;
std::string Copy() const { return text_; }
```

이는 전체 Label 클래스 중 발췌입니다. 이렇게 해도 함수 밖에서 Label이 먼저 소멸하거나 text\_가 무효화되면 뷰는 사용할 수 없습니다. ref-qualified 함수는 일부 잘못된 사용을 막는 도구이지 수명 검증기 전체가 아닙니다. <code>negative/temporary_view.cpp</code>에서는 명시적으로 금지한 임시 객체 호출이 거부되는지 확인합니다.

## 항목 29. 예외 안전성을 연산의 후조건으로 쓰기

실패했을 때 자원 누수 없이 객체 규칙이 유지되는 기본 보장, 변경이 없었던 것처럼 돌아가는 강한 보장, 예외가 전파되지 않는 보장을 구분합니다. 강한 보장은 성능·메모리 비용이 있을 수 있으므로 필요한 연산에 대해 설계합니다. 실패 후 일부 처리만 된 상태를 허용한다면 그 상태가 무엇인지도 계약입니다.

```mermaid
flowchart LR
    A["기존 GameState"] --> B["독립 candidate 복사"]
    B --> C["candidate 수정·검증"]
    C -->|"성공"| D["noexcept swap"]
    C -->|"실패"| E["candidate 폐기"]
    E --> F["기존 GameState 유지"]
```

transaction.cpp는 자체 소유 상태에 이 패턴을 적용합니다. candidate를 만들거나 수정하는 중 실패하면 원본은 바뀌지 않고, 성공하면 포인터 교체로 반영합니다. 내부 helper가 원본이 아닌 사본을 수정하므로 helper의 보장과 최종 공개 연산의 보장을 같은 것으로 단순화하지 않습니다.

반대로 함수 안에서 이메일을 보내거나 파일을 덮어쓴 뒤 메모리만 되돌리면 외부 부수 효과는 남습니다. 락 없이 두 스레드가 접근하는 원자성도 이 그림이 해결하지 않습니다. 예외 안전성·데이터베이스 트랜잭션·동시성의 원자성을 같은 단어처럼 사용하지 않습니다.

## 항목 30. inline과 인라이닝을 분리하기

inline 지정은 여러 번역 단위의 정의와 ODR에 관련된 언어 의미가 있습니다. 코드 호출을 항상 없애는 최적화 명령은 아닙니다. 컴파일러는 inline 없이도 호출을 펼치고, inline이 붙어도 실제 호출을 남길 수 있습니다.

구현을 헤더로 옮기면 호출 지점에 정의가 보여 최적화 기회가 늘 수 있지만 재빌드 의존성·코드 크기·배포 경계가 달라집니다. 링크 시점 최적화나 가상 호출의 직접화 등도 있어 ‘함수 포인터면 인라이닝 불가’처럼 단정하지 않습니다. 실제 핫패스에서 프로파일과 생성 코드를 확인합니다.

일반적인 헤더 기반 클래스 안 정의가 암묵적으로 inline인 경우와 C++20 named module 등 예외 문맥도 구별합니다. 함수의 한 가지 선언 형태를 보고 성능 결과를 확정하지 않습니다.

근거: [N4861 — inline](https://timsong-cpp.github.io/cppwp/n4861/dcl.inline).

## 항목 31. 헤더 의존성을 줄이는 실전 선택

공개 헤더가 거대한 구현 헤더를 포함하면 작은 변경에도 많은 번역 단위가 다시 컴파일될 수 있습니다. 전방 선언이 충분한 포인터·참조 선언과 완전한 정의가 필요한 값 멤버·멤버 접근 등을 구분합니다. 타입을 무조건 불완전하게 숨길 필요는 없으며 작은 값 타입의 단순성을 유지할 수 있습니다.

Pimpl은 구현 객체를 포인터 뒤로 숨겨 공개 선언과 구현 세부사항을 분리하는 방법입니다. 이번 Settings는 Impl을 cpp에서 정의하고 소멸·복사·이동 함수를 적절한 정의 지점에 둡니다. unique\_ptr를 썼다고 완전한 타입이 필요한 지점이 모두 없어지지는 않습니다.

```c++
// 헤더
class Settings {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    ~Settings();
};
// cpp: Impl 정의 이후
Settings::~Settings() = default;
```

단순화한 발췌이며 전체 생성·복사·이동·메서드는 실습 팩에 있습니다. Pimpl은 포인터 간접 접근·할당·인라이닝 제한 등 비용이 있고, 이것만으로 어떤 바이너리 변경에도 ABI가 안정되는 것은 아닙니다. DLL 경계와 allocator, 공개 함수 시그니처, 컴파일러 ABI는 별도로 고려해야 합니다.

## 예외 안전성과 구현 의존성 비교

<table fit-page-width="true" header-row="true">
<tr>
<td>
계약
</td>
<td>
예외 발생 후 보장
</td>
<td>
실습 적용
</td>
</tr>
<tr>
<td>
no-throw
</td>
<td>
연산이 예외를 외부로 내보내지 않음
</td>
<td>
swap·소멸 경로의 noexcept 조건 확인
</td>
</tr>
<tr>
<td>
strong
</td>
<td>
실패해도 외부 관찰 가능 상태 보존
</td>
<td>
임시 사본 준비 후 교체
</td>
</tr>
<tr>
<td>
basic
</td>
<td>
불변식·자원 안전성 유지, 상태 변경 가능
</td>
<td>
부분 상태 갱신을 계약으로 표현
</td>
</tr>
<tr>
<td>
no guarantee
</td>
<td>
실패 후 안전한 후조건을 약속하지 못함
</td>
<td>
원시 소유권과 예외 경로 재설계
</td>
</tr>
</table>

강한 예외 보장은 모든 하위 함수의 동일한 보장을 뜻하지 않습니다. 외부 부수효과의 되돌림 가능성을 별도로 판단합니다.

## 실습

interfaces의 임시 View 거부, transaction의 실패 후 원본 유지, pimpl의 실제 분할 빌드를 확인합니다. 오류를 고치는 과정에서 단순히 cast나 include를 늘리는 대신, 누가 상태를 소유하고 어느 경계에서 정의가 필요한지 설명해 봅니다.

<toggle>
함수 내부 helper가 강한 보장을 제공하지 않으면 상위 함수도 절대 강한 보장을 못 줄까?

독립적인 임시 상태에서 작업하고 성공 시에만 교체하면 상위 연산은 원본에 대해 강한 보장을 제공할 수 있습니다. 반대로 취소할 수 없는 외부 부수 효과가 있다면 별도 정책이 필요합니다.
</toggle>

<toggle>
소멸자를 헤더에서 default로 정의해도 Pimpl은 항상 괜찮을까?

unique\_ptr가 불완전한 Impl을 소멸시키는 지점이 어디인지 봐야 합니다. 이번 구현은 필요한 정의를 cpp로 옮겨 실제 분할 빌드로 확인했습니다. 단순히 포인터 크기만 안다는 사실로 삭제 요구사항까지 충족하는 것은 아닙니다.
</toggle>

검증 기록과 전체 파일 지도: <mention url="https://app.notion.com/p/3f8a99703f408186b6f6e6196b1be1a6" />. 설계 참고: [Core Guidelines — 인터페이스](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-interfaces), [unique\_ptr 요구사항](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr).
