# Modern C++ 04 · 값 범주·이동·noexcept

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

게임 자원의 소유권을 이전할 때 원본·대상·실패 보장을 한 표로 추적한다. 이동을 붙이면 무조건 빨라진다는 식의 추측 대신 호출 가능한 생성자와 예외 조건을 본다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_04
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
unique-owner-transferred copies=2 moves=0
PASS: 5 contract checks
```

check 호출 5회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

Resource에 추가 메타데이터를 넣을 때 default 이동 후에도 클래스 불변식이 유지되는지 검토하자. 파일 핸들처럼 별도 무효 값을 가진 자원에는 단순한 멤버 복사형 이동이 중복 해제를 만들지 않는지도 설명해 보자.

복사 횟수를 특정 vector 재할당 구현의 세부 동작으로 단정하지 않는다. 여기서는 move_if_noexcept를 직접 호출하여 표준 선택 계약을 좁게 검사한다. allocator·사용자 생성자·복사 생략에 따라 다른 프로그램의 호출 수는 달라질 수 있다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: 4bbd62a34bde17f282b077c414977f1b65750e1b421cd519057c9a8baa2c50ca.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
