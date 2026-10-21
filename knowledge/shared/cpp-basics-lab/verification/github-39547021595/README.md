# C++ 기초 01~05 전체 소스 · GitHub CI 원시 기록

- 실제 실행: [39547021595](https://github.com/jiwhan112/um.github.io/actions/runs/39547021595) · 2026-10-21 UTC
- 검사한 소스 커밋: [`10a5e43c015231d326e26a5b16ba39887a64aa84`](https://github.com/jiwhan112/um.github.io/commit/10a5e43c015231d326e26a5b16ba39887a64aa84)
- 워크플로: `C++ basics preserved sources validation` · 다섯 작업 모두 `completed / success`
- 환경: Ubuntu 24.04 / GCC 13.3.0 / Clang 18.1.3 / CMake 3.31.6

이 폴더는 이번 04~05 한국어판을 포함하는 새 실행 결과다. [이전 01~03 배치의 9/9 기록](../github-39532381043/README.md)과 Notion의 2026-10-12 과거 실행 요약은 별도 자료로 남긴다.

## 정상 프로그램

| 구성 | 실제 관찰 | 원시 Configure·Build·CTest | 원본·한국어판 비교 |
| --- | --- | --- | --- |
| GCC C++20 | 원본 6개 + 한국어판 6개, **CTest 12/12 통과** | [Configure](gcc-cxx20/configure.txt) · [Build](gcc-cxx20/build.txt) · [CTest](gcc-cxx20/ctest.txt) | [6쌍 결과](gcc-cxx20/output-equivalence/results.json) |
| Clang C++20 + ASan/UBSan | 원본 6개 + 한국어판 6개, **CTest 12/12 통과**, 정상 실행 중 sanitizer 오류 없음 | [Configure](clang-cxx20-asan-ubsan/configure.txt) · [Build](clang-cxx20-asan-ubsan/build.txt) · [CTest](clang-cxx20-asan-ubsan/ctest.txt) | [6쌍 결과](clang-cxx20-asan-ubsan/output-equivalence/results.json) |

정상 프로그램은 `types`, `flow`, `memory`, `containers`, `buffer`, `inventory`와 각각의 `_ko` 판이다. `inventory`는 구현·진입점 두 cpp를 실제로 함께 연결한다. 모든 빌드는 `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`를 사용한다. Clang 정상 빌드에는 `-fsanitize=address,undefined -fno-omit-frame-pointer`가 추가된다.

각 구성에서 원본·한국어판 6쌍의 stdout이 바이트 단위로 일치했다. 두 프로그램은 같은 작업 디렉터리에서 실행하며 `cwd=`를 비롯한 출력을 정규화하지 않았다. 환경·소스 무결성 출력은 각각 [GCC](gcc-cxx20/environment-and-integrity.txt), [Clang](clang-cxx20-asan-ubsan/environment-and-integrity.txt)에 있다.

## 예상한 거부·진단

| 실험 | 실제 결과 | 명령·종료 코드와 원시 출력 위치 |
| --- | --- | --- |
| C++20 부정 예제 4개 | GCC·Clang이 각각 컴파일 오류, 종료 코드 1로 거부 | [GCC](gcc-rejections/negative/results.json) · [Clang](clang-rejections/negative/results.json) |
| GCC C++11 엄격 모드 | 원본 6개 모두 미지원 진단으로 거부, 실행하지 않음 | [C++11 결과](gcc-rejections/cxx11-unsupported/results.json) |
| 의도적 signed overflow·해제 후 접근 | UBSan `signed integer overflow`, ASan `heap-use-after-free`, 각각 종료 코드 1 | [별도 진단 결과](clang-intentional-diagnostics/intentional-diagnostics/results.json) |

거부가 예상되는 실험과 의도적 미정의 동작은 정상 CTest 12개에 들어 있지 않다. C++11 검사기의 `status: pass`는 **예상한 미지원 진단을 확인했다는 뜻**이며 C++11 빌드·실행 성공이 아니다. sanitizer 실험도 정상 프로그램의 성공으로 세지 않는다.

## 원시 바이트와 집계

GitHub artifact ZIP 5개의 SHA-256을 API의 `digest`와 먼저 비교한 뒤 원시 파일 **67개, 185,553바이트**를 수정 없이 추출했다. 파일별 크기·SHA-256, artifact ID·이름·ZIP 해시는 [artifact-manifest.json](artifact-manifest.json)에 있다. 이 README와 manifest는 원시 파일 67개에 포함하지 않는다. 만료되는 다운로드 URL은 저장하지 않는다.

원본 프로그램 CHECK 횟수는 **15 / 15 / 19 / 49 / 104 / 19**, 합계 **221회**다. 04~05는 그중 172회다. 반복문과 원본·주석판·컴파일러의 재실행을 서로 다른 독립 조건으로 합산하지 않는다.

소스 보존과 이 CI 실행은 서로 다른 증거다. 원본 16개 파일은 첨부 코드 블록과 바이트를 비교하고, 한국어 소스·헤더 8개는 전처리 지시문과 C++ 토큰을 대조한다. Windows/MSVC·Direct3D·GPU·HLSL 및 게임 프로젝트 통합은 실행하지 않았다. 기존 ZIP과 과거 원시 로그 미회수 상태도 이 새 로그로 해소됐다고 표시하지 않는다.
