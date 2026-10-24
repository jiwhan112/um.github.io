# OS 05 · 캐시 매핑과 LRU 모델 소스 요약

원본 코드 조립과 한국어 주석 파생본의 검증을 분리합니다.

조립 규칙: Concatenate six main fragments with final newlines; restore four leading spaces to every nonempty line of parts2 and5; CMake fence exact

| 원본 파일 | 바이트 | SHA-256 | FNV-1a 64 |
| --- | ---: | --- | --- |
| [original/CMakeLists.txt](original/CMakeLists.txt) | 479 | `70db538671e405f48871fa7f40ada294ed2bcfe6723c8be939c242f6219dab9e` | `7284b757328b1b74` |
| [original/main.cpp](original/main.cpp) | 5105 | `e5959a649eeb386637fa3d62c492f1c6ffa15ce502c5d5bf4053c31e84631822` | `752e71e4867c725b` |

원본 코드와 조립 결과의 해시를 보존합니다.
새 실행 결과는 verification.json으로 분리하며 과거 실행 또는 원본 첨부 ZIP과 동일하다고 주장하지 않습니다.
과거 두 검증 기록은 서로 다른 경로와 실행이며 원래 첨부 ZIP/로그의 바이트 동일성은 확립되지 않았습니다. 코드 조립 해시 일치와 별개입니다.

[주석 변경 명세](annotation-manifest.json) · [새 CPU 실행 검증](verification.json)

공개 실행 로그는 내부 경로를 `<source>` / `<build>`로 치환한 파생본이며 원시 로그와 바이트 단위로 같지 않습니다. 검증 JSON의 로그 SHA-256은 이 공개 파생본을 가리킵니다.
