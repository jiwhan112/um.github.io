# DX11 13–18 · CPU 실행·원본 무결성 검증 원장

> **2026-10-20 재감사 정정:** 아래 최초 실행의 GCC11 job은 Font/Deferred에 사용되지 않는 `LAB_CXX_STANDARD` 변수를 전달했습니다. 두 프로그램의 당시 실행은 C++11 근거가 아닙니다. 새 검사에서는 원본별 표준 설정을 적용하고 실제 컴파일 명령의 `-std=` 옵션을 확인합니다. [정정 사유·재감사 범위](../../MIGRATION_DX11_13_18_AUDIT_20261020.md).

## 최초 독립 GitHub CPU 실행 원본 (2026-10-20)

[GitHub Actions #39351876445](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445)에서 복원된 GitHub 파일을 독립 컴파일하고 CTest 실행한 **실제 GitHub Actions 전체 Job 로그** 세 개를 여기에 보존합니다.

| 환경 | CTest | 실행 논리 조건 |
|---|---|---|
| GCC C++20 | 13 / 13B / 14 / 16 / 17 / 18 각 1/1, 합계 6/6 | 37+21+31+124+49+41 = **303** |
| 기존 gcc11 job | 6개 프로그램 각각 1/1. **17/18은 실제 C++20**, 나머지 4개만 C++11 | 같은 303조건 반복; 표준 혼합 실행 |
| Clang C++20 ASan/UBSan | 같은 6개 프로그램 각각 1/1 | 같은 303조건 반복 |

- [gcc20 작업 #118397712741](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445/job/118397712741) · [전체 출력 원문](./gcc20_run39351876445.raw.log)
- [gcc11 작업 #118397712659](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445/job/118397712659) · [전체 출력 원문](./gcc11_run39351876445.raw.log)
- [clangsan 작업 #118397712539](https://github.com/jiwhan112/um.github.io/actions/runs/39351876445/job/118397712539) · [전체 출력 원문](./clangsan_run39351876445.raw.log)

이 로그는 편집된 테스트 요약이 아니라 GitHub Actions 콘솔 전체 출력입니다. 기록된 테스트 성공은 Windows/MSVC·D3D11/GPU/HLSL 실기기 실행이 아니며 역대 원본 소스의 SHA 동일성을 입증하지 않습니다.

## 15 Frustum CPU 포함 최종 재검증 · 2026-10-20

[GitHub Actions #39353407595](https://github.com/jiwhan112/um.github.io/actions/runs/39353407595) — **7개 프로그램 × 3개 환경 모두 성공**.

| 프로그램 | 단일 환경별 고유 논리 조건 |
|---|---:|
| 13 Instancing | 37 |
| 13B InputLayout 경계 | 21 |
| 14 Geometry Shader CPU | 31 |
| 15 Frustum 기존 소스(미수정) | 33 |
| 16 QuadTree | 124 |
| 17 Font v2 | 49 |
| 18 Deferred MRT v2 | 41 |
| **합계** | **336** |

과거 세 job은 **같은 336개 조건을 반복**했습니다. gcc20·clangsan은 요청한 표준을 사용했지만, gcc11 job의 17/18은 C++20 기본값으로 실행되었습니다. 나머지 5개만 C++11입니다. 이를 1,008개의 독립 조건으로 보고하거나 17/18의 과거 C++11 검증 근거로 사용하지 않습니다.

- [gcc20 Job #118402619650](https://github.com/jiwhan112/um.github.io/actions/runs/39353407595/job/118402619650) · [전체 원시 출력](./gcc20_run39353407595.raw.log)
- [gcc11 Job #118402620017](https://github.com/jiwhan112/um.github.io/actions/runs/39353407595/job/118402620017) · [전체 원시 출력](./gcc11_run39353407595.raw.log)
- [clangsan Job #118402619343](https://github.com/jiwhan112/um.github.io/actions/runs/39353407595/job/118402619343) · [전체 원시 출력](./clangsan_run39353407595.raw.log)

해당 CI는 13A의 **현행 SHA-256** `ef9b0cfe2e764adc896a45451f5607fd751ab0c8b523e6477339994ba758a4b8` 검사를 포함해 통과했습니다. **2026-10-16의 역사적 원본 SHA-256** 불일치는 변경·해결되지 않았습니다.

## Notion 원문·과거 실행 로그 추가 보존

[무결성 명세](source-integrity.json)는 최신 조회한 26개 Notion 페이지와 99개 보존 파일을 기록합니다. 67개 파일은 코드 블록 인덱스에 따라 원문에서 그대로 재조립해 검사합니다. 각 블록의 마지막 LF도 파일 바이트에 포함합니다. `python3 knowledge/scripts/verify-dx11-13-18.py`로 저장소 루트에서 다시 검사할 수 있습니다.

| 자료 | 원시 로그 수 | 저장 위치 |
|---|---:|---|
| 13A 현행 6043B 소스의 과거 재현 실행 | 9 | [13a-current](notion-history/13a-current/) |
| 13B compact 13조건 과거 실행 | 9 | [13b-compact](notion-history/13b-compact/) |
| 13B two-pass 21조건 과거 실행 | 9 | [13b-two-pass](notion-history/13b-two-pass/) |
| 14–16 공통 과거 CTest | 3 | [14-16-shared](notion-history/14-16-shared/) |
| 17 Font v2 과거 실행 | 9 | [dx11-17](notion-historical/dx11-17/) |
| 18 Deferred v2 과거 실행 | 9 | [dx11-18](notion-historical/dx11-18/) |
| 합계 | **48** | 당시 Notion stdout, 이번 실행과 구분 |

13A·13B의 27개 출력은 원문에 기록된 바이트 수·FNV64와 모두 일치합니다. Font/Deferred 원문에 명시된 로그 지문도 일치합니다. 지문이 없는 기록을 새 실행 결과로 대체하지 않았습니다. Windows/MSVC·D3D11/GPU·HLSL 실행과 역사적 13A 원본 바이트 확인은 계속 미검증입니다.

## 재감사 실행의 판정 기준

- 대상: 기존 7개 주 실습과 별도 13B compact, 총 8개 CPU 실행 파일.
- 환경: GCC C++20 / GCC C++11 / Clang C++20 ASan·UBSan.
- 실제 `compile_commands.json`에서 언어 표준·sanitizer 플래그를 확인하며 Configure·Build·CTest 원시 출력을 개별 저장합니다.
- 주 실습 336조건과 compact 13조건을 구분합니다. compact에는 2-pass와 겹치는 조건이 있으므로 349개 독립 조건이라고 보고하지 않습니다.
- 로컬 GCC 13.3 직접 빌드에서는 `-std=c++20`과 `-std=c++11` 각각 8개 프로그램 실행에 성공했습니다. 이는 CMake/CTest·Clang 실행을 대체하지 않습니다.
- 새 GitHub 실행 결과는 검증한 run/commit ID와 함께 아래에 추가합니다.
