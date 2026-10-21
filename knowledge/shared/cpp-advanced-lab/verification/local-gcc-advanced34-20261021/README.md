# C++ 심화 03–04 · 2026-10-21 새 로컬 GCC 기록

이번 배치에서 GCC 13.3.0으로 원본과 누적 한국어 주석판을 직접 빌드·실행한 결과입니다. 이전 01–02 로컬 기록과 GitHub 실행 기록은 별도 폴더에 변경 없이 보존했습니다. 이번 결과로 CMake·CTest나 Clang sanitizer를 실행했다고 주장하지 않습니다.

| 구분 | 실제 결과 | 근거 |
| --- | --- | --- |
| C++20 직접 빌드·실행 | 원본 9개 + 한국어 주석판 6개, **15개 성공** | [positive/results.json](positive/results.json), 각 `*-build.txt`와 `*-run.txt` |
| 출력 비교 | 원본·주석판 **6쌍 바이트 동일**, 정규화 없음 | `all_stdout_pairs_equal: true`, `stdout_pair_count: 6` |
| 컴파일 거부 예제 | C++20 예제 6개 모두 예상 거부, 컴파일러 종료 코드 1 | [negative/results.json](negative/results.json) |
| 엄격한 C++11 호환성 | 원본 9개 모두 거부, **미지원·미실행** | [cxx11/results.json](cxx11/results.json) |
| unsigned 환경 폭 | `std::numeric_limits<unsigned>::digits` = **32** | [조회 소스](positive/unsigned-width-probe.cpp), [원시 출력](positive/unsigned-width-run.txt) |
| Clang·CMake·CTest | 로컬 도구가 없어 실행하지 않음 | [environment.json](environment.json) |

이번 03–04 프로그램은 `polymorphism` 12 + `templates` 10 + `interfaces` 7 = **29회**의 검사 호출입니다. 기존 01–02의 31회와 합친 주석판 대응 범위는 **60회**, 원본 아홉 프로그램 전체는 **89회**입니다. 주석판·컴파일러 반복·출력 비교 재실행을 독립 논리 조건으로 더하지 않습니다.

`templates`의 `static_assert` 3개와 `interfaces`의 1개는 이번 런타임 29회와 별도입니다. 기존 `ownership`의 13회에는 `Expect(true)` 1회가 있으며 prvalue 구성 예제의 핵심 근거는 컴파일 성공입니다. 해당 파일의 `static_assert` 2개도 별도입니다.

정상 프로그램은 `-std=c++20 -g -O0 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`로 빌드했습니다. 명령 배열·작업 경로·종료 코드는 결과 JSON에 기록했으며, 출력 없는 성공 빌드의 `.txt`는 0바이트입니다. 임시 실행 파일은 검사가 끝난 뒤 삭제했습니다.

unsigned 폭 조회는 원본을 바꾸지 않고 현재 환경을 확인하기 위해 따로 실행한 안전한 프로그램입니다. **정상 프로그램 15개, CTest 개수, 논리 검사 호출 수에 포함하지 않습니다.** 원본 `PowerOfTwo`의 고정 제한 31이 모든 unsigned 폭을 처리한다는 뜻도 아닙니다. 실제 원본 입력은 컴파일 시점 10과 런타임 거부값 31이며, 과도한 시프트·signed overflow·댕글링 뷰 역참조 같은 unsafe 예제는 실행하지 않았습니다.

실패 예제는 `-std=c++20 -pedantic-errors -c`로 컴파일만 했습니다. 진단 원인은 integral 제약 불충족, 값 매개변수 복사 생성자, 삭제된 unique_ptr 복사, list 반복자와 std::sort의 부적합, 임시 객체의 삭제된 View, 잘못된 override입니다. C++11은 `-std=c++11 -pedantic-errors -fsyntax-only`로 확인했습니다. 검사기의 `status: pass`는 예상한 거부·미지원 확인이 성공했다는 뜻입니다.

[file-manifest.json](file-manifest.json)에 원시 출력·환경 조회 소스·결과·환경 **55파일**의 바이트 수와 SHA-256을 기록했습니다. 이 55개는 이전 GitHub 실행 39557286731의 55개와 별개입니다. 저장소 루트에서 새 출력 경로로 재실행하려면 다음 명령을 사용합니다.

```sh
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py positive-direct \
  --compiler g++ --out build/cpp-advanced34-local/positive
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py negative \
  --compiler g++ --out build/cpp-advanced34-local/negative
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py cxx11 \
  --compiler g++ --out build/cpp-advanced34-local/cxx11
```

Windows/MSVC·GPU·동시 실행·경쟁 조건·ABI·성능·모든 입력의 커버리지는 미검증입니다. 새 GitHub CI의 CMake·CTest 15개와 Clang sanitizer 결과는 실제 실행 뒤 별도 원시 기록으로 추가합니다.
