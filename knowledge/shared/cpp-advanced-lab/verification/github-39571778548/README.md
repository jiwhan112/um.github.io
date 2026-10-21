# C++ 심화 05–06 · GitHub Actions 원시 검증 기록

[실행 39571778548](https://github.com/jiwhan112/um.github.io/actions/runs/39571778548)은 2026-10-21에 소스 커밋 [`f4ad911`](https://github.com/jiwhan112/um.github.io/commit/f4ad911d6407d0b9740ad8c75dd40ef45b6929dc)을 실제 빌드·실행한 기록이다. GitHub API의 실행 상태와 네 작업의 결론은 모두 `completed / success`다. 이 안내와 원시 로그 보관은 실행 뒤 추가한 증거이며, 로그를 보관하는 후속 커밋을 검증 대상 소스 커밋으로 바꾸어 인용하지 않는다.

## 실제 결과

| 환경 | Configure·Build·CTest | 추가 확인 |
| --- | --- | --- |
| Ubuntu 24.04 · GCC 13.3.0 · C++20 · CMake 3.31.6 | Configure·Build 성공, **CTest 18/18** | 원본·한국어판 출력 **9쌍** 바이트 동일, unsigned 값 비트 수 32 |
| Ubuntu 24.04 · Clang 18.1.3 · C++20 · ASan/UBSan · CMake 3.31.6 | Configure·Build 성공, **CTest 18/18** | 출력 **9쌍** 바이트 동일, 실행 경로의 sanitizer 진단 없음, unsigned 값 비트 수 32 |
| GCC / Clang C++20 컴파일 거부 예제 | 각 컴파일러에서 **6/6 예상 거부** | 컴파일만 수행, 링크·실행하지 않음 |
| GCC 엄격 C++11 조사 | 원본 **9/9 컴파일 거부** | C++11 미지원·미실행. 검사기의 pass는 예상 미지원 확인을 뜻함 |

실제 버전 문자열은 GCC `13.3.0-6ubuntu2~24.04.1`, Clang `18.1.3 (1ubuntu1)`이다. 각 구성은 원본 9개와 한국어 주석판 9개를 실행한다. 원본과 주석판 Pimpl은 각각 `pimpl.cpp`와 `pimpl_main.cpp`를 컴파일·링크하므로, 빌드에는 **번역 단위 20개와 실행 파일 링크 18개**가 기록되어 있다.

Clang의 **20개 컴파일 명령 모두**에 `-fsanitize=address,undefined -fno-omit-frame-pointer`, **18개 링크 명령 모두**에 `-fsanitize=address,undefined`가 포함된다. 원본과 주석판 모두 적용되며 GCC 구성에는 sanitizer를 사용하지 않았다. 모든 정상 소스는 `-std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`로 컴파일했다.

## 검사 호출 수와 반복 실행

| 단원 범위 | 원본 프로그램의 검사 호출 |
| --- | ---: |
| 01–02 · lifetime + ownership + transaction | 31 = 10 + 13 + 8 |
| 03–04 · polymorphism + templates + interfaces | 29 = 12 + 10 + 7 |
| 이번 05–06 · containers + allocation + pimpl | **29 = 16 + 7 + 6** |
| 전체 원본 9개 프로그램 | **89** |

`containers`에는 Expect 호출 위치가 14개 있으며 비교자 비반사성 루프의 한 위치가 세 원소에 대해 실행되어 16회가 된다. 이번 `pimpl_main`의 `static_assert` 1개는 런타임 6회와 별도다. 기존 `ownership`의 13회에는 prvalue 구성 예제 뒤의 `Expect(true)` 1회가 포함되며 해당 예제의 핵심 근거는 컴파일 성공이다. `ownership`의 `static_assert` 2개, `templates` 3개, `interfaces` 1개도 런타임 호출에 더하지 않는다.

한국어판의 같은 89회, 다른 컴파일러에서의 재실행, 출력 비교를 위한 재실행은 독립 논리 조건으로 더하지 않는다. 출력 비교는 9개 프로그램의 원본·한국어판을 같은 작업 경로에서 실행하여 **정규화 없이 원시 바이트가 같은지** 확인했다. 비교용 18회 실행은 모두 종료 코드 0이다.

unsigned 폭 조회는 원본을 변경하지 않는 별도 환경 관측이다. 두 컴파일러에서 `std::numeric_limits<unsigned>::digits`가 32임을 출력했으며 CTest 18개나 논리 검사 호출 수에 포함하지 않는다.

## 원시 출력과 실제 거부 원인

| 구성 | 환경·무결성 | Configure | Build | CTest | 출력 비교 |
| --- | --- | --- | --- | --- | --- |
| GCC C++20 | [환경](gcc-cxx20/environment-and-integrity.txt) | [출력](gcc-cxx20/configure.txt) | [출력](gcc-cxx20/build.txt) | [출력](gcc-cxx20/ctest.txt) | [결과 JSON](gcc-cxx20/output-equivalence/results.json) |
| Clang ASan/UBSan | [환경](clang-cxx20-asan-ubsan/environment-and-integrity.txt) | [출력](clang-cxx20-asan-ubsan/configure.txt) | [출력](clang-cxx20-asan-ubsan/build.txt) | [출력](clang-cxx20-asan-ubsan/ctest.txt) | [결과 JSON](clang-cxx20-asan-ubsan/output-equivalence/results.json) |

컴파일 거부 예제는 [GCC 결과](compile-rejections-gcc/negative/results.json)와 [Clang 결과](compile-rejections-clang/negative/results.json)에 기록했다. 각 결과와 같은 폴더에 원시 진단이 있다. 자동 판정은 종료 코드 1과 오류 진단 존재를 확인하며, 별도로 원시 진단을 읽어 아래의 의도한 원인을 대조했다.

| 예제 | 두 컴파일러의 실제 진단 원인 |
| --- | --- |
| constraint | double이 integral 제약을 만족하지 않음 |
| copy_by_value | 복사 생성자의 첫 인자를 값으로 선언 |
| copy_unique | 삭제된 unique_ptr 복사 생성자 호출 |
| list_sort | list 반복자에 std::sort가 요구하는 임의 접근 연산이 없음 |
| temporary_view | 삭제된 rvalue View 호출 |
| wrong_override | const 한정이 달라 override하지 못함 |

[GCC C++11 결과](compile-rejections-gcc/cxx11-unsupported/results.json)는 `-std=c++11 -pedantic-errors -fsyntax-only`로 원본 프로그램 9개를 조사한 기록이다. `std::invoke`, `make_unique`, inline 변수, concept/requires, `erase_if`, `string_view` 등 C++11 이후 기능의 진단이 실제로 남아 있다. Pimpl 조사도 구현과 main 두 CPP를 포함한다. **C++11 프로그램을 실행하거나 호환 버전으로 고친 결과가 아니다.** Clang 작업에서는 이 GCC 전용 C++11 단계를 의도적으로 실행하지 않았다.

## artifact 보존과 무결성

GitHub API digest, 다운로드한 ZIP 바이트의 SHA-256, 추출된 파일 바이트를 대조했다. 독립 재다운로드에서도 ZIP 4개의 digest와 보존된 **85파일 / 330,326바이트**가 모두 일치했다. [artifact-manifest.json](artifact-manifest.json)에 artifact ID·이름·ZIP 해시·파일별 바이트 수와 SHA-256을 보존했다.

| artifact ID | 보존 폴더 | 원시 파일 수 | ZIP 바이트 |
| --- | --- | ---: | ---: |
| 12332671542 | compile-rejections-gcc | 20 | 22,808 |
| 12332507901 | gcc-cxx20 | 28 | 17,627 |
| 12332292306 | compile-rejections-clang | 9 | 11,298 |
| 12332088251 | clang-cxx20-asan-ubsan | 28 | 17,503 |

85개에는 Configure·Build·CTest 출력, 환경·소스 무결성 출력, 실행 결과, 컴파일 진단, unsigned 폭 조회 소스와 출력이 포함된다. 이 README와 manifest 자체는 포함하지 않는다. 네 환경 기록의 소스 SHA와 포함된 무결성 목록도 서로 대조했다.

서명된 임시 다운로드 주소를 저장하지 않았다. 실행 당시의 runner 절대 경로·임시 경로·시각은 원시 기록에 남기고, 줄바꿈이나 출력 내용을 정규화하지 않았다. 이전 Notion 과거 로그와 01–02·03–04 GitHub 증거, 이번 로컬 GCC 기록은 별도 경로에서 변경 없이 보존한다.

## 해석 범위

이 결과는 해당 CPU 소스와 실행한 경로의 검증이다. Windows/MSVC·Direct3D·GPU·HLSL·동시 접근과 경쟁 조건·ABI·성능·모든 입력의 커버리지는 미검증이다. ASan/UBSan의 무진단도 실행하지 않은 경로의 안전성 증명이 아니다.

`Pool`의 재사용·범용 정렬, Pimpl 할당 실패 주입·자기 대입·이동 대입·빈 원본 복사, 모든 컨테이너 무효화 조합과 비교자의 전체 법칙을 추가로 실행한 결과로 넓히지 않는다. CPU CI 성공과 웹 UI 회귀 검사·PR 병합·실제 GitHub Pages 배포 완료는 각각 확인한다.
