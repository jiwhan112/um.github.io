# Effective Modern C++ 07 · 작업·결과·원자성을 각각 설계하기

async 정책, future 오류, const 캐시의 mutex와 relaxed 카운터를 분리한다

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
tasks: explicit policy / exception / join / atomic
PASS: 8 checks
```

Check 8개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
deferred 상태를 ready와 timeout과 구분한다. 작업에서 던진 예외가 어디서 다시 나타나는지 호출 스레드 관점으로 설명한다. 마지막 2000은 두 스레드가 끝난 뒤 확인한 값이다. join을 없애거나 참조 수명 밖으로 작업을 보내는 위험한 변형은 실행하지 않는다.

고정된 두 카운터 작업과 작은 캐시만 사용한다. 성능·lock-free 여부·실제 프레임 시간·ThreadSanitizer·모든 스케줄 탐색은 검증하지 않는다. 메모리 순서 교재 전체를 이 카운터 하나로 대체하지 않는다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/main.cpp
