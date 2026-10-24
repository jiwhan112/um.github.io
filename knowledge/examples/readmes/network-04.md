# 네트워크 04 · IPv4 CIDR·최장 접두부 경로 선택 · CPU 실습

## 파일
- `main.cpp`: 복원한 원본 코드에 한국어 설명 주석만 추가.
- `edge_cases.cpp`: 현재 작성한 별도 8회 경계 검사.
- `CMakeLists.txt`: 원본 빌드에 추가 검사 타깃·10초 CTest 제한을 더한 파생 빌드.

## 실행
CMake 3.16 이상과 C++11/20 컴파일러가 필요합니다. 아래 명령은 압축을 푼 `project` 폴더에서 실행합니다.

```sh
cmake -S . -B build -DCMAKE_CXX_STANDARD=20
cmake --build build
ctest --test-dir build -V --output-on-failure --timeout 10
```

C++11은 다른 build 폴더와 `-DCMAKE_CXX_STANDARD=11`을 사용하세요. CMake의 기본값은 20입니다. 현재 Linux GCC 14.2, CMake 3.31.6에서 원본과 한국어판을 C++11/20으로 확인했습니다. 원본 18회, 추가 8회 검사를 환경마다 반복했으며 고유 조건을 반복 합산하지 않습니다.

## 경계
실제 소켓, OS 라우팅 테이블, PMTU 탐색, Windows/MSVC, GPU, Clang·sanitizer는 이번 실행에 포함되지 않습니다. 원본의 과거 Clang 로그는 역사 자료입니다.

Lookup 포인터는 내부 vector를 빌립니다. Add 재할당/테이블 파괴 뒤 사용하지 마세요. 동일 정규화 접두부 중복은 거절하며 ECMP/정책 라우팅을 구현하지 않습니다.

## 증거
- 원본/주석/빌드 입력 해시와 현재 실행: `/knowledge/network-04/verification/validation.json`
- 공개 원본·파생 코드 무결성: `/knowledge/network-04/source-manifest.json`
- 교재: `/knowledge/network-04/`

현재 파생 교재와 CPU 실행일은 2026-10-24입니다. verification 요약의 재현 명령은 원시 로그가 아닙니다.
