# Effective Modern C++ 05 · 완벽 전달의 불완전한 경계

forwarding 생성자의 복사 가로채기와 타입 없는 중괄호 입력을 제약으로 다룬다

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
forwarding: constrain inputs and materialize exceptions
PASS: 6 checks
```

Check 6개는 프로그램 안의 검사 호출 수이며 static_assert나 빌드 모드 반복을 합산하지 않습니다. Check는 assert 매크로가 아니므로 NDEBUG 빌드에서도 실행됩니다.

## 학습 계약
Label b(a)가 템플릿 본문으로 들어가지 않고 복사로 해결되는 이유를 requires의 두 조건으로 설명한다. values 이름을 없애고 중괄호를 직접 Call에 넘기면 어떤 단계가 실패할지 예측한다. 실제 제출 예제에는 의도적 컴파일 오류를 넣지 않으며 지원하지 않는 호출을 문서와 정적 타입 검사로 구별한다.

C++20 requires를 사용한다. C++11/14 SFINAE 버전과의 컴파일 호환성을 주장하지 않는다. 고정된 ASCII 이름만 테스트하며 UTF-8 문자 단위 편집·정규화·길이 제한은 구현하지 않는다.

## 실제 검증
2026-10-26 Linux x86_64, g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6에서 Debug 구성·빌드·CTest 1/1, -O2 -DNDEBUG 직접 컴파일·실행, UBSan 직접 컴파일·실행 통과. 명령과 출력 및 소스 해시는 교재에 연결된 verification.json에 있습니다.

ASan+UBSan은 과정 01에서 LeakSanitizer의 ptrace 환경 제한 오류로 실패했으며 이 과정의 ASan 통과를 주장하지 않습니다. 검사를 끄지 않았습니다. Windows/MSVC·GPU·성능·ThreadSanitizer·게임 통합은 미검증입니다.

## 코드 파일
- project/CMakeLists.txt
- project/main.cpp
