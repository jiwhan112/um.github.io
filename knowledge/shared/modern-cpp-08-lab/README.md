# Modern C++ 08 · 가변 인자·fold·인덱스 팩

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

게임 명령들의 작은 비용 목록을 합산하는 함수를 만들며 인자 수·타입·평가 순서를 명시한다. printf식 비타입 가변 인자와 템플릿 팩을 혼동하지 않는다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_08
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
empty=0 pack=9 tuple=11 overflow=rejected
PASS: 5 contract checks
```

check 호출 5회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

from_tuple이 value()를 호출하지 않고 optional<uint32_t>를 반환하도록 바꾸어 초과 합을 호출자까지 전달해 보자. 합산 기능과 오류 처리 정책을 분리하면 어떤 호출자가 더 안전해지는지 적는다.

fold 식이라고 모든 연산의 부작용 순서가 자동으로 원하는 대로 정해지는 것은 아니다. 사용한 연산자의 시퀀싱 규칙을 확인해야 한다. 예제의 쉼표와 다른 연산자를 기계적으로 바꾸면 의미가 달라질 수 있다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: 5f3a7d1f76e9c9758e596175fcd28d39422889631f7cf68a059135446663c480.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
