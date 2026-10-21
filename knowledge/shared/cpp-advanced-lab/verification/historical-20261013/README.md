# 2026-10-13 원본 실행 기록

이 폴더의 `logs/` 22파일은 [보존 ZIP](../../archive/cpp_advanced_effective_lab_v1.zip)의 `cpp_advanced_effective_lab_v1/logs/` 항목에서 바이트를 바꾸지 않고 추출했습니다. 이번 작업에서 실행하여 만든 로그가 아닙니다. 원본 CMake·소스·실행 스크립트·요약은 [original/](../../original/)에 보존했습니다.

| 과거 구성 | Configure / Build / CTest | 원본 프로그램 | 기록된 검사 호출 |
| --- | --- | --- | ---: |
| GCC 14.2.0 | 모두 성공 | 9/9 | 89 |
| Clang 17.0.0 | 모두 성공 | 9/9 | 89 |
| Clang 17.0.0 + ASan·UBSan | 모두 성공 | 9/9 | 89 |

GCC와 Clang의 컴파일 거부 예제는 각각 6개이며 정상 CTest에 포함하지 않았습니다. 같은 프로그램을 여러 구성에서 실행한 89회를 합쳐 독립 조건 수로 표현하지 않습니다. `ownership`의 13회에는 prvalue 구성의 컴파일 성공 뒤에 놓인 `Expect(true)` 1회가 포함됩니다.

- [GCC Configure](logs/gcc-configure.txt), [Build](logs/gcc-build.txt), [CTest](logs/gcc-tests.txt)
- [Clang Configure](logs/clang-configure.txt), [Build](logs/clang-build.txt), [CTest](logs/clang-tests.txt)
- [Clang sanitizer Configure](logs/clang-sanitized-configure.txt), [Build](logs/clang-sanitized-build.txt), [CTest](logs/clang-sanitized-tests.txt)
- [원본 report.json](logs/report.json)
- [원본 검증 요약](../../original/VALIDATION.md)
- [이번 GCC 로컬 실행 기록](../local-gcc-20261021/README.md)

파일별 바이트 수와 SHA-256, ZIP 항목과의 정확한 일치 여부는 [source-integrity.json](../../source-integrity.json)의 `zip_files` 중 `kind: historical-raw-log`인 항목에서 확인할 수 있습니다. 이 기록에는 Windows/MSVC·GPU·게임 통합·경쟁 조건·성능 검증이 포함되어 있지 않습니다.
