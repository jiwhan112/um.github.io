# 네트워크 05 · MTU·IPv4/IPv6 헤더 예산 · CPU 실습

## 파일
- `main.cpp`: 복원한 원본 코드에 한국어 설명 주석만 추가.
- `edge_cases.cpp`: 현재 작성한 별도 8회 경계 검사.
- `CMakeLists.txt`: 원본 빌드에 추가 검사 타깃·10초 CTest 제한을 더한 파생 빌드.

## 실행
CMake 3.16 이상과 C++11/20 컴파일러가 필요합니다. 아래 명령은 압축을 푼 `project` 폴더에서 실행합니다.

```sh
cmake -S . -B build -DLAB_CXX_STANDARD=20
cmake --build build
ctest --test-dir build -V --output-on-failure --timeout 10
```

C++11은 다른 build 폴더와 `-DLAB_CXX_STANDARD=11`을 사용하세요. CMake의 기본값은 20입니다. 현재 Linux GCC 14.2, CMake 3.31.6에서 원본과 한국어판을 C++11/20으로 확인했습니다. 원본 30회, 추가 8회 검사를 환경마다 반복했으며 고유 조건을 반복 합산하지 않습니다.

## 경계
실제 소켓, OS 라우팅 테이블, PMTU 탐색, Windows/MSVC, GPU, Clang·sanitizer는 이번 실행에 포함되지 않습니다. 원본의 과거 Clang 로그는 역사 자료입니다.

유효한 enum만 입력하세요. MTU 상한·옵션·확장헤더·게임헤더를 원본 함수가 모두 검증하지 않습니다. units는 실제 TCP 패킷 수가 아닙니다. MTU65536 추가 검사는 모델 한계 관찰이지 실제 IPv4 허용값이 아닙니다.

## 증거
- 원본/주석/빌드 입력 해시와 현재 실행: `/knowledge/network-05/verification/validation.json`
- 공개 원본·파생 코드 무결성: `/knowledge/network-05/source-manifest.json`
- 교재: `/knowledge/network-05/`

현재 파생 교재와 CPU 실행일은 2026-10-24입니다. verification 요약의 재현 명령은 원시 로그가 아닙니다.
