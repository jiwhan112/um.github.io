# 2026-10-21 새 로컬 GCC 실행

이 기록은 `g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0`으로 이번 작업 중 직접 실행한 결과입니다. 소스와 컴파일·실행 명령, 종료 코드, stdout·stderr 원시 바이트를 함께 보관했습니다. 원본 ZIP에 들어 있던 2026-10-13 로그와 별개입니다.

| 조사 | 실제 결과 | 원시 기록 |
| --- | --- | --- |
| C++20 직접 빌드 | 원본 9개 + 주석판 3개, 모두 성공 | [positive/results.json](positive/results.json), 각 `*-build.txt` |
| 정상 실행 | 12개 모두 종료 코드 0 | 각 `positive/*-run.txt` |
| 출력 동일성 | lifetime / ownership / transaction 원본·주석판 3쌍, 정규화 없이 바이트 동일 | `positive/results.json`의 `all_three_stdout_pairs_equal: true` |
| C++20 컴파일 실패 예제 | 6/6 예상 거부, 각 컴파일러 종료 코드 1 | [negative/results.json](negative/results.json), 각 진단 `.txt` |
| 엄격한 C++11 호환성 | 원본 9/9 거부, 실행하지 않음 | [cxx11/results.json](cxx11/results.json), 각 진단 `.txt` |
| CMake·CTest·Clang | 로컬 도구가 없어 실행하지 않음 | [environment.json](environment.json) |

원본 전체의 검사 호출은 89회이며, 이번 교재의 `lifetime` 10 + `ownership` 13 + `transaction` 8 = 31회입니다. 주석판에서 같은 31회를 다시 실행한 값을 독립 조건으로 더하지 않습니다. `ownership`에는 컴파일 성공이 핵심인 prvalue 예제 뒤의 `Expect(true)` 1회가 포함됩니다.

정상 빌드는 `-std=c++20 -g -O0 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`를 사용했습니다. 성공한 빌드의 출력 파일이 0바이트인 것은 출력이 없었다는 뜻이며, `results.json`의 해당 명령 종료 코드 0과 함께 해석합니다. 컴파일러·임시 실행 파일 경로를 포함한 실제 명령 배열은 각 결과 JSON에 있습니다. 실행 파일은 임시 디렉터리에 만들고 실행 후 제거했습니다.

컴파일 실패 예제의 진단은 각각 제약 불충족, 값 매개변수 복사 생성자, `unique_ptr` 복사 삭제, `list` 반복자와 `std::sort`의 부적합, 임시 객체의 삭제된 `View() &&`, 잘못된 `override`입니다. `cxx11`은 `-pedantic-errors -fsyntax-only`로 원본을 조사했습니다. `status: pass`는 예상한 **C++11 미지원**을 확인했다는 뜻입니다.

[file-manifest.json](file-manifest.json)에 원시 로그·결과·환경 46파일의 바이트 수와 SHA-256을 기록했습니다. 현재 로그를 다시 생성하려면 저장소 루트에서 아래 명령을 실행하되 새 출력 경로를 선택합니다.

```sh
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py positive-direct \
  --compiler g++ --out build/cpp-advanced-local/positive
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py negative \
  --compiler g++ --out build/cpp-advanced-local/negative
python3 knowledge/shared/cpp-advanced-lab/verify_experiments.py cxx11 \
  --compiler g++ --out build/cpp-advanced-local/cxx11
```

이 로컬 기록은 CMake Configure·Build·CTest, Clang sanitizer, Windows/MSVC, GPU, 동시 실행, 성능을 검증하지 않습니다. 새 GitHub CI와 공개 배포는 각각 별도 확인 대상입니다.
