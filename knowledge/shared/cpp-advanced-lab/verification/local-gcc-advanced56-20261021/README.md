# C++ 심화 05–06 · 2026-10-21 새 로컬 GCC 기록

이번 배치에서 GCC 13.3.0으로 원본과 누적 한국어 주석판을 직접 빌드·실행한 결과입니다. 이전 01–02·03–04 로컬 기록과 GitHub 실행 기록은 각각 별도 폴더에 그대로 보존했습니다. 이번 직접 실행으로 CMake·CTest 또는 Clang sanitizer를 실행했다고 주장하지 않습니다.

| 구분 | 실제 결과 | 근거 |
| --- | --- | --- |
| C++20 직접 빌드·실행 | 원본 9개 + 한국어 주석판 9개, **18개 성공** | [positive/results.json](positive/results.json), 각 `*-build.txt`와 `*-run.txt` |
| 출력 비교 | 원본·주석판 **9쌍 바이트 동일**, 정규화 없음 | `all_stdout_pairs_equal: true`, `stdout_pair_count: 9` |
| Pimpl 다중 소스 | 원본과 주석판 각각 `pimpl.cpp` + `pimpl_main.cpp`를 함께 컴파일·링크 | `pimpl-build`, `pimpl_ko-build` 명령 배열과 6회 실행 출력 |
| 컴파일 거부 예제 | C++20 예제 6개 모두 예상 거부, 컴파일러 종료 코드 1 | [negative/results.json](negative/results.json) |
| 엄격한 C++11 호환성 | 원본 9개 모두 거부, **미지원·미실행** | [cxx11/results.json](cxx11/results.json) |
| unsigned 환경 폭 | `std::numeric_limits<unsigned>::digits` = **32** | [조회 소스](positive/unsigned-width-probe.cpp), [원시 출력](positive/unsigned-width-run.txt) |
| Clang·CMake·CTest | 로컬 도구가 없어 실행하지 않음 | [environment.json](environment.json) |

이번 05–06 프로그램은 `containers` 16 + `allocation` 7 + `pimpl` 6 = **29회**의 검사 호출입니다. 기존 01–02의 31회와 03–04의 29회를 합친 주석판 대응 범위는 원본 전체와 같은 **89회**입니다. 주석판·컴파일러 반복·출력 비교 재실행은 독립 논리 조건으로 더하지 않습니다.

`containers`에는 `Expect` 호출 위치가 14개 있습니다. 비교자 비반사성 루프의 한 위치가 세 원소에 대해 실행되므로 실행 횟수는 16회입니다. `allocation`은 Expect 5회와 Throws 2회, `pimpl_main`은 Expect 5회와 Throws 1회입니다. 이번 Pimpl의 이동 생성 특성 `static_assert` 1개는 런타임 6회와 별도입니다.

기존 `ownership`의 13회에는 `Expect(true)` 1회가 있으며 prvalue 구성 예제의 핵심 근거는 컴파일 성공입니다. 그 파일의 `static_assert` 2개와 `templates` 3개·`interfaces` 1개도 각각 런타임 호출에서 분리합니다.

정상 프로그램은 `-std=c++20 -g -O0 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`로 빌드했습니다. 명령 배열·작업 경로·종료 코드는 결과 JSON에 기록했으며, 출력 없는 성공 빌드의 `.txt`는 0바이트입니다. 임시 실행 파일은 검사가 끝난 뒤 삭제했습니다.

소스 작업 브랜치의 기준은 `59209c9674786f84fd0e9e50e01c9b269ea72e81`이며, 실행 시 HEAD는 작업 범위 원장을 추가한 `4bcd377dc4eff429335aa9263b7304ff21152d1b`였습니다. 새 주석판과 검사기 변경은 당시 미커밋 상태였으므로 [environment.json](environment.json)에 소스 무결성 목록·두 검사기·CMake·워크플로의 SHA-256을 기록해 실행에 사용한 소스를 식별했습니다. 이 기록을 이후 GitHub CI의 소스 커밋으로 잘못 인용하지 않습니다.

unsigned 폭 조회는 원본을 바꾸지 않고 현재 환경을 확인하기 위해 따로 실행한 프로그램입니다. **정상 프로그램 18개, CTest 개수, 논리 검사 호출 수에 포함하지 않습니다.** 실제 원본 `PowerOfTwo` 입력은 컴파일 시점 10과 런타임 거부값 31입니다. 과도한 시프트·무효 반복자나 댕글링 뷰 역참조·잘못된 저장소 해제 같은 unsafe 예제는 실행하지 않았습니다.

실패 예제는 `-std=c++20 -pedantic-errors -c`로 컴파일만 했습니다. 진단 원인은 integral 제약 불충족, 값 매개변수 복사 생성자, 삭제된 unique_ptr 복사, list 반복자와 std::sort의 부적합, 임시 객체의 삭제된 View, 잘못된 override입니다. C++11은 `-std=c++11 -pedantic-errors -fsyntax-only`로 확인했습니다. 검사기의 `status: pass`는 예상한 거부·미지원 확인이 성공했다는 뜻입니다.

이번 결과가 보여 주는 범위에도 한계가 있습니다. `Pool`은 일반 할당·해제 카운터 예제이며 확장 정렬·재사용·스레드 안전성을 구현하지 않습니다. PMR 4096바이트 요청은 1024바이트 arena보다 큰 요청과 null upstream의 조합이며 실제 시스템 메모리 고갈이 아닙니다. Pimpl 복사 대입·Rename에 할당 실패를 주입하지 않았고, Rename 뒤 두 이름이 다르다는 관측만으로 최초 복사 시의 포인터 분리를 증명하지 않습니다. 해당 구현의 `make_unique`와 후보 후 swap 구조는 코드 근거로 구분합니다.

[file-manifest.json](file-manifest.json)에 원시 출력·환경 조회 소스·결과·환경 **61파일 / 99,713바이트**의 바이트 수와 SHA-256을 기록했습니다. 이 파일 수에는 안내 README와 manifest 자체를 포함하지 않습니다. 이전 GitHub 기록 55파일·73파일 및 이전 로컬 기록은 이 집계에서 제외했습니다. 저장소 루트에서 새 출력 경로로 재실행하려면 다음 명령을 사용합니다.

```sh
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py positive-direct \
  --compiler g++ --out build/cpp-advanced56-local/positive
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py negative \
  --compiler g++ --out build/cpp-advanced56-local/negative
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py cxx11 \
  --compiler g++ --out build/cpp-advanced56-local/cxx11
```

Windows/MSVC·GPU·동시 실행·경쟁 조건·ABI·성능·모든 입력의 커버리지는 미검증입니다. 새 GitHub CI의 CMake·CTest 18개와 Clang sanitizer 결과는 실제 실행 뒤 별도 원시 기록으로 추가합니다.
