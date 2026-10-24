# C 입력 계약과 C++ 검사 모델 · 예제 다운로드

## 서로 다른 두 프로그램

`original/`은 보존 원본, `ko/`는 한국어 주석 전체 코드입니다. C11의 scanf 예제와 C++11 이상의 엄격한 한 줄 입력·checked-add 모델은 별개입니다. 아래는 POSIX 셸에서 ko를 직접 컴파일하는 명령입니다.

```sh
cd project
cc -std=c11 -Wall -Wextra -Wpedantic -Werror ko/c11/main.c -o c-input
printf '3 4\n' | ./c-input
c++ -std=c++11 -Wall -Wextra -Wpedantic -Werror ko/c01b/main.cpp -o checked-add
./checked-add --test
printf '3 4\n' | ./checked-add
```

C11 출력:

```text
You entered: 3 and 4
Sum: 7
```

C++의 `--test` 출력은 `PASS 10 independent conditions`, 입력 실행 출력은 `7`입니다. 셋 모두 정상 종료 코드는 0입니다. C11 예제에 숫자 변환을 못하는 입력을 주면 `Expected two integers.`와 1을 반환합니다. scanf로 int 범위를 넘는 숫자를 안전하게 거부한다고 가정하지 마세요. 범위 검사가 필요한 입력은 별도 C++ 모델의 계약과 구별합니다.

## 보존 원본 CMake로 검사하기

```sh
cmake -S original/c01b -B build-checked -DCXX_LEVEL=11
cmake --build build-checked --config Debug
ctest --test-dir build-checked -C Debug --output-on-failure
```

C++ 원본은 `C01CheckedAdd` 1개 테스트가 통과하는 것이 정상입니다. 별도 `original/c11/CMakeLists.txt`는 C와 C++ 양쪽으로 컴파일하며 테스트에 bash·printf·grep과 32비트 int 입력 경계를 사용합니다. C11 쪽 CTest를 Windows 네이티브 셸/모든 정수폭 환경에서 바로 동작하는 범용 검사로 오해하지 마세요. 이 ZIP은 SDK·컴파일러·셸을 포함하지 않습니다.


## 원본과 검증 경계

이 팩은 기존 공개 교재의 실제 전체 소스만 묶습니다. 기존 코드 바이트는 바꾸지 않았습니다. ZIP 루트의 이 README만 다운로드용 안내입니다. 외부 페이지·원본 첨부·검증 로그를 실행 의존성으로 넣지 않았습니다.

패키징 단계에서는 CPU 검사를 재실행하지 않았습니다. 기존 실행 기록과 소스 해시 대조는 신규 빌드 성공이나 사용자의 플랫폼 실행을 뜻하지 않습니다. 압축 해제 뒤 위 명령을 직접 실행한 결과와 구분하세요. Windows/MSVC·GPU·성능에 대한 신규 검증은 없습니다.

교재: https://jiwhan112.github.io/um.github.io/knowledge/c-language-01/
