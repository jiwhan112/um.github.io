# C++ 심화 01–02 · GitHub CI 원시 기록

[워크플로 실행 39557286731](https://github.com/jiwhan112/um.github.io/actions/runs/39557286731)은 **2026-10-21 06:44:39 UTC에 success로 완료**되었습니다. 네 작업 모두 성공했으며, 대상 소스는 [커밋 4cbd10710d0bfce8cb93fae89a9545369c797985](https://github.com/jiwhan112/um.github.io/commit/4cbd10710d0bfce8cb93fae89a9545369c797985)입니다.

이 디렉터리는 실행이 끝난 뒤 받은 artifact의 보관 자료이며 후속 커밋으로 추가합니다. 위 커밋은 **실제 컴파일한 소스 커밋**이고 원시 로그를 추가하는 커밋과 구분합니다. 원본 소스·한국어 주석판·CMake·워크플로의 바이트는 이 감사에서 변경하지 않았습니다. 이 문서의 판정 범위는 C++ CI이며, 웹 UI와 GitHub Pages 공개 배포는 별도 검증 대상입니다.

## 실제 실행 결과

| 구성 | 환경 | Configure / Build / CTest | 추가 확인 |
| --- | --- | --- | --- |
| `positive-gcc-cxx20` | GCC 13.3.0, CMake 3.31.6, C++20 | 모두 성공, **12/12** | 원본·한국어 주석판 출력 3쌍 바이트 동일 |
| `positive-clang-cxx20-asan-ubsan` | Clang 18.1.3, CMake 3.31.6, C++20, ASan·UBSan | 모두 성공, **12/12** | 원본·한국어 주석판 출력 3쌍 바이트 동일 |
| `expected-compile-rejections-g++` | GCC 13.3.0 | 정상 CTest와 별도 | C++20 실패 예제 **6/6 거부**, 엄격한 C++11 원본 **9/9 거부** |
| `expected-compile-rejections-clang++` | Clang 18.1.3 | 정상 CTest와 별도 | C++20 실패 예제 **6/6 거부**. C++11 단계는 설정에 따라 실행하지 않음 |

컴파일러의 전체 버전 문자열은 `g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`과 `Ubuntu clang version 18.1.3 (1ubuntu1)`입니다. 실행 환경은 GitHub Actions Ubuntu 24.04의 x86-64 Linux이며, 정확한 커널·glibc 정보는 각 `results.json`에 기록되어 있습니다. Clang 실패 진단에는 libstdc++ 14 헤더 경로가 나타납니다. 이 실행을 libc++ 검증으로 표현하지 않습니다.

### 실제 빌드 명령에서 확인한 설정

[GCC Build](positive-gcc/build.txt)와 [Clang Build](positive-clang-sanitized/build.txt)에는 각각 **번역 단위 컴파일 13개와 실행 파일 링크 12개**가 있습니다. `pimpl`이 두 `.cpp` 파일을 링크하므로 번역 단위 수가 하나 더 많습니다.

모든 컴파일 명령에서 `-std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`를 확인했습니다. Clang의 원본 9개와 주석판 3개에 해당하는 모든 번역 단위에는 `-fsanitize=address,undefined -fno-omit-frame-pointer`, 모든 링크 명령에는 `-fsanitize=address,undefined`가 있습니다. 주석판만 sanitizer를 적용한 구성이 아닙니다.

[GCC CTest](positive-gcc/ctest.txt)와 [Clang sanitizer CTest](positive-clang-sanitized/ctest.txt)는 모두 `100% tests passed, 0 tests failed out of 12`를 기록했습니다. 정상 실행 및 추가 출력 대조에서 sanitizer 진단을 발견하지 않았습니다. 실행하지 않은 경로나 모든 입력에 대해 안전성을 증명하는 결과는 아닙니다.

## 검사 횟수의 의미

| 원본 프로그램 | 정상 실행에서 센 호출 수 | 이번 집필 범위 |
| --- | ---: | --- |
| lifetime | 10 | 심화 01 |
| ownership | 13 | 심화 02 |
| polymorphism | 12 | 후속 단원의 보존 원본 |
| templates | 10 | 후속 단원의 보존 원본 |
| containers | 16 | 후속 단원의 보존 원본 |
| interfaces | 7 | 후속 단원의 보존 원본 |
| transaction | 8 | 심화 02 |
| allocation | 7 | 후속 단원의 보존 원본 |
| pimpl | 6 | 후속 단원의 보존 원본 |

전체 원본은 **89회**, 이번 두 단원에 해당하는 프로그램은 **31회(10+13+8)**입니다. 주석판 세 개의 같은 31회, 두 컴파일러에서의 재실행, 출력 비교를 위한 재실행은 독립 조건 수에 더하지 않습니다. 후속 단원의 원본을 실행했다는 사실을 후속 교재 집필 완료로 집계하지 않습니다.

`ownership`에는 prvalue 반환 예제 뒤의 `Expect(true)` 1회가 있습니다. 그 예제는 복사·이동 생성자가 삭제된 타입을 구성하는 코드의 **컴파일 성공**이 핵심 근거이며, 모든 13회를 서로 다른 비자명한 런타임 논리 조건으로 표현하지 않습니다. 같은 파일의 이동 특성 `static_assert` 2개는 이 13회와 별도입니다.

출력 비교는 각 구성에서 `lifetime`, `ownership`, `transaction`의 원본·주석판을 같은 작업 디렉터리에서 실행하여 **정규화 없이 원시 바이트로 비교**했습니다. 세 쌍 모두 같고, 각 명령의 종료 코드가 0입니다. [GCC 결과](positive-gcc/output-equivalence/results.json)와 [Clang 결과](positive-clang-sanitized/output-equivalence/results.json)에 명령·작업 경로·종료 코드를 보관했습니다.

## 컴파일 거부와 C++11 미지원

실패 예제는 `-std=c++20 -pedantic-errors -c`로 각각 컴파일만 했으며 링크·실행하지 않았습니다. GCC와 Clang의 원시 진단을 읽어 다음 원인과 대응하는지 확인했습니다.

| 예제 | 확인한 거부 이유 | GCC 진단 | Clang 진단 |
| --- | --- | --- | --- |
| constraint | double이 integral 제약을 충족하지 않음 | [원시 출력](compile-rejections-gcc/negative/constraint.txt) | [원시 출력](compile-rejections-clang/negative/constraint.txt) |
| copy_by_value | 복사 생성자의 첫 매개변수를 값으로 선언 | [원시 출력](compile-rejections-gcc/negative/copy_by_value.txt) | [원시 출력](compile-rejections-clang/negative/copy_by_value.txt) |
| copy_unique | 삭제된 unique_ptr 복사 생성자 호출 | [원시 출력](compile-rejections-gcc/negative/copy_unique.txt) | [원시 출력](compile-rejections-clang/negative/copy_unique.txt) |
| list_sort | list 반복자가 std::sort에 필요한 연산을 제공하지 않음 | [원시 출력](compile-rejections-gcc/negative/list_sort.txt) | [원시 출력](compile-rejections-clang/negative/list_sort.txt) |
| temporary_view | 임시 객체에서 삭제된 View() && 호출 | [원시 출력](compile-rejections-gcc/negative/temporary_view.txt) | [원시 출력](compile-rejections-clang/negative/temporary_view.txt) |
| wrong_override | const 한정이 다른 멤버를 override로 선언 | [원시 출력](compile-rejections-gcc/negative/wrong_override.txt) | [원시 출력](compile-rejections-clang/negative/wrong_override.txt) |

각 컴파일러가 여섯 번 모두 종료 코드 1을 반환했습니다. 헤더를 찾지 못하거나 컴파일러 실행 자체가 실패한 결과를 예상 거부로 오인한 기록은 없습니다.

[C++11 결과](compile-rejections-gcc/cxx11-unsupported/results.json)는 GCC의 `-std=c++11 -pedantic-errors -fsyntax-only` 실행입니다. 원본 9개 모두 종료 코드 1이며 공통 헤더의 `std::invoke`, `std::make_unique` 등 해당 표준에서 사용할 수 없는 기능이 진단됩니다. **C++11은 미지원이고 프로그램을 실행하지 않았습니다.** 검사기의 `status: pass`는 예상한 미지원 진단을 확인했다는 뜻입니다.

이 팩의 예외 주입은 정상 프로그램 내부의 예외 경로입니다. 의도적 UB 프로그램을 추가하거나 실행한 기록은 없습니다.

## artifact와 파일 무결성

GitHub API에서 이 실행의 완료 상태·소스 SHA·네 작업의 성공을 확인했습니다. artifact ZIP 네 개를 독립적으로 다시 받아 SHA-256을 계산하여 **GitHub API digest 및 보관 manifest와 모두 일치**하는지 확인했고, ZIP의 모든 항목을 보관한 파일과 바이트 단위로 대조했습니다.

| 보관 디렉터리 | GitHub artifact ID | ZIP 바이트 | 원시 출력·환경·결과 파일 |
| --- | ---: | ---: | ---: |
| compile-rejections-gcc | 12325958067 | 21,353 | 20 |
| compile-rejections-clang | 12325649740 | 10,086 | 9 |
| positive-gcc | 12325623543 | 12,052 | 13 |
| positive-clang-sanitized | 12325149366 | 11,897 | 13 |
| 합계 | 네 artifact | 55,388 | **55** |

추출된 55파일은 **261,873바이트**입니다. [artifact-manifest.json](artifact-manifest.json)에 각 ZIP 해시와 파일별 바이트 수·SHA-256·artifact ID를 기록했습니다. 실제 파일 목록과 manifest의 목록도 정확히 일치합니다. README와 manifest 자체는 원시 파일 55개에 포함하지 않습니다.

네 작업의 `environment-and-integrity.txt`에 기록된 소스 SHA는 모두 `4cbd10710d0bfce8cb93fae89a9545369c797985`이며, 각 파일 안의 원본 무결성 JSON도 [source-integrity.json](../../source-integrity.json)과 같습니다. 원본 ZIP·소스·한국어 주석판·과거 로그는 소스 커밋 이후 이 감사까지 변경되지 않았습니다.

로그의 `/home/runner/work/...`, `/tmp/atlas-cpp-advanced-...`, `/usr/...`는 CI 작업·임시 빌드·도구 경로로 그대로 보존했습니다. 검사한 55파일에서 자격증명, 개인 키, 서명된 임시 다운로드 주소는 발견하지 않았습니다.

## 검증 범위의 한계

Windows/MSVC, Direct3D·GPU·HLSL, 실제 게임 통합, 동시 실행과 경쟁 조건, ABI, 성능, 모든 입력과 예외 지점의 커버리지는 검증하지 않았습니다. ASan·UBSan 성공은 이번에 실행한 경로의 결과입니다. `IntBuffer`의 실패 주입은 결정적으로 던진 예외이며 실제 메모리 고갈 실험이 아닙니다. `GameState`의 후보 교체는 소유 상태의 순차 실행에 관한 것으로 파일·네트워크 롤백이나 스레드 간 원자적 게시를 검증하지 않습니다.

## 부록 · 보관한 원시 파일 55개

아래 파일 모두를 새로 받은 ZIP 항목 및 manifest의 SHA-256과 대조했습니다.

### compile-rejections-gcc

| 파일 | 바이트 |
| --- | ---: |
| [cxx11-unsupported/allocation.txt](compile-rejections-gcc/cxx11-unsupported/allocation.txt) | 6,867 |
| [cxx11-unsupported/compiler-version.txt](compile-rejections-gcc/cxx11-unsupported/compiler-version.txt) | 246 |
| [cxx11-unsupported/containers.txt](compile-rejections-gcc/cxx11-unsupported/containers.txt) | 6,260 |
| [cxx11-unsupported/interfaces.txt](compile-rejections-gcc/cxx11-unsupported/interfaces.txt) | 4,967 |
| [cxx11-unsupported/lifetime.txt](compile-rejections-gcc/cxx11-unsupported/lifetime.txt) | 1,273 |
| [cxx11-unsupported/ownership.txt](compile-rejections-gcc/cxx11-unsupported/ownership.txt) | 9,683 |
| [cxx11-unsupported/pimpl.txt](compile-rejections-gcc/cxx11-unsupported/pimpl.txt) | 5,687 |
| [cxx11-unsupported/polymorphism.txt](compile-rejections-gcc/cxx11-unsupported/polymorphism.txt) | 3,268 |
| [cxx11-unsupported/results.json](compile-rejections-gcc/cxx11-unsupported/results.json) | 7,041 |
| [cxx11-unsupported/templates.txt](compile-rejections-gcc/cxx11-unsupported/templates.txt) | 10,758 |
| [cxx11-unsupported/transaction.txt](compile-rejections-gcc/cxx11-unsupported/transaction.txt) | 5,073 |
| [environment-and-integrity.txt](compile-rejections-gcc/environment-and-integrity.txt) | 25,708 |
| [negative/compiler-version.txt](compile-rejections-gcc/negative/compiler-version.txt) | 246 |
| [negative/constraint.txt](compile-rejections-gcc/negative/constraint.txt) | 1,735 |
| [negative/copy_by_value.txt](compile-rejections-gcc/negative/copy_by_value.txt) | 249 |
| [negative/copy_unique.txt](compile-rejections-gcc/negative/copy_unique.txt) | 884 |
| [negative/list_sort.txt](compile-rejections-gcc/negative/list_sort.txt) | 2,596 |
| [negative/results.json](compile-rejections-gcc/negative/results.json) | 4,563 |
| [negative/temporary_view.txt](compile-rejections-gcc/negative/temporary_view.txt) | 721 |
| [negative/wrong_override.txt](compile-rejections-gcc/negative/wrong_override.txt) | 309 |

### compile-rejections-clang

| 파일 | 바이트 |
| --- | ---: |
| [environment-and-integrity.txt](compile-rejections-clang/environment-and-integrity.txt) | 25,708 |
| [negative/compiler-version.txt](compile-rejections-clang/negative/compiler-version.txt) | 110 |
| [negative/constraint.txt](compile-rejections-clang/negative/constraint.txt) | 1,023 |
| [negative/copy_by_value.txt](compile-rejections-clang/negative/copy_by_value.txt) | 306 |
| [negative/copy_unique.txt](compile-rejections-clang/negative/copy_unique.txt) | 585 |
| [negative/list_sort.txt](compile-rejections-clang/negative/list_sort.txt) | 3,964 |
| [negative/results.json](compile-rejections-clang/negative/results.json) | 4,587 |
| [negative/temporary_view.txt](compile-rejections-clang/negative/temporary_view.txt) | 835 |
| [negative/wrong_override.txt](compile-rejections-clang/negative/wrong_override.txt) | 664 |

### positive-gcc

| 파일 | 바이트 |
| --- | ---: |
| [build.txt](positive-gcc/build.txt) | 26,214 |
| [configure.txt](positive-gcc/configure.txt) | 412 |
| [ctest.txt](positive-gcc/ctest.txt) | 5,116 |
| [environment-and-integrity.txt](positive-gcc/environment-and-integrity.txt) | 26,045 |
| [output-equivalence-summary.txt](positive-gcc/output-equivalence-summary.txt) | 749 |
| [output-equivalence/compiler-version.txt](positive-gcc/output-equivalence/compiler-version.txt) | 246 |
| [output-equivalence/lifetime-korean.txt](positive-gcc/output-equivalence/lifetime-korean.txt) | 25 |
| [output-equivalence/lifetime-original.txt](positive-gcc/output-equivalence/lifetime-original.txt) | 25 |
| [output-equivalence/ownership-korean.txt](positive-gcc/output-equivalence/ownership-korean.txt) | 26 |
| [output-equivalence/ownership-original.txt](positive-gcc/output-equivalence/ownership-original.txt) | 26 |
| [output-equivalence/results.json](positive-gcc/output-equivalence/results.json) | 3,575 |
| [output-equivalence/transaction-korean.txt](positive-gcc/output-equivalence/transaction-korean.txt) | 27 |
| [output-equivalence/transaction-original.txt](positive-gcc/output-equivalence/transaction-original.txt) | 27 |

### positive-clang-sanitized

| 파일 | 바이트 |
| --- | ---: |
| [build.txt](positive-clang-sanitized/build.txt) | 27,411 |
| [configure.txt](positive-clang-sanitized/configure.txt) | 418 |
| [ctest.txt](positive-clang-sanitized/ctest.txt) | 5,116 |
| [environment-and-integrity.txt](positive-clang-sanitized/environment-and-integrity.txt) | 25,909 |
| [output-equivalence-summary.txt](positive-clang-sanitized/output-equivalence-summary.txt) | 749 |
| [output-equivalence/compiler-version.txt](positive-clang-sanitized/output-equivalence/compiler-version.txt) | 110 |
| [output-equivalence/lifetime-korean.txt](positive-clang-sanitized/output-equivalence/lifetime-korean.txt) | 25 |
| [output-equivalence/lifetime-original.txt](positive-clang-sanitized/output-equivalence/lifetime-original.txt) | 25 |
| [output-equivalence/ownership-korean.txt](positive-clang-sanitized/output-equivalence/ownership-korean.txt) | 26 |
| [output-equivalence/ownership-original.txt](positive-clang-sanitized/output-equivalence/ownership-original.txt) | 26 |
| [output-equivalence/results.json](positive-clang-sanitized/output-equivalence/results.json) | 3,575 |
| [output-equivalence/transaction-korean.txt](positive-clang-sanitized/output-equivalence/transaction-korean.txt) | 27 |
| [output-equivalence/transaction-original.txt](positive-clang-sanitized/output-equivalence/transaction-original.txt) | 27 |
