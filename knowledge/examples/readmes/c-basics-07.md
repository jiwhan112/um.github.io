# C 기초 07 · 포인터 기초 · 예제 다운로드

## 구성과 실행

압축을 새 빈 폴더에 풀면 `README.md`와 `project/`가 있습니다. C11을 지원하는 GCC 또는 Clang이 필요합니다. 아래는 Linux/macOS의 POSIX 셸 명령입니다. `cc`가 다른 컴파일러라면 `gcc` 또는 `clang`으로 바꾸세요.

```sh
cd project
cc -std=c11 -O0 -Wall -Wextra -Wpedantic -Werror main.c -o lesson
./lesson
```

표준입력이나 외부 데이터 파일은 필요 없습니다. 코드 안의 고정 입력과 경계 검사가 실행됩니다. `-DNDEBUG`를 추가하지 마세요. assert가 있는 예제의 검사 의미가 달라집니다.

## 정상 실행의 기대 출력

```text
level=21 count=3 total=24 half=12 last=12
```

정상 종료 코드는 0입니다. 실패 진단 또는 비정상 종료가 나오면 성공 출력으로 취급하지 마세요.

기존 CPU 실행·해시 원장: https://jiwhan112.github.io/um.github.io/knowledge/shared/c-basics-06-08/verification.json


## 원본과 검증 경계

이 팩은 기존 공개 교재의 실제 전체 소스만 묶습니다. 기존 코드 바이트는 바꾸지 않았습니다. ZIP 루트의 이 README만 다운로드용 안내입니다. 외부 페이지·원본 첨부·검증 로그를 실행 의존성으로 넣지 않았습니다.

패키징 단계에서는 CPU 검사를 재실행하지 않았습니다. 기존 실행 기록과 소스 해시 대조는 신규 빌드 성공이나 사용자의 플랫폼 실행을 뜻하지 않습니다. 압축 해제 뒤 위 명령을 직접 실행한 결과와 구분하세요. Windows/MSVC·GPU·성능에 대한 신규 검증은 없습니다.

교재: https://jiwhan112.github.io/um.github.io/knowledge/c-basics-07/
