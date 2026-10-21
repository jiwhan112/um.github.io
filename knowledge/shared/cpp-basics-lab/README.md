# C++ 기초 실습 팩 · 보존 원본과 한국어 주석판

이 폴더는 Notion의 `cpp_basics_lab_sources_v1.md` 첨부를 보존하고, 첫 세 단원을 읽기 쉽게 만든 한국어 주석판을 함께 제공합니다. **전체 팩은 C++20 기준**입니다. C++11 호환판은 작성하지 않았습니다.

## 파일과 학습 순서

| 단원·역할 | 보존 원본 | 한국어 주석판 | 정상 실행의 CHECK 호출 횟수 |
|---|---|---|---:|
| 01 빌드·타입·초기화·표현식 | [types.cpp](original/src/types.cpp) | [types.cpp](ko/types.cpp) | 15 |
| 02 제어 흐름·함수·콜백 | [flow.cpp](original/src/flow.cpp) | [flow.cpp](ko/flow.cpp) | 15 |
| 03 포인터·참조·수명·소유권 | [memory.cpp](original/src/memory.cpp) | [memory.cpp](ko/memory.cpp) | 19 |
| 이후 단원의 배열·문자열·표준 도구 | [containers.cpp](original/src/containers.cpp) | 이번 배치 없음 | 49 |
| 이후 단원의 int 버퍼 | [buffer.cpp](original/src/buffer.cpp) | 이번 배치 없음 | 104 |
| 이후 단원의 분할 빌드·파일 | [inventory.hpp](original/src/inventory.hpp), [inventory.cpp](original/src/inventory.cpp), [inventory_main.cpp](original/src/inventory_main.cpp) | 이번 배치 없음 | 19 |

원본 여섯 프로그램은 **총 221회**의 런타임 조건 검사를 합니다. 이 수에는 루프가 원소마다 반복하는 검사도 포함됩니다. 이번 본문 01~03의 검사는 그중 **49회(15 + 15 + 19)**이며, 한국어 주석판과 다른 컴파일러는 같은 조건을 다시 실행합니다. 221회와 49회를 더하거나, 반복 실행을 새 독립 조건으로 합산하지 않습니다. `types.cpp`의 `static_assert` 네 개는 컴파일 시 검사이므로 런타임 15회에 포함하지 않습니다.

원본과 주석판은 모두 [original/src/check.hpp](original/src/check.hpp)에 의존합니다. `ko/*.cpp` 하나만 내려받아서는 공통 헤더가 부족합니다. 전체 저장소의 이 폴더를 사용하거나, 원본 헤더를 함께 받고 include 경로를 지정하세요.

## 원본 보존과 변경 범위

- [source-attachment.md](source-attachment.md): Notion에서 실제로 내려받은 전체 Markdown **19,602바이트**. SHA-256은 `c48d34d0e978b43f3e1b81d86e1e902052e6c5f8536c6b372c2fae8a4d2ed761`입니다.
- [original/](original/): 첨부의 CMake·헤더·C++ **16개 파일 블록**을 마지막 개행까지 그대로 분리했습니다. 정상 프로그램 여섯 개, 컴파일 거부 예제 네 개, 위험 진단 예제 두 개를 포함합니다.
- [source-integrity.json](source-integrity.json): 첨부·원본·한국어 주석판의 파일별 크기와 SHA-256, 원문 페이지 capture 검증, 토큰 비교 결과를 보존합니다.
- [verify_sources.py](verify_sources.py): 첨부 고정 해시, 16개 블록 바이트, 01~03 및 상위 목차 원문의 capture 해시, 한국어 주석판의 전처리 전 토큰을 확인합니다.

한국어 주석판은 설명·들여쓰기·줄바꿈을 바꾸고 코드 토큰은 유지했습니다. `types` **360개**, `flow` **471개**, `memory` **549개** 토큰이 각각 원본과 같습니다. 예외·출력 문자열도 원형을 유지합니다. `CHECK`는 원본 헤더의 같은 매크로를 사용하며, `__LINE__`은 각 파일의 실제 줄을 가리키므로 주석판의 실패 진단 줄번호는 달라집니다. 토큰 검사는 실제 컴파일·실행과 구분합니다.

첨부에는 2026-10-12의 GCC 14.2.0·Clang 17.0.0·CMake 3.31.6 실행 성공 **서술**이 있습니다. 당시의 ZIP과 원시 Configure·Build·CTest 로그는 이번에 회수하지 못했습니다. 첨부 자체가 ZIP의 Notion 업로드 실패와 과거 대화 첨부를 구분하므로, Markdown 확보를 ZIP·과거 로그 확보로 표시하지 않습니다. 새 실행 로그는 아래 `verification/`에 별도로 보존합니다.

## C++20 정상 빌드

저장소 루트에서 실행합니다. CMake 3.20 이상과 C++20 컴파일러가 필요합니다.

```sh
python3 knowledge/shared/cpp-basics-lab/verify_sources.py
cmake -S knowledge/shared/cpp-basics-lab -B build/cpp-basics -DCMAKE_BUILD_TYPE=Debug
cmake --build build/cpp-basics --verbose
ctest --test-dir build/cpp-basics --output-on-failure -V
```

루트 [CMakeLists.txt](CMakeLists.txt)는 보존한 [original/CMakeLists.txt](original/CMakeLists.txt)를 하위 프로젝트로 그대로 사용합니다. 원본 테스트 `types`, `flow`, `memory`, `containers`, `buffer`, `inventory`와 한국어판 `types_ko`, `flow_ko`, `memory_ko`를 등록합니다. 정상 CTest에 등록된 실행 파일은 **9개**이며, 컴파일 거부·위험 진단 예제는 포함하지 않습니다.

첫 단원만 실행하려면 다음처럼 대상 이름으로 고릅니다. 02는 `flow`, 03은 `memory`로 바꾸면 됩니다.

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
| `positive-direct` | 원본 6개 + 한국어판 3개 | C++20 직접 컴파일·실행 성공, 예상 CHECK 횟수, 첫 3개 stdout 일치 |
| `compare` | CMake로 빌드한 첫 3개 원본·주석판 | 두 실행이 각각 정상 종료하고 stdout이 같음 |
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

## 2026-10-21 새 실행 기록

로컬 Linux, **GCC 13.3.0**에서 다음을 실제 실행했습니다. 원시 출력과 명령·종료 코드 JSON은 [local-gcc-20261021](verification/local-gcc-20261021/)에 있습니다.

- C++20 직접 빌드: 원본 여섯 개 **15·15·19·49·104·19회 PASS**, 한국어판 세 개 **15·15·19회 PASS**. `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`를 통과했습니다. 첫 세 원본·주석판의 stdout이 각각 같았습니다.
- 컴파일 거부 네 개: 모두 컴파일러 종료 코드 **1**, 예상 거부를 확인했습니다.
- C++11 엄격 모드: 원본 여섯 개 모두 종료 코드 **1**로 거부했습니다. **C++11 미지원**이며 실행 성공 기록이 아닙니다.
- 원본 첨부·16개 파일, 한국어 토큰, 원문 4개 capture 검사: 통과했습니다.

로컬에는 Clang·CMake·CTest가 없어 이 환경에서 실행하지 않았습니다. [.github/workflows/knowledge-cpp-basics.yml](../../../.github/workflows/knowledge-cpp-basics.yml)에 GCC C++20·Clang C++20 ASan/UBSan 정상 CMake/CTest, 별도 컴파일 거부, 별도 위험 진단 작업을 구성했습니다. **워크플로 작성은 실행 성공과 다릅니다.** 실제 GitHub run과 artifact를 확인한 결과는 [Actions 실행 목록](https://github.com/jiwhan112/um.github.io/actions/workflows/knowledge-cpp-basics.yml) 및 이후 별도 검증 원장으로 구분합니다.

## 적용 한계

`Add`·`Scale`·`Sum`은 결과가 표현 가능한 작은 입력을 전제로 하며 범용 안전 산술이 아닙니다. int 전용 `IntBuffer`는 복사·이동을 금지하고 자체 2배 성장 정책을 사용합니다. `Inventory`는 ASCII 이름으로 시험했으며 UTF-8 검증, 읽기 전 총 바이트 제한, 원자적 저장, 전원 장애 복구를 구현하지 않았습니다. Windows/MSVC·게임 통합은 실행 미검증입니다.

첨부의 공식 참고 자료: [C++20 공개 초안 N4861](https://timsong-cpp.github.io/cppwp/n4861/), [Microsoft 표준 선택](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170), [Clang ASan](https://clang.llvm.org/docs/AddressSanitizer.html), [Clang UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).
