# Modern C++ 12 · jthread·협력적 취소·도입 경계

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

리소스 로딩 작업을 멈출 때 “요청했다”와 “끝났다”를 구분한다. C++20의 도구로 종료 경로를 만든 뒤, C++23·26 도입을 기능별 검증으로 제한하는 기준을 정한다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_12
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
stop-requested joined=true callback=1
PASS: 7 contract checks
```

check 호출 7회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

작업이 준비됨과 취소 요청이 거의 동시에 일어나면 어느 결과를 우선할지 API 계약을 적어 보자. predicate가 참이면 작업을 처리할지, 취소를 먼저 확인할지 정하지 않으면 테스트 결과만으로 정책을 만들게 된다.

stop_callback은 등록 시 이미 중지된 상태이거나 요청을 실행하는 경로에서 동기 호출될 수 있으므로, 콜백 안에서 같은 잠금을 다시 얻으려는 설계는 주의한다. 예제의 별도 callback 검사는 현재 스레드에서 순차 수행하며, 콜백의 일반 스케줄링을 측정한 것이 아니다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: bec640294af7151a602c7981d44d08ceff25e42d26b9c405b7bb9b68ab2efac3.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
