# Effective Modern C++ 04 · 이동은 호출 계약이고 forward는 보존이다

복사·이동 횟수를 직접 추적하며 const와 이름 있는 참조의 함정을 구분한다

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
move: casts select operations, const is retained
PASS: 6 checks
```

Check 6개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
코드 실행 전에 copies와 moves를 각 생성 줄 뒤에 적는다. immutable 경로에서 왜 moves가 늘지 않는지 설명한 뒤 출력과 확인한다. Packet의 -1 상태는 이 타입의 문서화된 계약이며 std::string의 이동 후 내용에 같은 기대를 적용하지 않는다.

Packet은 id 정수 하나인 교육용 계수 타입이다. 네트워크 버퍼·동적 할당·실제 이동 비용을 측정하지 않는다. C++20의 생성 규칙을 사용하며 C++11의 선택적 복사 생략 횟수와 비교하지 않는다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/main.cpp
