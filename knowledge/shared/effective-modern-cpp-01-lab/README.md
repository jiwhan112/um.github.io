# Effective Modern C++ 01 · 타입 추론은 저장 정책이다

auto·decltype(auto)·대리 객체를 체력 UI의 복사와 관찰 계약으로 구분한다

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
deduction: value / reference / proxy separated
PASS: 7 checks
```

Check 7개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
실행 전에 health[0], detached, snapshot, proxy 네 값을 종이에 적는다. live에 80을 대입한 뒤 어떤 값만 바뀌는지 비교한다. 이어 flags의 원소 변경을 되돌리고 대리 객체와 값 스냅샷의 차이를 설명한다. at(9) 오류는 정상 거절 경로이며 범위 밖 메모리를 읽는 실험이 아니다.

입력은 소스에 고정된 작은 벡터와 정수다. 동시 접근·컨테이너 재할당 후 참조 사용·프레임간 실제 HUD 통합은 하지 않는다. 문자열 인코딩은 UTF-8 주석뿐이며 표준 입력과 바이너리 버퍼를 파싱하지 않는다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/main.cpp
