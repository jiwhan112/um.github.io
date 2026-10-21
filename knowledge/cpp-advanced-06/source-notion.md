<callout icon="🧪" color="blue_bg">
<b>언어 규칙, 설계 선택, 실행으로 확인한 사실을 분리합니다.</b>

개정일 <date start="2026-10-13" /> · C++ 심화와 Effective C++가 공유하는 실습 안내입니다.
</callout>

<mention url="https://app.notion.com/p/aaa55bced8ff459ba67fb036fb34bb3b">C++ 심화문법 — 개정판 v1.0</mention> · <mention url="https://app.notion.com/p/8178c17f593b4f4e83a0b42544b8fe2b" />

<table-of-contents color="gray" />

## 1. 헤더 의존성과 실행 중 객체 연결은 다르다

헤더는 이름·타입·인라인 또는 템플릿 정의를 사용 지점에 제공합니다. 함수 선언만 알고 호출 코드를 만들 수 있는 경우와, 값 멤버의 크기·멤버 접근·소멸을 위해 완전한 정의가 필요한 경우를 나눕니다. 헤더 가드는 같은 번역 단위의 중복 포함을 막지만 여러 cpp에 외부 변수 정의를 반복해도 되는 장치는 아닙니다.

전방 선언을 많이 쓰면 무조건 좋다는 것도 아닙니다. 표준 라이브러리 타입을 임의로 forward-declare하지 말고 제공되는 헤더를 사용합니다. 구현을 숨기는 비용보다 단순한 값 타입을 공개하는 편익이 큰 경우도 있습니다.

## 2. Pimpl은 구현 포인터를 통한 의존성 분리

```c++
// pimpl.hpp — 발췌
class Settings {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    explicit Settings(std::string name);
    ~Settings();
    Settings(Settings&&) noexcept;
    Settings& operator=(Settings&&) noexcept;
    Settings(const Settings&);
    Settings& operator=(const Settings&);
    std::string Name() const;
};
```

Impl의 완전한 정의는 cpp에 둡니다. unique\_ptr의 삭제가 인스턴스화되는 지점에서 완전한 Impl이 필요하므로 소멸자 등을 적절히 cpp에서 정의합니다. 이번 Settings는 복사 시 Impl을 깊게 복사하고, 이동 후 비어 있는 상태를 명시적으로 허용합니다. Name은 비어 있으면 logic\_error, Rename은 다시 유효한 데이터를 만드는 계약입니다.

Pimpl은 복사 후 swap과 같은 단어가 아닙니다. 구현 의존성을 숨기는 조직 방식이며, 그 안에서 강한 예외 보장을 위한 교체를 사용할 수 있을 뿐입니다. 포인터 간접 접근, 할당, 인라이닝 제약 등의 비용이 있어 모든 Vec3나 작은 컴포넌트에 기계적으로 적용하지 않습니다.

실습은 <code>pimpl.hpp</code>, <code>pimpl.cpp</code>, <code>pimpl_main.cpp</code>를 실제로 분리 빌드합니다. 이것은 특정 DLL ABI 호환성이나 플러그인 버전 호환성을 검증한 것은 아닙니다.

## 3. inline의 의미와 최적화를 구분

inline은 컴파일러에 반드시 호출을 없애라는 명령이 아닙니다. C++에서 여러 번역 단위에 정의를 둘 수 있는 조건과 ODR에 관련된 의미를 먼저 봅니다. 클래스 안 정의가 암묵적으로 inline인 일반적인 헤더 기반 코드와 C++20 named module 등 예외 문맥을 구별합니다.

컴파일러는 inline 키워드가 없는 함수도 펼칠 수 있고, 붙어 있어도 펼치지 않을 수 있습니다. 함수 포인터 호출도 최적화기가 대상을 알아내면 직접화할 수 있습니다. 헤더에 모든 코드를 밀어 넣거나 가상 함수를 없애는 것만으로 빠른 코드가 된다는 판단은 하지 않습니다.

## 4. 할당 함수와 객체 생성

new 표현식은 보통 저장 공간을 확보하고 생성자를 실행합니다. operator new는 저장 공간 확보를 맡는 함수이며 생성자와 같지 않습니다. delete 표현식의 소멸과 저장 공간 해제도 나눠 읽습니다. <code>new[]</code>·<code>delete[]</code>, C 할당과 free의 짝을 유지합니다.

일반적인 throwing new의 할당 실패, <code>new(std::nothrow)</code>의 할당 실패, 생성자 본문이 던지는 예외는 서로 다른 상황입니다. nothrow 할당을 선택했다고 생성자에서 발생하는 예외까지 사라지는 것은 아닙니다. 전역 new\_handler를 바꾸는 코드는 프로세스 전체에 영향을 줄 수 있으므로 작은 학습을 위해 함부로 바꾸거나 실제 메모리를 고갈시키지 않습니다.

## 5. 위치 지정 생성의 두 경로

실습 Widget은 <code>operator new(size_t, Pool&)</code>와 이에 대응하는 <code>operator delete(void*, Pool&)</code>를 제공합니다. 생성자가 실패할 때 대응되는 위치 지정 deallocation 함수가 선택되어 확보한 공간을 돌려줍니다.

반면 생성에 성공한 객체를 나중에 지울 때는 단순히 <code>delete p</code>가 처음의 모든 추가 인수를 기억해 같은 함수를 부르는 것으로 생각하면 안 됩니다. 이번 코드는 unique\_ptr의 custom deleter가 소멸과 Pool 반환을 맡습니다. Pool이 먼저 소멸하지 않도록 소유 관계도 맞춥니다.

```c++
struct ReturnToPool {
    Pool* pool;
    void operator()(Widget* p) const noexcept {
        if (p) {
            std::destroy_at(p);
            pool->Free(p);
        }
    }
};
```

위는 전체 allocation.cpp 중 발췌입니다. 또 다른 예에서는 <code>alignas(int)</code>로 정렬된 자동 byte 배열 안에 <code>::new(storage) int(42)</code>로 객체를 구성하고 std::destroy\_at으로 수명만 끝냅니다. <b>자동 배열의 주소를 delete하지 않습니다.</b> 자원 확보 방식과 객체 수명 시작·끝을 구분하는 연습입니다.

## 6. pmr로 한정된 할당 정책 실험

std::pmr은 메모리 자원 인터페이스를 통해 컨테이너가 사용할 할당 정책을 선택하게 합니다. 실습은 고정된 1024바이트 버퍼와 null upstream 자원을 가진 monotonic\_buffer\_resource를 사용합니다. vector가 그 자원을 쓰도록 연결하고, 버퍼보다 큰 요청은 bad\_alloc으로 실패하게 합니다.

이 방식은 실제 시스템 메모리를 고갈시키지 않고 할당 실패를 재현합니다. monotonic 자원은 개별 해제로 모든 저장 공간을 바로 되돌리는 일반 힙과 다르고, 자원을 사용하는 컨테이너보다 자원 자체가 오래 살아 있어야 합니다. 캐시·속도·단편화 개선을 주장하려면 별도 측정이 필요합니다.

## 7. 실습 파일 지도

<table fit-page-width="true" header-row="true">
<tr>
<td>
실행 파일
</td>
<td>
소스
</td>
<td>
이번에 검사한 핵심
</td>
</tr>
<tr>
<td>
lifetime
</td>
<td>
lifetime.cpp
</td>
<td>
생성·소멸 순서, 부분 생성 실패, Health 불변식
</td>
</tr>
<tr>
<td>
ownership
</td>
<td>
ownership.cpp
</td>
<td>
깊은 복사, 실패 주입, 자기 대입, 이동 후 계약, 복사 생략
</td>
</tr>
<tr>
<td>
polymorphism
</td>
<td>
polymorphism.cpp
</td>
<td>
가상 호출·기본 인수·삭제·캐스팅·Clone·NVI
</td>
</tr>
<tr>
<td>
templates
</td>
<td>
templates.cpp
</td>
<td>
concept·의존 이름·전달 참조·람다 수명
</td>
</tr>
<tr>
<td>
containers
</td>
<td>
containers.cpp
</td>
<td>
컨테이너 수정·조회·참조 안정성·비교자
</td>
</tr>
<tr>
<td>
interfaces
</td>
<td>
interfaces.cpp
</td>
<td>
explicit·const iterator·비소유 뷰·대칭 연산
</td>
</tr>
<tr>
<td>
transaction
</td>
<td>
transaction.cpp
</td>
<td>
임시 상태와 commit, 실패 후 보존, weak\_ptr
</td>
</tr>
<tr>
<td>
pimpl
</td>
<td>
pimpl.cpp + pimpl\_main.cpp
</td>
<td>
분할 빌드, 깊은 복사·이동·ADL swap
</td>
</tr>
<tr>
<td>
allocation
</td>
<td>
allocation.cpp
</td>
<td>
위치 지정 생성 실패·deleter·한정 자원
</td>
</tr>
</table>

전체 소스와 CMakeLists, 테스트 실행 스크립트, 로그는 두 상위 문서에서 연결하는 공통 실습 팩에 있습니다. 문서에 표시한 코드는 설명용 발췌이며 전체 파일과 main은 실습 팩을 사용합니다.

## 8. 실제 실행 결과

GCC 14.2.0, Clang 17.0.0, Clang 17.0.0 + ASan·UBSan의 C++20 Debug 빌드에서 <b>정상 실행 9/9, 조건 검사 89회</b>가 각각 통과했습니다. 세 번 실행했으므로 독립 테스트가 267개라는 뜻은 아닙니다. 정상 빌드는 <code>-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror</code>로 경고를 오류로 처리했습니다.

별도로 의도적으로 잘못된 소스 6개를 두 컴파일러에서 <code>-c</code>로 검사했습니다. 잘못된 override, unique\_ptr 복사, 값 매개변수 복사 생성자 선언, 제약 위반, 임시 객체에서 금지한 뷰 반환, list 반복자에 std::sort가 각각 거부되었습니다. main이 없어서 링크만 실패한 것이 아니도록 컴파일 단계만 검사했습니다.

이번 검증은 <b>Linux x86-64</b>입니다. Windows/MSVC, DirectX·GPU, 실제 회사 코드, 멀티스레드 경쟁 조건, ABI 호환성과 성능 측정은 실행 검증하지 않았습니다. sanitizer 무진단도 실행하지 않은 모든 경로의 안전성 증명은 아닙니다.

## 9. 빌드와 디버깅

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Visual Studio 같은 다중 구성 생성기를 쓰는 Windows에서는 build 명령에 <code>--config Debug</code>, ctest에 <code>-C Debug</code>를 붙입니다. MSVC 프로젝트 설정은 제공했지만 여기서 실제 실행하지는 않았습니다. run\_[checks.py](http://checks.py)는 GCC·Clang과 CMake가 설치된 환경에서 세 구성을 실행하는 별도 도구입니다.

실행 전 예측한 결과를 적고 중단점을 생성자·대입 연산자·소멸자에 둡니다. 실패 예제는 먼저 오류 메시지를 읽고 어떤 선언·계약이 어긋났는지 찾아봅니다. 경고를 없애려고 무작정 캐스트를 추가하지 않습니다. 플랫폼 도구의 단축키는 설치된 IDE 설정에 따라 달라질 수 있으므로 기능 이름을 기준으로 찾습니다.

## 이해 확인

<toggle>
Pimpl과 copy-and-swap은 같은 기법일까?

Pimpl은 공개 선언에서 구현 데이터를 분리하는 구조입니다. copy-and-swap은 실패할 수 있는 준비를 사본에서 한 뒤 교체하는 대입 등의 구현 전략입니다. 각각 독립적으로 사용할 수 있습니다.
</toggle>

<toggle>
위치 지정 new가 성공했으니 대응 위치 지정 delete가 나중에 자동 선택될까?

생성 실패 시의 정리와 정상 생성 뒤 사용 종료는 별도입니다. 이 실습은 실패 경로에는 대응 위치 지정 delete, 성공 경로에는 custom deleter를 사용합니다. 저장 공간을 제공한 주체와 수명을 함께 확인해야 합니다.
</toggle>

## 출처

[N4861 — inline](https://timsong-cpp.github.io/cppwp/n4861/dcl.inline), [new 표현식](https://timsong-cpp.github.io/cppwp/n4861/expr.new), [delete 표현식](https://timsong-cpp.github.io/cppwp/n4861/expr.delete), [메모리 자원](https://timsong-cpp.github.io/cppwp/n4861/mem.res), [Clang ASan](https://clang.llvm.org/docs/AddressSanitizer.html), [Clang UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).
