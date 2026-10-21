# C++ 기초 04~05 · 로컬 GCC 원시 실행 기록

2026-10-21의 Linux 실행 환경에서 **GCC 13.3.0**으로 확장된 한국어 주석판을 직접 빌드하고 실행했습니다. Clang·CMake·CTest는 이 환경에 없으므로 이 폴더는 해당 실행 결과를 포함하지 않습니다.

| 검사 | 실제 결과 | 명령·종료 코드·원시 출력 색인 |
|---|---|---|
| C++20 정상 프로그램 | 원본 6개 + 한국어 주석판 6개 빌드·실행 성공, 여섯 stdout 쌍 바이트 일치 | [positive/results.json](positive/results.json) |
| C++20 컴파일 거부 예제 | 네 소스 모두 예상대로 종료 코드 1로 거부 | [negative/results.json](negative/results.json) |
| 엄격한 C++11 구문 검사 | 원본 여섯 개 모두 예상대로 종료 코드 1로 거부, 실행 없음 | [cxx11-unsupported/results.json](cxx11-unsupported/results.json) |
| 원본 무결성 | 첨부 SHA, 16개 원본 블록, 6개 원문 capture, 8개 주석 파일 토큰·전처리 지시문 동일성 통과 | [source-integrity.txt](source-integrity.txt) |

정상 실행의 런타임 CHECK는 `types` 15회, `flow` 15회, `memory` 19회, `containers` 49회, `buffer` 104회, `inventory` 19회입니다. 합계 **221회에는 루프 반복이 포함**됩니다. 주석판이나 다른 환경에서 반복한 실행을 새 독립 조건으로 더하지 않습니다. 04~05의 부분합은 172회입니다.

빌드는 `-std=c++20 -g -O0 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`를 적용했습니다. `inventory`와 `inventory_ko`는 각각 구현과 main 두 cpp를 함께 연결했습니다. 모든 실행의 작업 디렉터리는 동일한 실습 팩 루트이며, `cwd=` 출력도 정규화하지 않고 비교했습니다.

원시 출력은 subprocess의 stdout와 stderr를 합쳐 바이트 그대로 저장했습니다. JSON에는 실제 argv·작업 디렉터리·종료 코드가 있습니다. 파일별 크기와 SHA-256은 [raw-log-manifest.json](raw-log-manifest.json)에 기록했습니다. 빈 빌드 출력도 경고·오류가 없었던 원시 기록으로 보존합니다.

실행 명령은 저장소 루트 기준입니다.

```sh
python3 knowledge/shared/cpp-basics-lab/verify_sources.py
python3 knowledge/shared/cpp-basics-lab/verify_experiments.py positive-direct --compiler g++ --out /tmp/cpp-positive
python3 knowledge/shared/cpp-basics-lab/verify_experiments.py negative --compiler g++ --out /tmp/cpp-negative
python3 knowledge/shared/cpp-basics-lab/verify_experiments.py cxx11 --compiler g++ --out /tmp/cpp-cxx11
```

이는 현재 파생 주석판의 실제 실행 기록입니다. Notion의 2026-10-12 GCC 14.2.0·Clang 17.0.0 서술, 이전 01~03의 GitHub 9/9 CTest, 이후 확장 구성의 GitHub CMake/CTest 실행과 서로 구분합니다. Windows/MSVC·Direct3D·GPU·HLSL 및 실제 게임 프로젝트 통합은 실행하지 않았습니다.
