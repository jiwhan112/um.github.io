# Effective Modern C++ 02 · 초기화와 인터페이스에서 실수를 거절하기

중괄호·nullptr·삭제 함수·override를 피해량 API의 허용 범위로 연결한다

## 준비
C++20 지원 컴파일러와 CMake 3.16 이상, 스레드 라이브러리가 필요합니다. 소스는 UTF-8입니다. 실행 파일은 제공하지 않습니다. ZIP을 빈 폴더에 푼 뒤 README가 있는 위치에서 실행합니다.

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/lesson
```

Windows 다중 구성 생성기를 선택하면 `cmake --build build --config Debug`, `ctest --test-dir build -C Debug --output-on-failure`를 사용합니다. 해당 Windows 실행은 이번에 검증하지 않았습니다.

## 기대 출력
```text
interface: construction and overloads explicit
PASS: 8 checks
```

Check 8개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
먼저 두 벡터의 길이와 원소를 예측한다. Damage(double)가 삭제된 상태에서 is_constructible 결과를 확인한다. 그런 다음 Damage(-1)의 실행 오류와 타입 수준 거절이 왜 다른지 정리한다. Armor의 하한 검사를 0, 1, 2, 3으로 확장해 경계가 어디인지 찾는다.

검사 입력은 작은 고정 정수다. 계산은 hp-2 전에 hp>2를 확인한다. 일반적인 모든 정수 연산의 오버플로를 검증한 프로그램은 아니며 파일 입력·네트워크 직렬화는 포함하지 않는다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/main.cpp
