# C 기초 13 · 전처리기·다중 파일 빌드 · 예제 다운로드

## 구성과 실행

압축을 새 빈 폴더에 풀면 `README.md`와 `project/`가 있습니다. C11을 지원하는 GCC 또는 Clang이 필요합니다. 아래는 Linux/macOS의 POSIX 셸 명령입니다. `cc`가 다른 컴파일러라면 `gcc` 또는 `clang`으로 바꾸세요.

```sh
cd project
cc -std=c11 -O0 -Wall -Wextra -Wpedantic -Werror main.c inventory.c -o lesson
./lesson
```

표준입력이나 외부 데이터 파일은 필요 없습니다. 코드 안의 고정 입력과 경계 검사가 실행됩니다. `-DNDEBUG`를 추가하지 마세요. assert가 있는 예제의 검사 의미가 달라집니다.

## 정상 실행의 기대 출력

```text
build: declarations, linkage, inventory contracts OK
```

정상 종료 코드는 0입니다. 실패 진단 또는 비정상 종료가 나오면 성공 출력으로 취급하지 마세요.

`main.c`, `inventory.c`, `inventory.h` 세 파일을 같은 디렉터리에 유지해야 합니다. `main.c`만 링크하면 구현 정의를 찾을 수 없습니다. 헤더만 단독 실행하지 않습니다.

기존 CPU 실행·해시 원장: https://jiwhan112.github.io/um.github.io/knowledge/shared/c-basics-09-14/verification.json


## 원본과 검증 경계

이 팩은 기존 공개 교재의 실제 전체 소스만 묶습니다. 기존 코드 바이트는 바꾸지 않았습니다. ZIP 루트의 이 README만 다운로드용 안내입니다. 외부 페이지·원본 첨부·검증 로그를 실행 의존성으로 넣지 않았습니다.

2026-10-23 Cloud Linux에서 패키징 경로 그대로 복사한 독립 폴더를 검사했습니다: main.c + inventory.c 엄격 C11 빌드와 실행 성공, 기대 출력 일치. GCC 14.2.0을 사용했고 C++ 팩은 CMake 3.31.6 / Debug 구성입니다. ZIP 파일 자체의 압축 해제 검증과는 구별합니다. sanitizer·negative·diagnostics·Windows 검사는 이번에 다시 실행하지 않았습니다. 기존 실행 기록과 소스 해시 대조는 신규 빌드 성공이나 사용자의 플랫폼 실행을 뜻하지 않습니다. 압축 해제 뒤 위 명령을 직접 실행한 결과와 구분하세요. Windows/MSVC·GPU·성능에 대한 신규 검증은 없습니다.

교재: https://jiwhan112.github.io/um.github.io/knowledge/c-basics-13/
