# Effective Modern C++ 08 · Pimpl·삽입·비용의 통합 판단

새 문법을 모으는 대신 자원 캐시의 빌드 경계·오류 계약·측정 계획을 완성한다

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
integration: explicit ownership and narrow build boundary
PASS: 9 checks
```

Check 9개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
세 파일을 별도로 읽고 헤더에 unordered_map이 없는 이유를 설명한다. 중복 hero 삽입, 없는 키 조회, 음수 값 거절, 이동 후 재사용 순서대로 객체 상태를 표로 적는다. cpp를 빼고 main만 빌드하면 링크할 구현이 없다는 점도 다중 파일 프로젝트의 일부다.

CPU 문자열→정수 맵일 뿐 GPU 리소스 캐시가 아니다. C++20 빌드에서 C++17 try_emplace를 사용한다. 동시 접근·할당 실패 주입·실제 ABI 호환·컴파일 시간·프레임 성능은 미검증이다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/asset_cache.cpp
- project/asset_cache.hpp
- project/main.cpp
