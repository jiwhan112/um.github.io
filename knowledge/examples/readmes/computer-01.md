# 컴퓨터 구조 01 · 새 CPU 명령 추적 모델

이 소스는 기존 프로그램을 복구한 것이 아니라, 개념을 확인하려고 새로 쓴 C++11 교육 모델입니다. 실제 CPU/ISA 에뮬레이터가 아닙니다. 프로그램은 네 레지스터, 여덟 32비트 메모리 셀, 명령 배열 인덱스 PC를 사용합니다. 실제 바이트 주소·캐시·OS·I/O를 구현하지 않습니다.

## 준비와 실행

C++11 지원 컴파일러(GCC/Clang 등)와 CMake 3.16 이상이 필요합니다. ZIP을 새 빈 폴더에 풀고 이 README가 있는 위치에서 실행하세요. 실행 파일은 들어 있지 않습니다.

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

직접 실행(Linux/macOS): `./build/instruction_trace`

```text
LOAD[2]=7 ADD=14 STORE[3]=14 HALT
PASS: 19 contract checks
```

표준입력·외부 파일·인터넷은 필요 없습니다. CTest 1개가 상태/오류 경계 19조건을 확인하며, 19개의 독립 성능 실험이라는 뜻이 아닙니다. 조건 실패는 예외로 비정상 종료됩니다. `assert`의 NDEBUG 해제 여부로 검사 결과가 사라지는 구조가 아닙니다.

## 코드 읽기

- LOAD/STORE 인덱스는 0–7, 레지스터 인덱스는 0–3입니다. 검사 후 접근합니다.
- ADD는 uint32_t의 2^32 모듈러 덧셈입니다. signed overflow를 유발하지 않습니다.
- HALT와 명령 배열의 끝을 구분합니다. budget은 Cpu.steps의 누적 최대 허용값입니다.
- Cpu를 재사용하면 레지스터·PC·steps도 유지됩니다. 새 실행을 비교하려면 새 Cpu를 만드세요.
- 잘못된 Op/주소/레지스터와 실행상한은 일반 C++ 반환 상태로 처리합니다. 실제 page fault/인터럽트를 발생시키지 않습니다.

## 실제 확인 범위

2026-10-24 클라우드 Linux GCC 14.2.0, CMake 3.31.6, Debug C++11의 깨끗한 구성/빌드/CTest 1/1 통과. 내부 경로를 가린 실행 기록과 파일 해시는 교재의 `knowledge/shared/computer-01-lab/verification.json`에 따로 있습니다. Windows/MSVC·C++20 별도 빌드·실제 ISA·캐시 타이밍·GPU는 검증하지 않았습니다. CMake의 MSVC 분기는 실행 가능성을 보장한 테스트 기록이 아닙니다.
