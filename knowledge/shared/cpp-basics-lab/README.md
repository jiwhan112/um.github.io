# C++ 기초 실습 팩 · 보존 원본과 한국어 주석판

이 폴더는 Notion의 `cpp_basics_lab_sources_v1.md` 첨부를 보존하고, 기초 01~05를 읽기 쉽게 만든 한국어 주석판을 함께 제공합니다. **전체 팩은 C++20 기준**입니다. C++11 호환판은 작성하지 않았습니다.

## 파일과 학습 순서

| 단원·역할 | 보존 원본 | 한국어 주석판 | 정상 실행의 CHECK 호출 횟수 |
|---|---|---|---:|
| 01 빌드·타입·초기화·표현식 | [types.cpp](original/src/types.cpp) | [types.cpp](ko/types.cpp) | 15 |
| 02 제어 흐름·함수·콜백 | [flow.cpp](original/src/flow.cpp) | [flow.cpp](ko/flow.cpp) | 15 |
| 03 포인터·참조·수명·소유권 | [memory.cpp](original/src/memory.cpp) | [memory.cpp](ko/memory.cpp) | 19 |
| 04 배열·문자열·표준 도구 | [containers.cpp](original/src/containers.cpp) | [containers.cpp](ko/containers.cpp) | 49 |
| 04 int 버퍼 | [buffer.cpp](original/src/buffer.cpp) | [buffer.cpp](ko/buffer.cpp) | 104 |
| 05 분할 빌드·파일 | [inventory.hpp](original/src/inventory.hpp), [inventory.cpp](original/src/inventory.cpp), [inventory_main.cpp](original/src/inventory_main.cpp) | [inventory.hpp](ko/inventory.hpp), [inventory.cpp](ko/inventory.cpp), [inventory_main.cpp](ko/inventory_main.cpp) | 19 |

원본 여섯 프로그램은 **총 221회**의 런타임 조건 검사를 합니다. 이 수에는 루프가 원소마다 반복하는 검사도 포함됩니다. 본문 01~03의 검사는 그중 **49회(15 + 15 + 19)**, 04~05는 **172회(49 + 104 + 19)**입니다. 한국어 주석판과 다른 컴파일러는 같은 조건을 다시 실행합니다. 팩 전체 221회와 단원별 부분합을 더하거나, 반복 실행을 새 독립 조건으로 합산하지 않습니다. 정적 CHECK 호출 지점은 `containers` 23개, `buffer` 5개이며 루프 실행 횟수와 구분합니다. `types.cpp`의 `static_assert` 네 개는 컴파일 시 검사이므로 런타임 15회에 포함하지 않습니다.

원본과 주석판은 모두 [original/src/check.hpp](original/src/check.hpp)에 의존합니다. `ko/*.cpp` 하나만 내려받아서는 공통 헤더가 부족합니다. 전체 저장소의 이 폴더를 사용하거나, 원본 헤더를 함께 받고 include 경로를 지정하세요.

## 원본 보존과 변경 범위

- [source-attachment.md](source-attachment.md): Notion에서 실제로 내려받은 전체 Markdown **19,602바이트**. SHA-256은 `c48d34d0e978b43f3e1b81d86e1e902052e6c5f8536c6b372c2fae8a4d2ed761`입니다.
- [original/](original/): 첨부의 CMake·헤더·C++ **16개 파일 블록**을 마지막 개행까지 그대로 분리했습니다. 정상 프로그램 여섯 개, 컴파일 거부 예제 네 개, 위험 진단 예제 두 개를 포함합니다.
- [source-integrity.json](source-integrity.json): 첨부·원본·한국어 주석판의 파일별 크기와 SHA-256, 원문 페이지 capture 검증, 토큰 비교 결과를 보존합니다.
- [verify_sources.py](verify_sources.py): 첨부 고정 해시, 16개 블록 바이트, 01~05 및 상위 목차 원문 6개의 capture 해시, 한국어 주석판의 전처리 전 토큰을 확인합니다.

한국어 주석판은 설명·들여쓰기·줄바꿈을 바꾸고 코드 토큰은 유지했습니다. 8개 파일의 토큰이 각각 원본과 같습니다. `types.cpp` 360개, `flow.cpp` 471개, `memory.cpp` 549개, `containers.cpp` 764개, `buffer.cpp` 434개, `inventory.hpp` 87개, `inventory.cpp` 565개, `inventory_main.cpp` 728개입니다. 예외·출력 문자열도 원형을 유지합니다. `CHECK`는 원본 헤더의 같은 매크로를 사용하며, `__LINE__`은 각 파일의 실제 줄을 가리키므로 주석판의 실패 진단 줄번호는 달라집니다. 토큰 검사는 실제 컴파일·실행과 구분합니다.

첨부에는 2026-10-12의 GCC 14.2.0·Clang 17.0.0·CMake 3.31.6 실행 성공 **서술**이 있습니다. 당시의 ZIP과 원시 Configure·Build·CTest 로그는 이번에 회수하지 못했습니다. 첨부 자체가 ZIP의 Notion 업로드 실패와 과거 대화 첨부를 구분하므로, Markdown 확보를 ZIP·과거 로그 확보로 표시하지 않습니다. 새 실행 로그는 아래 `verification/`에 별도로 보존합니다.

## C++20 정상 빌드

저장소 루트에서 실행합니다. CMake 3.20 이상과 C++20 컴파일러가 필요합니다.

```sh
python3 knowledge/shared/cpp-basics-lab/verify_sources.py
cmake -S knowledge/shared/cpp-basics-lab -B build/cpp-basics -DCMAKE_BUILD_TYPE=Debug
cmake --build build/cpp-basics --verbose
ctest --test-dir build/cpp-basics --output-on-failure -V
```

루트 [CMakeLists.txt](CMakeLists.txt)는 보존한 [original/CMakeLists.txt](original/CMakeLists.txt)를 하위 프로젝트로 그대로 사용합니다. 원본 테스트 `types`, `flow`, `memory`, `containers`, `buffer`, `inventory`와 한국어판 `types_ko`, `flow_ko`, `memory_ko`, `containers_ko`, `buffer_ko`, `inventory_ko`를 등록합니다. 정상 CTest에 등록된 실행 파일은 **12개**이며, 컴파일 거부·위험 진단 예제는 포함하지 않습니다.

첫 단원만 실행하려면 다음처럼 대상 이름으로 고릅니다. 02는 `flow`, 03은 `memory`, 04는 `containers` 또는 `buffer`, 05는 `inventory`로 바꾸면 됩니다.

```sh
cmake --build build/cpp-basics --target types types_ko
ctest --test-dir build/cpp-basics --output-on-failure -V -R '^types(_ko)?$'
```

CMake 없이 한국어판 하나를 직접 빌드할 수도 있습니다.

```sh
g++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror \
  -I knowledge/shared/cpp-basics-lab/original/src \
  knowledge/shared/cpp-basics-lab/ko/types.cpp -o /tmp/cpp-types-ko
/tmp/cpp-types-ko
```

Clang에서 정상 프로그램에 ASan/UBSan을 적용하는 CMake 선택지는 `-DCMAKE_CXX_COMPILER=clang++ -DCPP_BASICS_SANITIZERS=ON`입니다. 이는 아래의 의도적 오류 예제와 별개의 정상 프로그램 검사입니다.

## 정상 검사와 예상된 실패를 구분하기

[verify_experiments.py](verify_experiments.py)는 명령의 stdout·stderr를 합친 원시 출력과 실제 종료 코드를 파일로 남깁니다. 컴파일러의 오류 문구를 성공 출력으로 고쳐 쓰지 않습니다.

| 모드 | 대상 | 통과로 인정하는 관찰 |
|---|---|---|
| `positive-direct` | 원본 6개 + 한국어판 6개 | C++20 직접 컴파일·실행 성공, 예상 CHECK 횟수, 6쌍 stdout 바이트 일치 |
| `compare` | CMake로 빌드한 6개 원본·주석판 | 각 실행이 정상 종료하며 예상 CHECK 횟수와 6쌍 stdout 바이트 일치 |
| `negative` | const 쓰기·축소 변환·배열 포인터·임시 참조 4개 | `-std=c++20 -pedantic-errors -c`가 컴파일 오류로 거부 |
| `cxx11` | 보존 원본 6개 | `-std=c++11 -pedantic-errors -fsyntax-only`에서 예상한 미지원 진단. 실행하지 않음 |
| `diagnostics` | signed overflow·해제 후 접근 2개 | 의도적 미정의 동작을 sanitizer가 진단하고 비정상 종료. 정상 프로그램 PASS가 아님 |

예를 들어 다음 명령은 잘못된 코드가 거부되는지 확인합니다.

```sh
python3 knowledge/shared/cpp-basics-lab/verify_experiments.py negative --compiler g++ --out /tmp/cpp-negative-logs
python3 knowledge/shared/cpp-basics-lab/verify_experiments.py cxx11 --compiler g++ --out /tmp/cpp-cxx11-logs
```

`check.hpp`의 inline 변수, `types`의 `is_same_v`, `flow`의 `optional`, `memory`의 `make_unique`·`bit_cast`, `containers`의 `span`·`string_view`·`variant`, 파일 예제의 `filesystem` 등이 C++11 이후 기능입니다. 원본 CMake도 C++20을 고정합니다. 기본 포인터·반복문 같은 개념을 설명한다는 사실과 **전체 파일의 C++11 호환성**은 별개입니다. 이 보존 팩을 C++11로 빌드했다고 표시하지 않습니다.

위험 진단은 별도 CI 작업에서만 정상 검사와 구분해 실행합니다. `signed_overflow.cpp`는 UBSan의 `signed integer overflow`, `use_after_free.cpp`는 ASan의 `heap-use-after-free` 진단과 비정상 종료를 함께 요구합니다. 진단 프로그램은 학습용 의도적 오류이며 게임 코드에 복사할 정상 구현이 아닙니다.

## 2026-10-21 이전 01~03 배치의 실행 기록

로컬 Linux, **GCC 13.3.0**에서 다음을 실제 실행했습니다. 원시 출력과 명령·종료 코드 JSON은 [local-gcc-20261021](verification/local-gcc-20261021/)에 있습니다.

- C++20 직접 빌드: 원본 여섯 개 **15·15·19·49·104·19회 PASS**, 한국어판 세 개 **15·15·19회 PASS**. `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`를 통과했습니다. 첫 세 원본·주석판의 stdout이 각각 같았습니다.
- 컴파일 거부 네 개: 모두 컴파일러 종료 코드 **1**, 예상 거부를 확인했습니다.
- C++11 엄격 모드: 원본 여섯 개 모두 종료 코드 **1**로 거부했습니다. **C++11 미지원**이며 실행 성공 기록이 아닙니다.
- 원본 첨부·16개 파일, 한국어 토큰, 원문 4개 capture 검사: 통과했습니다.

로컬에는 Clang·CMake·CTest가 없어 이 환경에서 실행하지 않았습니다. 이후 GitHub Actions의 [실행 39532381043](https://github.com/jiwhan112/um.github.io/actions/runs/39532381043)에서 커밋 `d6065fee0468d0f0447b826a608363632f36bc0e`를 실제 검증했고, 다섯 작업이 모두 성공했습니다. 실행 환경은 Ubuntu 24.04, **GCC 13.3.0 / Clang 18.1.3 / CMake 3.31.6**입니다.

| GitHub CI 작업 | 실제 결과 | 보존한 원시 기록 |
|---|---|---|
| GCC C++20 | Configure·Build 성공, 정상 CTest **9/9**, 첫 세 원본·한국어판 stdout 일치 | [Configure](verification/github-39532381043/gcc-cxx20/configure.txt) · [Build](verification/github-39532381043/gcc-cxx20/build.txt) · [CTest](verification/github-39532381043/gcc-cxx20/ctest.txt) |
| Clang C++20 + ASan/UBSan | Configure·Build 성공, 정상 CTest **9/9**, 첫 세 원본·한국어판 stdout 일치 | [Configure](verification/github-39532381043/clang-cxx20-asan-ubsan/configure.txt) · [Build](verification/github-39532381043/clang-cxx20-asan-ubsan/build.txt) · [CTest](verification/github-39532381043/clang-cxx20-asan-ubsan/ctest.txt) |
| GCC·Clang 부정 예제 | 같은 네 소스를 각 컴파일러가 모두 종료 코드 1로 거부 | [GCC 결과](verification/github-39532381043/gcc-rejections/negative/results.json) · [Clang 결과](verification/github-39532381043/clang-rejections/negative/results.json) |
| GCC C++11 엄격 모드 | 원본 여섯 프로그램 모두 종료 코드 1로 거부 · **C++11 미지원** | [명령·종료 코드](verification/github-39532381043/gcc-rejections/cxx11-unsupported/results.json) |
| Clang 의도적 오류 실험 | signed overflow·해제 후 접근의 예상 진단과 종료 코드 1 확인 | [별도 진단 결과](verification/github-39532381043/clang-intentional-diagnostics/intentional-diagnostics/results.json) |

정상 CTest 9개는 원본 여섯 프로그램과 한국어 주석판 세 프로그램입니다. 다른 환경에서 같은 검사를 다시 통과한 결과를 독립 조건으로 더하지 않습니다. 의도적 오류 실험은 정상 프로그램의 성공에 포함하지 않습니다.

GitHub artifact 다섯 개의 ZIP SHA-256을 확인한 뒤 **원시 파일 55개를 바이트 그대로 보관**했습니다. artifact ID·원본 커밋·파일별 크기와 해시는 [artifact-manifest.json](verification/github-39532381043/artifact-manifest.json), 실행 옵션과 기록을 읽는 순서는 [이 실행의 검증 원장](verification/github-39532381043/README.md)에 있습니다. [.github/workflows/knowledge-cpp-basics.yml](../../../.github/workflows/knowledge-cpp-basics.yml)이 이후 변경에도 같은 검사를 수행합니다.

## 2026-10-21 · 04~05 한국어 주석판 확장 검증

같은 Notion 파일 블록을 다시 다운로드하여 현재 첨부 **19,602바이트**, SHA-256 `c48d34d0e978b43f3e1b81d86e1e902052e6c5f8536c6b372c2fae8a4d2ed761`이 보존본과 바이트 단위로 같음을 확인했습니다. 안정적인 원본 참조는 다음과 같습니다. 만료되는 서명 다운로드 URL은 저장하지 않습니다.

```text
notion-file-block://5fb8e4e7-6ccb-4edd-889f-bba260b4392b/2d74b3e5-ff5f-4dd0-9fff-4d3559528daf?space_id=815d1335-3b56-43a7-903b-d7addf097c9f&name=cpp_basics_lab_sources_v1.md
```

원본 16개 파일은 변경하지 않았습니다. 04~05용 한국어 주석 파일 5개를 추가하여 총 8개 소스·헤더, 6개 정상 프로그램의 원본 토큰을 대조합니다. `inventory`는 구현과 main 두 cpp를 함께 연결하며, 한국어판은 같은 디렉터리의 한국어 주석 헤더를 사용합니다. 공통 `check.hpp`는 계속 보존 원본을 사용합니다.

로컬 **GCC 13.3.0 / Linux**에서 현재 확장 코드를 실제 실행했습니다. [이번 로컬 원시 기록](verification/local-gcc-04-05-20261021/README.md)에는 명령·종료 코드와 원시 stdout/stderr가 있습니다.

- C++20 직접 빌드·실행 **원본 6개 + 한국어판 6개 모두 성공**. 프로그램별 런타임 CHECK는 각각 15·15·19·49·104·19회입니다. 여섯 원본·주석판의 stdout이 바이트 단위로 같고 출력 정규화는 적용하지 않았습니다.
- 컴파일 거부 예제 네 개는 모두 예상대로 종료 코드 1로 거부했습니다.
- 엄격한 C++11 구문 검사는 원본 여섯 개를 모두 종료 코드 1로 거부했습니다. C++11 실행 성공으로 집계하지 않습니다.
- 01~05 및 상위 목차 원문 6개 capture 해시, 전체 첨부와 원본 16개 블록, 한국어 주석판 8개 파일의 토큰·전처리 지시문 동일성 검사를 통과했습니다.

`compare`는 모든 실행의 작업 디렉터리를 이 팩의 루트로 고정하여 `inventory`의 `cwd=` 출력도 수정 없이 비교합니다. 같은 입력의 반복 실행과 다른 컴파일러의 재실행은 독립 조건을 늘리지 않습니다.

현재 로컬에는 Clang·CMake·CTest가 없습니다. **04~05 한국어 주석판을 포함하는 12개 CTest의 새 GitHub 실행은 별도로 확인해야 합니다.** 위 9/9 기록은 이전 01~03 배치의 성공이며 새 12개 구성을 실행한 증거로 사용하지 않습니다.

## 적용 한계

`Add`·`Scale`·`Sum`은 결과가 표현 가능한 작은 입력을 전제로 하며 범용 안전 산술이 아닙니다. int 전용 `IntBuffer`는 복사·이동을 금지하고 자체 2배 성장 정책을 사용합니다. `Inventory`는 ASCII 이름으로 시험했으며 UTF-8 검증, 읽기 전 총 바이트 제한, 원자적 저장, 전원 장애 복구를 구현하지 않았습니다. Windows/MSVC·게임 통합은 실행 미검증입니다.

첨부의 공식 참고 자료: [C++20 공개 초안 N4861](https://timsong-cpp.github.io/cppwp/n4861/), [Microsoft 표준 선택](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170), [Clang ASan](https://clang.llvm.org/docs/AddressSanitizer.html), [Clang UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).
