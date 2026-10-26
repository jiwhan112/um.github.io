# Modern C++ 10 · optional·variant·expected의 오류 모델

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

자원 조회, 로딩 상태, 입력 실패에 서로 다른 반환 모델을 선택한다. 타입 이름보다 호출자가 무엇을 해야 하는지에서 출발한다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_10
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
expected=not-compiled (requires C++23 library)
variant=failed optional=missing
PASS: 5 contract checks
```

check 호출 5회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

parse_count의 오류를 enum class ParseError로 바꾸고 메시지 변환은 UI 계층으로 옮겨 보자. 음수와 범위 초과를 구분하려면 from_chars의 errc와 문법 오류를 어떤 순서로 처리해야 할까?

expected.error()를 성공 상태에서 읽거나 optional의 빈 값을 무검사 역참조하지 않는다. 오류 타입에 enum·구조체를 쓰면 분기와 지역화가 더 명확해질 수 있다. 이 수업의 string 오류는 작은 콘솔 설명을 위한 선택이며 대형 엔진의 표준 오류 정책으로 제시하지 않는다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: 2e23aa008d6fcce32a57b6701c50a17df955381296b73a8d22ebaf500d0df959.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## C++23 expected 추가 실행

`g++ -std=c++23 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson23 && ./lesson23`로 기능이 있는 라이브러리에서 추가 실행합니다. 현재 GCC14에서 실제 실행한 출력:

```text
expected=executed
variant=failed optional=missing
PASS: 8 contract checks
```

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
