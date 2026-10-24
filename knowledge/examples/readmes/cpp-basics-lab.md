# C++ 기초 공통 실습 · 예제 다운로드

## 구성

C++ 기초 01–05에서 공유하는 **하나의 C++20 실습 프로젝트**입니다. 설명 역할이 다른 페이지들이 같은 프로그램을 재사용합니다. C++11 호환판이 아닙니다.

- `project/CMakeLists.txt`: 기존 원본과 한국어 주석판을 함께 빌드하는 진입점
- `project/original/src/`: 보존 원본 구현·공통 헤더
- `project/original/CMakeLists.txt`: 보존 원본 타깃
- `project/ko/`: 한국어 주석판(전체 코드)
- `project/original/negative/`: 정상 빌드에 넣지 않는 의도적 컴파일 거부 예제

폴더 구조를 유지하세요. ko는 original/src/check.hpp도 사용합니다. 파일 하나만 복사하면 의존성이 빠질 수 있습니다.

## 빌드와 실행

CMake 3.20 이상과 C++20을 지원하는 GCC/Clang 또는 지원되는 MSVC 툴체인이 필요합니다. 아래는 압축을 푼 루트에서 실행합니다. CMake에 MSVC 분기가 있다는 사실은 이번 Windows 실행 검증을 뜻하지 않습니다.

```sh
cd project
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

정상 CTest 기대값은 **12/12 통과**입니다(원본과 ko 각각 6개). stdin·별도 데이터 입력은 필요 없습니다. CTest 결과는 독립 조건 개수와 다릅니다. Debug로 검사 실행 의미를 유지하고 `-DNDEBUG`를 임의로 넣지 마세요.

## 프로그램별 정상 출력

| 실행 파일 이름 | 예상 출력(원본·ko 동일) |
|---|---|
| `types` / `types_ko` | `PASS 15 checks` |
| `flow` / `flow_ko` | `PASS 15 checks` |
| `memory` / `memory_ko` | `PASS 19 checks` |
| `containers` / `containers_ko` | `PASS 49 checks` |
| `buffer` / `buffer_ko` | `PASS 104 checks` |
| `inventory` / `inventory_ko` | `PASS 19 checks` |

원본 프로그램은 `build/original/`, ko는 `build/`에 생성되는 단일 구성 생성기가 일반적입니다. 다중 구성 생성기는 `Debug/` 하위 경로가 더해질 수 있으므로 개별 실행 파일 위치를 가정하기보다 위 CTest 명령을 사용하세요.

`negative/`는 정상 CTest에 등록하지 않았습니다. 일부러 빌드되지 않아야 하는 예제이며 일반 소스 전체를 와일드카드로 묶어 빌드하지 마세요.

`original/diagnostics/` 두 파일은 signed overflow와 use-after-free를 진단하기 위한 의도적 미정의 동작입니다. 정상 프로그램이 아니며 실행하지 않아도 정상 실습을 완료할 수 있습니다. sanitizer 실험은 별도로 준비된 환경에서만 분리하세요.

과거 실행 근거: https://github.com/jiwhan112/um.github.io/actions/runs/39547021595 . 원본 저장소의 verify_sources.py/verify_experiments.py, 캡처, 중복 원본 ZIP, 역사 로그 전체는 이 독립 팩에 넣지 않았습니다. 해당 스크립트는 저장소 문맥을 필요로 하므로 이 팩에서 실행하라고 안내하지 않습니다.


## 원본과 검증 경계

이 팩은 기존 공개 교재의 실제 전체 소스만 묶습니다. 기존 코드 바이트는 바꾸지 않았습니다. ZIP 루트의 이 README만 다운로드용 안내입니다. 외부 페이지·원본 첨부·검증 로그를 실행 의존성으로 넣지 않았습니다.

2026-10-23 Cloud Linux에서 패키징 경로 그대로 복사한 독립 폴더를 검사했습니다: CMake configure/build 성공, CTest 12/12 통과. GCC 14.2.0을 사용했고 C++ 팩은 CMake 3.31.6 / Debug 구성입니다. ZIP 파일 자체의 압축 해제 검증과는 구별합니다. sanitizer·negative·diagnostics·Windows 검사는 이번에 다시 실행하지 않았습니다. 기존 실행 기록과 소스 해시 대조는 신규 빌드 성공이나 사용자의 플랫폼 실행을 뜻하지 않습니다. 압축 해제 뒤 위 명령을 직접 실행한 결과와 구분하세요. Windows/MSVC·GPU·성능에 대한 신규 검증은 없습니다.

교재: https://jiwhan112.github.io/um.github.io/knowledge/cpp-basics-01/, https://jiwhan112.github.io/um.github.io/knowledge/cpp-basics-02/, https://jiwhan112.github.io/um.github.io/knowledge/cpp-basics-03/, https://jiwhan112.github.io/um.github.io/knowledge/cpp-basics-04/, https://jiwhan112.github.io/um.github.io/knowledge/cpp-basics-05/
