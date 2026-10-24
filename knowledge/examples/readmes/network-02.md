# 네트워크 02 · 전송량·단위·Chunk 비용

## 예제 범위
기존 Chunk Cost 전체 소스의 두 코드 블록을 순서대로 LF로 연결하고 마지막 LF 1개를 추가해 복원했다. 실행 소스는 한국어 해설 주석만 추가한 파생본이다.
전체 소스와 명시된 CPU 실습만 제공합니다.

## 파일 지도
- README.md: 이 안내.
- project/main.cpp: 한국어 주석 전체 CPU 예제.
- project/CMakeLists.txt: 독립 빌드 설정.
- verification/results.json: 도구 버전·소스 해시·상대 재현 명령·실제 종료 코드·결과 요약.
- provenance.json: 코드 복원 상태·코드 해시.
- original/main.cpp, original/CMakeLists.txt: 순수 코드 복원본.
- verification/source-identity.json: 주석 전후 코드 무결성 검사.

## 빌드와 실행
CMake 3.16+, C++20 컴파일러, 빌드 도구가 필요합니다. GCC 14.2에서 확인했습니다. 압축을 푼 최상위에서:
```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure -V
```
기대 결과는 CTest 1/1 통과, 종료 코드 0, PASS 30 조건 호출입니다. 횟수는 독립 요구사항 수가 아닙니다. 외부 서버·키·네트워크 연결은 필요하지 않습니다.

## 검증 경계
실제 소켓·TCP/UDP 통신·TLS·패킷 캡처·RTT·Windows·sanitizer는 검증하지 않았습니다. 이 결과는 CPU 모델의 유한 입력 검사이며 과거 원본의 실행 기록을 대신하지 않습니다.
