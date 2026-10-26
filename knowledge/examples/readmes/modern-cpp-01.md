# Modern C++ 01 · 표준 버전·컴파일러·기능 검사

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

새 SDK나 라이브러리를 붙일 때 “C++20 지원”이라는 한 문장 대신 팀의 실제 컴파일러·표준 라이브러리·빌드 옵션을 기록한다. 이 과정의 기본 예제는 C++20이며, 10장의 expected만 C++23 추가 실행을 구분한다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_01
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
mode=C++20-or-later span=available
PASS: 3 contract checks
```

check 호출 3회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

C++17 모드로 이 파일을 별도 빌드해 요구 버전 오류를 읽어 보자. 이 실패는 기능 미지원 확인 실습이며 성공 빌드로 집계하면 안 된다. CI에 넣을 경우 “예상 실패”임을 명시하고 실제 성공 경로와 분리한다.

빌드 서버와 개발 PC의 컴파일러 이름이 같아도 버전·표준 라이브러리·플래그가 다를 수 있다. 자동화에는 정확한 도구 버전과 커맨드, 소스 해시를 남긴다. __cplusplus만으로 Windows/MSVC의 전체 지원을 추정하지 말고 해당 도구의 모드 설정 문서도 확인한다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: dfdaa12b179523a9ee7b2d1dcb5ea79332b6d4c12b66b3108ce843f9c40e7a15.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
