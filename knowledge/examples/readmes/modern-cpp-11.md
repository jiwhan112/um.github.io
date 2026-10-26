# Modern C++ 11 · concepts·ranges·지연 평가

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

살아 있는 캐릭터의 체력을 읽는 파이프라인을 작성한다. 간결한 표현 뒤에 숨어 있는 수명과 반복자·캐시 계약을 따로 확인한다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_11
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
snapshot=4,7 live-first=5 matching=2
PASS: 5 contract checks
```

check 호출 5회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

체력 합계를 여러 번 계산하는 UI에서 view 순회와 snapshot 저장 중 무엇을 택할지 비교하자. 값이 자주 바뀌는지, 비용이 큰지, 소유자 수명은 어떤지 세 기준으로 답한다.

range-for가 간단하다는 이유로 비용을 상수 시간이라고 생각하지 않는다. filter는 조건을 만족하는 다음 요소를 찾으려고 여러 원소를 읽을 수 있고, 파이프라인을 반복 순회하면 계산을 반복할 수 있다. 필요하면 한 번 materialize하고 갱신 시점을 명시한다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: 43305b12f21cd9f8f8a2d454dc61d4bef83647373da24447795d408fcd35adda.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
