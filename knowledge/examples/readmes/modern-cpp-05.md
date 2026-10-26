# Modern C++ 05 · unique·shared·weak 소유 관계

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

씬 트리의 부모·자식 연결을 예로, 강한 소유와 약한 관찰을 구별한다. 공유 포인터를 모든 곳에 붙이는 대신 객체를 언제 없애야 하는지부터 정한다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_05
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
tree-destroyed alive=0 weak-lock=empty
PASS: 8 contract checks
```

check 호출 8회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

씬 노드가 삭제되어도 이벤트 큐가 안전하게 무시할 수 있게 weak_ptr를 캡처하는 콜백을 설계해 보자. 큐 자체가 shared_ptr로 노드를 붙잡으면 “논리적으로 삭제된 노드”가 살아 있을 수 있는 차이도 적는다.

raw pointer에서 이미 다른 shared_ptr가 관리하는 객체를 다시 독립 소유로 만들지 않는다. 같은 주소라도 제어 블록이 둘이면 이중 해제 위험이 있다. 또한 use_count가 특정 숫자라는 사실을 동시성 제어 도구나 유일성 증명으로 사용하지 않는다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: 8202df57b26d7fd2cd893a900c93161d50fe2e1a10e36d2749bd44a6bb538cc5.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
