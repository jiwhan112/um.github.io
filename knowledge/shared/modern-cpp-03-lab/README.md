# Modern C++ 03 · 초기화·nullptr·범위 있는 enum

새로 작성한 C++20 교육 예제입니다. 기존 책·Notion의 원본 프로그램을 복원한 것이 아닙니다.

## 목표와 전제

인벤토리 슬롯 생성 코드와 네트워크 상태 코드를 예로, 짧은 문법 변경이 의미를 바꾸는 지점을 찾는다. 이미 배운 자료형 문법을 반복하기보다 안전한 경계 설계에 집중한다.

- UTF-8 소스, C++20 컴파일러/표준 라이브러리, CMake 3.16 이상이 필요합니다.
- 표준입력, 외부 파일, 네트워크는 사용하지 않습니다. 고정 입력은 소스에서 바꿉니다.
- 실행 파일은 포함하지 않습니다. ZIP을 빈 폴더에 풀고 README 위치에서 실행합니다.

## 구성·실행

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/modern_cpp_03
```

Windows의 다중 구성 생성기는 `cmake --build build --config Release`와 `ctest --test-dir build -C Release --output-on-failure`를 사용합니다. Windows/MSVC 실행은 이번 검증에 포함하지 않았습니다.

직접 빌드할 경우:

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread project/main.cpp -o lesson
./lesson
```

## 실제 기본 출력

```text
parentheses=3-elements braces=2-elements invalid-mode=rejected
PASS: 7 contract checks
```

check 호출 7회이며, static_assert는 별도의 컴파일 시 검사입니다. 여러 최적화에서 반복 실행한 횟수를 독립 조건 수로 합산하지 않습니다.

## 직접 바꾸어 볼 경계

상태 코드 2를 Dash로 추가한다고 가정하자. enum 선언과 decode_mode 허용 분기, 성공 검사와 거절 검사를 함께 바꾸고, 257처럼 기저형을 넘는 값이 여전히 거절되는지 확인하자.

중괄호와 auto를 함께 쓰면 auto x{7}과 auto y={7}도 같은 타입이 아니다. 현재 검증 모드에서는 x가 int, y가 initializer_list<int>임을 static_assert로 확인한다. 리스트의 내부 저장소를 장기 보관하는 API는 그 수명도 따로 검토해야 한다.

## 검증 범위

2026-10-26 Linux GCC 14.2.0에서 C++20 -O0, -O2 UBSan, CMake 3.31.6 Release CTest 1/1을 실제 실행했습니다. 소스 SHA-256: 9b40a26ed1d9afd40656be95a39a4e4f5b0f0a125f9b520ad0712ed0b22ca422.

Windows/MSVC·Clang·ThreadSanitizer·GPU·실제 엔진 통합·성능 비교·C++26 실행은 미검증입니다. UBSan 통과는 모든 오류 부재를 보장하지 않습니다.

## MSVC 언어 모드 매크로 설정

MSVC에서는 /std:c++20만으로 __cplusplus가202002L로 바뀐다고 가정하면 안 된다. 기본 호환 동작에서는199711L이 유지될 수 있으므로 이 과정의 CMake는 /Zc:__cplusplus를 함께 지정한다. _MSVC_LANG는 이 옵션과 독립적으로 선택한 표준 모드를 보고한다. 이 설정은 공식 문서로 대조했으며 Windows/MSVC에서 실제 빌드·실행한 결과는 아니다.

공식 근거: https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
