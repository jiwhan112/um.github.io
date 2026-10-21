# C++ 기초 · GitHub Actions 실행 39532381043

이 폴더는 [C++ basics preserved sources validation](https://github.com/jiwhan112/um.github.io/actions/runs/39532381043)의 **실제 원시 출력**입니다. 2026-10-21의 push 실행이며, 검사 대상은 [`d6065fee0468d0f0447b826a608363632f36bc0e`](https://github.com/jiwhan112/um.github.io/commit/d6065fee0468d0f0447b826a608363632f36bc0e)입니다. 이후 문서·검증 기록을 추가한 커밋과 이 실행 대상 커밋을 구분합니다.

## 환경과 정상 실행

- 실행기: Ubuntu 24.04, Linux x86_64.
- GCC: `g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`.
- Clang: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake: `3.31.6`.
- 빌드: C++20, Debug, `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`.
- Clang 정상 프로그램: 컴파일·링크에 `-fsanitize=address,undefined -fno-omit-frame-pointer`. 실행 환경은 `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`, `UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`.

| 실제 실행 | Configure | Build | 정상 CTest | 원본·한국어판 stdout |
|---|---|---|---|---|
| [GCC C++20](gcc-cxx20/environment-and-integrity.txt) | [성공](gcc-cxx20/configure.txt) | [성공](gcc-cxx20/build.txt) | [9/9](gcc-cxx20/ctest.txt) | [3쌍 일치](gcc-cxx20/output-equivalence/results.json) |
| [Clang C++20 + ASan/UBSan](clang-cxx20-asan-ubsan/environment-and-integrity.txt) | [성공](clang-cxx20-asan-ubsan/configure.txt) | [성공](clang-cxx20-asan-ubsan/build.txt) | [9/9](clang-cxx20-asan-ubsan/ctest.txt) | [3쌍 일치](clang-cxx20-asan-ubsan/output-equivalence/results.json) |

각 CTest에는 원본 `types`, `flow`, `memory`, `containers`, `buffer`, `inventory`와 한국어판 `types_ko`, `flow_ko`, `memory_ko`가 있습니다. 원본 출력의 CHECK 호출 수는 차례대로 **15·15·19·49·104·19회**이고 한국어판은 **15·15·19회**입니다. 원본 팩의 221회에는 루프 반복이 포함됩니다. 이번 01~03은 그중 49회이며, 원본·한국어판·다른 컴파일러에서 같은 조건을 다시 실행한 것을 추가 독립 조건으로 집계하지 않습니다.

## 예상된 거부와 의도적 오류

| 구분 | 실제 관찰 | 기록 |
|---|---|---|
| GCC C++20 부정 예제 4개 | 모두 컴파일러 종료 코드 1 | [명령·결과](gcc-rejections/negative/results.json) |
| Clang C++20 부정 예제 4개 | 모두 컴파일러 종료 코드 1 | [명령·결과](clang-rejections/negative/results.json) |
| GCC C++11 엄격 검사 6개 | 모두 컴파일러 종료 코드 1 · 실행하지 않음 | [명령·결과](gcc-rejections/cxx11-unsupported/results.json) |
| signed overflow 진단 | `signed integer overflow`와 프로그램 종료 코드 1 | [UBSan 원시 출력](clang-intentional-diagnostics/intentional-diagnostics/signed_overflow-expected-failure.txt) |
| 해제 후 접근 진단 | `heap-use-after-free`와 프로그램 종료 코드 1 | [ASan 원시 출력](clang-intentional-diagnostics/intentional-diagnostics/use_after_free-expected-failure.txt) |

예상 실패 검사기의 `status: pass`는 예상한 거부·진단을 관찰했다는 뜻입니다. 해당 C++ 프로그램의 정상 실행 성공을 뜻하지 않습니다. 이 예제들은 정상 CTest에 포함되지 않습니다. C++11은 이 보존 팩에서 **미지원**입니다.

## 무결성과 보관 방식

GitHub artifact 5개의 ZIP digest를 GitHub 메타데이터와 대조한 다음, 내부 `.txt`·`.json` **55개**를 수정하지 않고 추출했습니다. [artifact-manifest.json](artifact-manifest.json)에 artifact ID·ZIP SHA-256·파일별 바이트 수와 SHA-256을 기록했습니다. 임시 서명 다운로드 URL은 보관하지 않았습니다. 이 README와 manifest는 원시 로그와 별도로 작성한 안내입니다.

각 작업의 `environment-and-integrity.txt`에는 실제 소스 커밋, 컴파일러 버전, Notion 본문 4개 capture 해시, 원본 첨부·16개 파일의 무결성, 한국어판의 전처리 지시문·토큰 비교 결과가 있습니다. 원본 첨부 SHA-256은 `c48d34d0e978b43f3e1b81d86e1e902052e6c5f8536c6b372c2fae8a4d2ed761`입니다.

이 기록은 새 GitHub Linux 실행입니다. 과거 Notion의 실행 서술·미회수 ZIP·미회수 원시 로그를 대신 확보한 것으로 표시하지 않습니다. Windows/MSVC 실행은 미검증이며, Direct3D·GPU·HLSL 검증 결과를 포함하지 않습니다.
