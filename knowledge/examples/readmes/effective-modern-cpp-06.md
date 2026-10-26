# Effective Modern C++ 06 · 콜백이 기다리는 동안 누가 살아 있는가

값 캡처·참조 캡처·weak_ptr·이동 캡처를 HUD와 일회성 작업으로 비교한다

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
capture: snapshot / borrow / weak / move ownership
PASS: 8 checks
```

Check 8개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
값 캡처와 참조 캡처 결과를 먼저 예상하고, HUD 생존·변경·소멸 세 상태에서 callback 반환값을 적는다. oneShot은 한 번만 호출하는 정상 경로뿐 아니라 두 번 호출하는 반복 경로도 확인한다. reference capture를 오래 보관하는 잘못된 버전은 실행하지 않는다.

실제 시간 지연·작업 스케줄러·UI 스레드 전환은 없다. 정수와 ASCII 문자열로 수명 및 반복 호출만 확인한다. weak_ptr는 데이터 경쟁을 해결하지 않으며 실제 동시 콜백은 검증하지 않는다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/main.cpp
