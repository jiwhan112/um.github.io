# C++ 심화 · 보존 원본과 한국어 주석 실습

웹 교재 **심화 01–06**의 실행 가능한 소스 묶음입니다. 기존 01–04의 여섯 프로그램에 이어 이번 **05–06 배치**는 `containers`, `allocation`, `pimpl`을 한국어 주석판으로 추가했습니다. 원본 실습 팩 전체를 보존하며 원본 9개 프로그램 모두에 대응하는 주석판을 제공합니다. Pimpl은 헤더와 구현·진입점 두 번역 단위를 나누어 보존합니다.

현재 05–06 배치는 GCC 13.3.0 직접 빌드·실행과 [새 GitHub CI 39571778548](https://github.com/jiwhan112/um.github.io/actions/runs/39571778548)의 원시 로그 확인을 완료했습니다. **원본 9개 + 주석판 9개, 총 18개 CTest가 GCC와 Clang ASan·UBSan에서 각각 통과했으며 출력 9쌍이 바이트 단위로 일치합니다.** 이전 [GitHub CI 39565624710](https://github.com/jiwhan112/um.github.io/actions/runs/39565624710)의 CTest 15/15 및 01–02 배치의 12/12 성공은 각 과거 구성의 기록으로 구분합니다.

## 무엇을 보존했는가

| 경로 | 내용 | 원본과의 관계 |
| --- | --- | --- |
| [archive/cpp_advanced_effective_lab_v1.zip](archive/cpp_advanced_effective_lab_v1.zip) | 원본 ZIP, 32,140바이트 | Notion 검증 첨부에 기록된 SHA-256과 일치 |
| [archive/cpp_advanced_effective_sources_v1.md](archive/cpp_advanced_effective_sources_v1.md) | 전체 소스 Markdown, 34,754바이트 | 회수한 첨부를 그대로 보존 |
| [archive/validation-notion-attachment.md](archive/validation-notion-attachment.md) | Notion의 검증 첨부 | 본문·표·실패 예제·ZIP 해시를 그대로 보존 |
| [original/](original/) | ZIP의 소스·헤더·CMake·README·검증 요약·실행 스크립트 22파일 | ZIP 항목과 바이트 단위 일치 |
| [verification/historical-20261013/logs/](verification/historical-20261013/logs/) | ZIP의 과거 원시 로그 22파일 | ZIP의 `logs/` 항목을 바이트 변경 없이 분리 |
| [source-notion-index.md](source-notion-index.md) | C++ 심화 Notion 상위 목차의 전체 본문 | [capture](source-notion-index-capture.json)에 바이트 수·해시·확인 가능한 메타데이터 기록 |
| [ko/](ko/) | 한국어 해설 주석을 더한 헤더 2개·구현 및 진입점 10개, 프로그램 9개 | 주석·공백 외 C++ 토큰 및 전처리 지시문 동일 |
| [source-integrity.json](source-integrity.json) | 파일별 무결성 목록 | [verify_sources.py](verify_sources.py)로 재계산·대조 |

ZIP SHA-256: `a97a4927e5d3472ad055dbffbad0e14ab72c4c81f6ea13a1d72b771d64d339ba`.

전체 소스 Markdown SHA-256: `62a33518b9ba68cb689ad6b330a1327160f834ee8df9f052f11f34aef7986a4e`.

Notion 검증 첨부 SHA-256: `e9f9b5dbba6f631e9a95f4ac426647603eefcfe7d758d85a3541b41ee82bdeff`.

상위 원문: <https://app.notion.com/p/aaa55bced8ff459ba67fb036fb34bb3b>. 검증 첨부의 만료하지 않는 파일 블록 식별자는 `source-integrity.json`의 `provenance.notion_validation_attachment`에 보존했습니다. 서명된 임시 다운로드 주소는 게시하지 않습니다.

### Markdown의 줄 경계는 ZIP과 다르다

전체 소스 Markdown의 코드 블록은 21개이며, **각 블록 본문은 해당 ZIP 파일 뒤에 LF 한 개를 더한 바이트열**입니다. 이 차이를 지우거나 ZIP 파일에 줄을 덧붙이지 않았습니다. 실제 소스 파일의 기준은 Notion에 기록된 해시와 일치하는 ZIP이며, Markdown 첨부도 별도 원본으로 보존합니다. 두 바이트열의 개별 해시와 관계는 `markdown_zip_boundaries`에 기록했습니다. 검증 첨부의 컴파일 실패 예제 6개는 ZIP의 대응 소스와 바이트가 정확히 같습니다.

## 전체 코드와 단원 연결

| 웹 교재 | 프로그램 | 원본 파일 | 한국어 주석 파일 | 정상 실행의 검사 호출 |
| --- | --- | --- | --- | ---: |
| [심화 01](../../cpp-advanced-01/) | 생성·소멸 순서, 생성 실패, 불변식 | [lifetime.cpp](original/src/lifetime.cpp) | [lifetime.cpp](ko/lifetime.cpp) | 10 |
| [심화 02](../../cpp-advanced-02/) | 깊은 복사, 이동, 복사 실패, Rule of Zero | [ownership.cpp](original/src/ownership.cpp) | [ownership.cpp](ko/ownership.cpp) | 13 |
| [심화 02](../../cpp-advanced-02/) | 예외 안전성, RAII 인자, weak_ptr 관찰 | [transaction.cpp](original/src/transaction.cpp) | [transaction.cpp](ko/transaction.cpp) | 8 |
| [심화 03](../../cpp-advanced-03/) | 가상 호출, 생성·소멸, 기본 인자, RTTI, NVI, Clone | [polymorphism.cpp](original/src/polymorphism.cpp) | [polymorphism.cpp](ko/polymorphism.cpp) | 12 |
| [심화 04](../../cpp-advanced-04/) | concept, forwarding, 종속 이름, 변환 제약, constexpr | [templates.cpp](original/src/templates.cpp) | [templates.cpp](ko/templates.cpp) | 10 |
| [심화 04](../../cpp-advanced-04/) | explicit, 뷰 수명, iterator const, 비멤버 연산자 | [interfaces.cpp](original/src/interfaces.cpp) | [interfaces.cpp](ko/interfaces.cpp) | 7 |
| [심화 05](../../cpp-advanced-05/) | 컨테이너 선택, 무효화, 조회·삽입, 비교자 | [containers.cpp](original/src/containers.cpp) | [containers.cpp](ko/containers.cpp) | 16 |
| [심화 06](../../cpp-advanced-06/) | 생성 실패 정리, 저장소·객체 수명, PMR | [allocation.cpp](original/src/allocation.cpp) | [allocation.cpp](ko/allocation.cpp) | 7 |
| [심화 06](../../cpp-advanced-06/) | 불완전 타입 경계, Pimpl 복사·이동·swap | [pimpl.hpp](original/src/pimpl.hpp), [pimpl.cpp](original/src/pimpl.cpp), [pimpl_main.cpp](original/src/pimpl_main.cpp) | [pimpl.hpp](ko/pimpl.hpp), [pimpl.cpp](ko/pimpl.cpp), [pimpl_main.cpp](ko/pimpl_main.cpp) | 6 |
| 여섯 단원 공통 | 검사 도우미 | [check.hpp](original/src/check.hpp) | [check.hpp](ko/check.hpp) | 별도 실행 파일 없음 |

`ownership`의 13회에는 복사·이동이 삭제된 타입의 prvalue 반환 예제 뒤에 놓인 `Expect(true)` 1회가 포함됩니다. 그 예제의 핵심 근거는 프로그램이 컴파일되었다는 사실입니다. 검사 호출 수를 모두 서로 다른 비자명한 논리 조건으로 표현하지 않습니다. 같은 파일의 이동 특성 `static_assert` 2개도 런타임 13회와 분리합니다.

전체 원본의 호출 수는 `lifetime` 10 + `ownership` 13 + `polymorphism` 12 + `templates` 10 + `containers` 16 + `interfaces` 7 + `transaction` 8 + `allocation` 7 + `pimpl` 6 = **89회**입니다. 기존 01–02는 **31회**, 03–04는 **29회(12+10+7)**, 이번 05–06은 **29회(16+7+6)**이며 한국어 주석판에 대응하는 누적 범위는 **89회**입니다. 주석판에서 같은 89회를 다시 실행한 값, 다른 컴파일러에서의 재실행, 출력 비교 재실행은 독립 조건으로 더하지 않습니다.

`containers`의 `Expect` 호출 위치는 **14개**지만 비교자 비반사성 루프의 한 위치가 세 원소에 대해 실행되어 정상 호출 수는 **16회**입니다. `templates`의 `static_assert` 3개, `interfaces`의 1개, `ownership`의 2개, 이번 `pimpl_main`의 1개는 런타임 호출과 별도입니다.

## 실제로 확인한 범위

| 기록 | 환경·실행 | 결과 | 근거 |
| --- | --- | --- | --- |
| 원본에 포함된 2026-10-13 기록 | GCC 14.2 / Clang 17 / Clang 17 + ASan·UBSan | 각 구성에서 원본 CTest 9/9, 89회. GCC·Clang 각각 실패 예제 6개 거부 | [과거 로그 안내](verification/historical-20261013/README.md), [원본 요약](original/VALIDATION.md) |
| 2026-10-21 · 01–02 배치 로컬 실행 | GCC 13.3.0, C++20, 경고를 오류로 취급 | 원본 9개 + 주석판 3개 빌드·실행 성공. 출력 3쌍 바이트 일치 | [로컬 원시 로그](verification/local-gcc-20261021/README.md) |
| 2026-10-21 · 01–02 배치 로컬 실패 예제 | GCC 13.3.0, C++20, `-pedantic-errors -c` | 6/6 예상 컴파일 거부. 정상 테스트 통과 수와 별도 | [실패 예제 결과](verification/local-gcc-20261021/negative/results.json) |
| 2026-10-21 · 01–02 배치 로컬 C++11 호환성 조사 | GCC 13.3.0, `-std=c++11 -pedantic-errors -fsyntax-only` | 원본 9/9 컴파일 거부. **C++11 미지원**, 실행하지 않음 | [C++11 진단](verification/local-gcc-20261021/cxx11/results.json) |
| 2026-10-21 · 01–02 배치 GitHub Actions 검증 | GCC 13.3.0 / Clang 18.1.3 + ASan·UBSan, C++20, CMake 3.31.6 | 각 Configure·Build 성공, **CTest 12/12**. 출력 3쌍씩 일치. 각 컴파일러에서 실패 예제 6개 거부, GCC C++11 원본 9개 미지원 확인 | [실행 39557286731](https://github.com/jiwhan112/um.github.io/actions/runs/39557286731), [원시 기록 55파일](verification/github-39557286731/README.md) |
| 2026-10-21 · 03–04 배치 로컬 실행 | GCC 13.3.0, C++20, 경고를 오류로 취급 | 원본 9개 + 주석판 6개 **15개 성공**, 출력 **6쌍** 바이트 동일. 별도 환경 조회의 unsigned 값 비트 수 **32** | [03–04 로컬 기록](verification/local-gcc-advanced34-20261021/README.md) |
| 2026-10-21 · 03–04 배치 실패·호환성 조사 | GCC 13.3.0, 엄격한 컴파일 설정 | C++20 실패 예제 **6/6 거부**, C++11 원본 **9/9 미지원·미실행** | [실패 예제](verification/local-gcc-advanced34-20261021/negative/results.json), [C++11 진단](verification/local-gcc-advanced34-20261021/cxx11/results.json) |
| 2026-10-21 · 03–04 배치 GitHub CI | GCC13.3 / Clang18.1.3 ASan·UBSan, C++20, CMake3.31.6 | 각 Configure·Build 성공, **CTest15/15**, 출력 **6쌍** 바이트 동일. 각 컴파일러의 거부 예제6개, GCC C++11 원본9개 미지원 확인 | [실행39565624710](https://github.com/jiwhan112/um.github.io/actions/runs/39565624710), [새 원시73파일](verification/github-39565624710/README.md) |
| 2026-10-21 · 05–06 배치 로컬 실행 | GCC 13.3.0, C++20, 경고를 오류로 취급 | 원본 9개 + 주석판 9개 **18개 성공**, 출력 **9쌍** 바이트 동일. 원본과 주석판 Pimpl 각각 두 CPP를 함께 링크 | [05–06 로컬 기록](verification/local-gcc-advanced56-20261021/README.md) |
| 2026-10-21 · 05–06 배치 실패·호환성 조사 | GCC 13.3.0, 엄격한 컴파일 설정 | C++20 실패 예제 **6/6 거부**, C++11 원본 **9/9 미지원·미실행** | [실패 예제](verification/local-gcc-advanced56-20261021/negative/results.json), [C++11 진단](verification/local-gcc-advanced56-20261021/cxx11/results.json) |
| 2026-10-21 · 05–06 배치 GitHub CI | GCC 13.3.0 / Clang 18.1.3 ASan·UBSan, C++20, CMake 3.31.6 | 각 Configure·Build 성공, **CTest 18/18**, 출력 **9쌍** 바이트 동일. 각 컴파일러의 거부 예제 6개, GCC C++11 원본 9개 미지원 확인 | [실행 39571778548](https://github.com/jiwhan112/um.github.io/actions/runs/39571778548), [새 원시 85파일](verification/github-39571778548/README.md), [워크플로](https://github.com/jiwhan112/um.github.io/blob/master/.github/workflows/knowledge-cpp-advanced.yml) |

로컬 환경에 Clang·CMake·CTest가 없어 이번 로컬 기록도 GCC 직접 빌드입니다. 위 **01–02 배치의** CMake·CTest와 Clang sanitizer 성공은 소스 커밋 `4cbd10710d0bfce8cb93fae89a9545369c797985`에서 실행한 12개 구성의 기록이며, **03–04 배치의** 15개 구성 및 현재 18개 구성과 구분합니다. 당시 네 artifact ZIP의 GitHub digest와 추출된 **55파일 / 261,873바이트**를 독립 대조한 증거는 그대로 보존했습니다. 이전 03–04 실행은 소스 커밋 `cfce833acbd970e7bdbd675efd8e18e3f9f5e61c`을 사용했으며, artifact 4개의 GitHub digest와 원시 **73파일 / 296,016바이트**를 대조해 별도로 보존했습니다.

이번 **05–06 GitHub 실행의 소스 커밋은 `f4ad911d6407d0b9740ad8c75dd40ef45b6929dc`**입니다. 새 artifact 4개의 GitHub digest와 원시 **85파일 / 330,326바이트**를 독립적으로 대조했습니다. 각 빌드는 원본·주석판의 Pimpl 두 CPP를 포함한 **20개 번역 단위와 18개 실행 파일 링크**이며, Clang의 모든 컴파일·링크에 ASan/UBSan이 적용되었습니다. 현재 로컬 기록·새 GitHub 증거·과거 기록은 별도 폴더에 보존하고, 로그를 보관하는 후속 커밋과 실제 검증 대상 소스 커밋을 구분합니다. C++ CI 성공과 웹 UI·공개 배포 결과도 별개입니다.

**미검증:** Windows/MSVC, Direct3D·GPU·HLSL, 실제 게임 통합, 동시 실행과 경쟁 조건, ABI 호환성, 성능 측정, 모든 예외 지점과 입력을 망라한 커버리지. ASan·UBSan 실행도 해당 실행 경로의 검출 결과이며 전체 프로그램의 안전성 증명이 아닙니다.

## 재실행 방법

아래 명령은 저장소 루트에서 실행합니다. Python 3, CMake 3.20 이상과 C++20 컴파일러가 필요합니다. 원본과 주석판을 함께 구성하는 **이 폴더의 CMake**를 사용합니다.

```sh
python3 knowledge/shared/cpp-advanced-lab/verify_sources.py
cmake -S knowledge/shared/cpp-advanced-lab -B build/cpp-advanced-gcc \
  -DCMAKE_CXX_COMPILER=g++ -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Debug
cmake --build build/cpp-advanced-gcc --parallel 2
ctest --test-dir build/cpp-advanced-gcc --output-on-failure --verbose
```

현재 정상 CTest 구성은 원본 9개와 한국어 주석판 9개, 총 18개입니다. `pimpl`과 `pimpl_ko`는 각각 구현 CPP와 진입점 CPP를 함께 링크하며 헤더를 별도 프로그램으로 세지 않습니다. 단원별로 좁혀 실행하려면 다음과 같이 이름을 지정합니다.

```sh
ctest --test-dir build/cpp-advanced-gcc --output-on-failure -R '^lifetime(_ko)?$'
ctest --test-dir build/cpp-advanced-gcc --output-on-failure -R '^(ownership|transaction)(_ko)?$'
ctest --test-dir build/cpp-advanced-gcc --output-on-failure -R '^polymorphism(_ko)?$'
ctest --test-dir build/cpp-advanced-gcc --output-on-failure -R '^(templates|interfaces)(_ko)?$'
ctest --test-dir build/cpp-advanced-gcc --output-on-failure -R '^containers(_ko)?$'
ctest --test-dir build/cpp-advanced-gcc --output-on-failure -R '^(allocation|pimpl)(_ko)?$'
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py compare \
  --compiler g++ --build build/cpp-advanced-gcc --out build/cpp-advanced-logs/output-equivalence
```

Clang과 sanitizer 실행은 별도 빌드 디렉터리에서 구성합니다.

```sh
cmake -S knowledge/shared/cpp-advanced-lab -B build/cpp-advanced-clang \
  -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_SANITIZERS=ON
cmake --build build/cpp-advanced-clang --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ctest --test-dir build/cpp-advanced-clang --output-on-failure --verbose
```

실패 예제와 C++11 호환성 조사는 정상 CTest에 섞지 않습니다.

```sh
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py negative \
  --compiler g++ --out build/cpp-advanced-logs/negative
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py cxx11 \
  --compiler g++ --out build/cpp-advanced-logs/cxx11-unsupported
```

`negative`는 소스 6개를 각각 컴파일만 하여 실패를 확인하며 링크·실행하지 않습니다. `cxx11`의 `status: pass`는 **예상한 미지원 진단을 확인했다**는 검사기의 상태이며, C++11 빌드 성공을 뜻하지 않습니다. 공통 헤더의 `std::invoke`부터 C++17 기능이며 팩 전체는 C++20 기준으로 작성되었습니다. CMake에 다른 표준을 지정하면 조용히 C++20으로 바꾸지 않고 중단합니다.

원본 `original/run_checks.py`는 역사 보존용입니다. 스크립트가 원래 `logs/`에 출력을 쓰는 동작까지 그대로 보존했습니다. 새 로그를 보관할 때는 위의 검사기와 별도 출력 경로를 사용해 과거 원시 로그와 구분합니다. 이 팩의 예외 주입 예제는 정상 프로그램 안의 예외 경로이며, 기초 팩의 의도적 UB 프로그램과는 별개입니다.

## 해석할 때 지킬 경계

- `IntBuffer`의 실패 주입은 `FailPoint::next_copy`를 소비한 뒤 `bad_alloc`을 던지는 결정적 실험입니다. 실제 메모리 고갈을 일으킨 결과가 아니며, 강한 보장은 대상 버퍼의 소유 상태에 관한 것입니다. 소비한 전역 실패 주입 플래그까지 되돌리는 계약은 아닙니다.
- `GameState::Transition`은 후보 상태를 복사·편집·검증한 뒤 `unique_ptr::swap`으로 교체합니다. 단일 실행 흐름에서 소유 상태를 보존하는 예이며, 파일·네트워크 부수 효과의 롤백이나 스레드 간 원자적 게시를 구현하지 않습니다.
- `Consume(std::make_unique<Resource>(), FailPriority())`는 인자 평가 순서를 고정하지 않습니다. 리소스가 생성되기 전에 예외가 발생할 수도 있고, 생성되었다면 이미 소유한 `unique_ptr`이 정리합니다. 잔존 개수가 0이라는 결과만으로 두 평가 순서를 모두 관찰했다고 말하지 않습니다.
- `weak_ptr` 테스트는 순차 실행에서 관찰자와 `lock()`의 소유권을 확인합니다. 다른 스레드와 동시에 실행하는 수명 안전성을 이 테스트 하나로 검증하지 않습니다.

- `PowerOfTwo`의 제한값 31은 unsigned 비트 폭을 구한 결과가 아닙니다. 이번 GCC 환경에서 `std::numeric_limits<unsigned>::digits`는 32였고, 원본의 컴파일 시점 입력 10과 런타임 거부 입력 31을 확인했습니다. 임의의 플랫폼과 모든 입력에 대한 안전성을 주장하지 않으며, 16비트 unsigned 환경에서 원본 제한을 일반화할 수 없습니다. 별도 폭 조회는 환경 기록으로서 CTest·논리 검사 수에 포함하지 않습니다.
- `polymorphism`의 `Events`는 16개를 넘는 기록을 버리며, `ValueBase`의 슬라이싱 예시는 비가상 함수와 파생 데이터가 없는 최소 예입니다. NVI는 허용 상한 1000을 구현하지만 실제 거부 테스트는 -1만 실행합니다. 기본 인자 실험의 곱셈과 `Counter`·`Quantity`의 합산은 작은 테스트 값의 범위입니다.
- `Label::View()`의 rvalue 오버로드 삭제는 임시 객체에서 직접 뷰를 얻는 호출을 막습니다. 이미 받은 뷰의 소유자 수명을 자동 관리하지 않으며, 댕글링 뷰를 실제로 역참조하는 실험은 하지 않습니다. Notion 03의 소스 목록에 있던 `interfaces.cpp`와 달리 NVI·Clone 구현은 실제 `polymorphism.cpp`에 있습니다.
- `containers`는 vector 삭제·삽입 후 새로운 반복자를 얻고, 재해시 뒤에는 유지되는 원소 포인터만 사용합니다. 무효화된 반복자를 역참조하지 않습니다. 비교자의 세 비반사성 표본은 모든 입력에 대한 엄격 약순서 증명이 아니며, `owners.clear()` 뒤의 직접 관측값은 `empty()`이지 별도 해제 카운터가 아닙니다.
- `allocation`의 `Pool`은 전역 `operator new/delete`를 호출하는 카운터 예제이며 재사용 풀·확장 정렬 allocator·동시성을 구현하지 않습니다. 생성 실패 시 matching placement delete와 정상 생성 시 사용자 정의 deleter를 구분합니다. PMR의 4096바이트 요청은 고정 1024바이트 arena보다 크고 upstream이 `null_memory_resource`인 조건의 실패이며, 실제 시스템 메모리 고갈이나 정확한 vector 할당 바이트를 측정한 결과가 아닙니다.
- `pimpl`의 복사 생성자가 별도 `Impl`을 만드는 근거는 `make_unique` 코드입니다. `Rename` 뒤 두 이름을 확인하는 첫 조건만으로 최초 포인터 분리를 입증하지 않습니다. 후보 생성 후 swap하는 복사 대입·Rename의 예외 보장은 코드 구조에서 읽는 계약이며, 이 프로그램의 6회 검사에 할당 실패 주입은 없습니다. 자기 대입·이동 대입·빈 원본 복사·ABI 호환성도 이 여섯 회의 실행 범위에 포함하지 않습니다.
