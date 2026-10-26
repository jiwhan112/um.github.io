# Modern C++ 02 · auto·decltype와 참조 추론

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

함수 반환형을 짧게 쓰는 것보다 “새 값인가, 기존 객체를 빌리는가”를 먼저 결정한다. UI나 게임 상태 접근자의 괄호 하나가 참조 반환으로 바뀌는 상황을 안전하게 추적한다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_02
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
copy=5 actor.hp=17 snapshot=true proxy=false
PASS: 4 contract checks
```

check 호출 4회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

borrow_hp의 반환형을 int로 바꾸고 대입식이 왜 거부되는지 생각하자. 그런 다음 임시 Actor를 빌리는 API를 만들지 않도록 참조 한정 멤버 함수나 명시적인 값 반환 중 적합한 설계를 제안해 보자.

이름 붙은 rvalue reference 변수는 식으로 사용하면 lvalue다. auto&& named의 선언형이 Actor&&여도 decltype((named))는 Actor&다. &&를 보면 무조건 이동된다고 판단하지 말고 실제 호출 위치의 값 범주를 본다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: f6c2044522e71aa91e06f60e4b935bf4e1694f542b25abd4cf93fd014e92976d.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
