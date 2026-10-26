# Effective Modern C++ 03 · 소유권 그래프와 약한 관찰자

리소스 공유와 엔티티 참조를 서로 다른 간선으로 설계한다

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
ownership: strong resource / weak observer
PASS: 9 checks
```

Check 9개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
asset.reset(), b.reset(), a.reset() 세 위치에서 Texture::alive와 observer.lock()의 성공 여부를 먼저 예상한다. b가 제거되어도 a가 텍스처를 보유한다는 점과 target만 만료되는 점을 별도로 확인한다. 리소스 수명과 엔티티 수명은 같은 타임라인이 아니다.

Texture id는 예제에서 7과 42만 사용한다. alive는 단일 스레드에서만 변경한다. 동시 소멸·allocator 실패 주입·실제 GPU 리소스 지연 해제는 검증하지 않는다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/main.cpp
