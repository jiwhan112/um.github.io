# C 기초 04–05 · 독립 C11 새 교재 CPU 검증

이 폴더는 04(조건문·반복문)와 05(함수·범위·수명)에 **새로 작성한** C11 `main.c`의 검사 기록이다. 기존 Notion, C++ 프로그램과 다르며 반복 실행을 새로운 테스트 사례로 곱하지 않는다.

저장소 루트에서 `sh knowledge/shared/c-basics-04-05/verify.sh`을 실행한다. GCC와 POSIX 셸이 필요하다. `-Wall -Wextra -Wpedantic -Werror` 엄격 C11 빌드·실행 및 같은 소스의 `-fsanitize=undefined -fno-sanitize-recover=all` 빌드·실행으로 결과를 비교한다. 검사 프로그램에는 `NDEBUG`를 정의하지 않는다.

2026-10-22 실제 Linux x86_64 GCC 14.2 로컬 기록: [검증 세부 JSON](verification.json). C04 출력 `grade=pass total=15 visits=3 attempts=1 selected=6 status=found`; C05 출력 `original=120 clamped=100 sum5=15 tickets=1,2`. 두 프로그램의 strict 및 UBSan 실행 결과 모두 종료 코드 0이었다. 이 배치의 소스 SHA-256이 JSON에 기록되어 있다. 새로 추가된 이 프로그램들만 실행하며 과거 C01–03이나 공유 C++ 팩은 재실행하지 않는다.

UBSan 통과는 실행한 경로에서 검출된 문제가 없다는 뜻이지 모든 입력·플랫폼의 안전성을 증명하지 않는다. Windows/MSVC, 모든 아키텍처, 동시 실행 및 브라우저의 SVG·Mermaid 표현은 본 검증의 대상이 아니다. 새 웹 게시의 스키마·타입·PC/모바일 검사는 master의 React Pages CI에서 별도로 확인한다.
