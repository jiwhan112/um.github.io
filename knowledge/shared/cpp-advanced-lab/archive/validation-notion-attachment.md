# C++ 심화 · Effective C++ 실습 검증 기록 v1.0

작성일: 2026-10-13. 학습 기준 C++20.

## 실제 실행 환경

Linux x86-64, GCC 14.2.0, Clang 17.0.0, CMake/CTest.
정상 빌드는 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror를 사용했습니다.
Clang 별도 구성에 -fsanitize=address,undefined -fno-omit-frame-pointer를 적용했습니다.

| 실행 파일 | 조건 검사 수 | 주제 |
|---|---:|---|
| lifetime | 10 | 생성·소멸 순서, 부분 생성 실패, 체력 불변식 |
| ownership | 13 | 복사·이동·자기 대입·할당 실패 주입 |
| polymorphism | 12 | 가상 호출·기본 인수·캐스팅·삭제·Clone·NVI |
| templates | 10 | concept·typename·의존 이름·forward·capture |
| containers | 16 | 삽입·삭제·조회·반복자·비교자 |
| interfaces | 7 | explicit·const iterator·뷰 수명·대칭 연산 |
| transaction | 8 | 준비·검증·교체·weak_ptr |
| allocation | 7 | 위치 지정 생성 실패·deleter·한정 pmr 자원 |
| pimpl | 6 | h/cpp 분리·깊은 복사·이동·ADL swap |
| 합계 | 89 | 독립 프로그램 9개 |

GCC, Clang, Clang ASan/UBSan 구성 각각 정상 실행 9/9, 조건 검사 89회 통과했습니다.
같은 조건을 세 번 실행했으므로 독립 조건 267개를 뜻하지 않습니다.

## 컴파일 거부 검사

아래 6개 파일은 GCC와 Clang 모두 -std=c++20 -pedantic-errors -c로 거부됨을 확인했습니다.
-c를 써서 main 부재로 인한 링크 실패와 구분했습니다. 이 파일들은 정상 빌드에서 제외합니다.

### negative/wrong_override.cpp
```cpp
struct Base { virtual int Read() const = 0; };
struct Derived : Base { int Read() override { return 1; } };
```
### negative/copy_unique.cpp
```cpp
#include <memory>
void F() { auto a = std::make_unique<int>(1); auto b = a; }
```
### negative/copy_by_value.cpp
```cpp
struct Bad { Bad(Bad); };
```
### negative/constraint.cpp
```cpp
#include <concepts>
template<std::integral T> T F(T value) { return value; }
int main() { return F(1.5); }
```
### negative/temporary_view.cpp
```cpp
#include <string>
#include <string_view>
struct Label {
    std::string data;
    std::string_view View() const & { return data; }
    std::string_view View() && = delete;
};
int main() { auto view = Label{"temporary"}.View(); return static_cast<int>(view.size()); }
```
### negative/list_sort.cpp
```cpp
#include <algorithm>
#include <list>
void F() { std::list<int> values{3,1,2}; std::sort(values.begin(), values.end()); }
```

## 빌드

전체 ZIP의 루트에서:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```
Visual Studio 생성기를 쓰면 build에 --config Debug, ctest에 -C Debug를 붙입니다.
MSVC 명령은 제공했지만 여기서 실행 검증하지 않았습니다.

GCC와 Clang이 모두 설치되어 있다면 python3 run_checks.py로 세 빌드와 부정 예제를 재현합니다.
새 빌드 폴더와 logs에 결과를 쓰며 회사 프로젝트나 시스템 설정을 변경하지 않습니다.

## 배포 파일

대화 첨부 cpp_advanced_effective_lab_v1.zip에 전체 소스, CMakeLists.txt, README, run_checks.py, 원시 로그와 report.json이 들어 있습니다.
ZIP SHA-256: a97a4927e5d3472ad055dbffbad0e14ab72c4c81f6ea13a1d72b771d64d339ba
대화 첨부 cpp_advanced_effective_sources_v1.md는 파일별 코드의 텍스트 묶음입니다.

ZIP을 Notion 바이너리 첨부로 직접 전송하려 했지만 실행 환경에서 업로드 호스트 이름을 해석하지 못했습니다. 이 검증 기록은 Notion 텍스트 첨부로 보존하며 바이너리 업로드 성공을 의미하지 않습니다.

## 한계와 계약

- Windows/MSVC, DirectX/GPU, 회사 프로젝트 통합은 실행 미검증입니다.
- 멀티스레드 경쟁 조건, ABI 호환성, 성능 측정은 수행하지 않았습니다.
- sanitizer 무진단은 실행하지 않은 경로까지 안전하다는 증명이 아닙니다.
- IntBuffer는 int 전용 학습 타입입니다. 범용 vector 대체품이 아닙니다.
- Counter·Quantity·AddLater 등은 예제의 작은 정수를 전제로 합니다. 범용 범위 안전 수치 API가 아닙니다.
- GameState의 강한 보장은 자체 소유한 동기 상태에 한정되며 파일/네트워크 부수 효과의 롤백이나 동시성 원자성을 제공하지 않습니다.
- Pool은 단일 Widget 교육 예입니다. 전역 new 교체, 범용 정렬 지원, 멀티스레드 할당자는 아닙니다.
- 실제 메모리를 고갈시키지 않고 실패 주입·한정 자원으로 실패를 시험했습니다.
- C++20은 학습 기준이며 C++11 프로젝트 표준 변경을 지시하는 것이 아닙니다.
