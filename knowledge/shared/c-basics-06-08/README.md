# C 기초 06–08 · 독립 C11 실습 검증

2026-10-22 Linux x86-64/GCC 14.2에서 C11 전체 소스 세 개를 엄격 경고 빌드와 UBSan+bounds로 각각 한 번 실행했다. 단계별 실제 출력·SHA-256·종료 코드는 [verification.json](verification.json)에서 확인한다. 커밋 대상 main.c는 해당 검사 당시 소스와 Git blob SHA가 일치한다. 과거 C++ 공유 소스는 다시 검사하지 않았다.

재현: 저장소 루트에서 `sh knowledge/shared/c-basics-06-08/verify.sh`. gcc, sha256sum, sh가 필요하다. assert 경로를 검사하므로 NDEBUG를 정의하지 않는다. 정상 종료는 제공한 경로에 한정되며 모든 입력과 포인터가 안전하다는 증명은 아니다. 새 문서의 웹/모바일/공개 Pages 검증은 master 게시 게이트에서 별도로 수행한다.
