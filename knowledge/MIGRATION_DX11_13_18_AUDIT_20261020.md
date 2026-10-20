# DX11 13–18 이관 재감사와 누락 자료 보완

감사일: 2026-10-20 KST. 기준 `master`: `f966cd4becfd73d050a01f052a7579bff26392b2`.

## 재개 시점의 실제 상태

요청서의 마지막 기록은 60개 문서였지만, 재조회한 저장소에는 [PR #10](https://github.com/jiwhan112/um.github.io/pull/10)이 이미 병합되어 있었다. 기존 작업 브랜치 `feature/dx11-13-18-knowledge-20261020`의 HEAD는 `b2d1a69cd46d0b5938fb5b0f3d3e16668f1b1051`, squash 병합은 `4d4981098807979d20a53521e7da2b75a5db7dd2`이다. 작업 브랜치와 master의 소스는 같고 이후 배포 생성물만 달랐다. 따라서 기존 브랜치를 덮어쓰거나 12개 문서를 중복 생성하지 않았다.

보완 브랜치는 최신 master에서 분기한 `feature/dx11-13-18-audit-20261020`이다. 학습 카탈로그는 **72개**로 유지한다. 범위는 13–18의 여섯 단원과 13B 보충 자료이며, 15 Frustum의 기존 경로를 그대로 사용한다.

## 확인한 누락과 실제 보완

| 종류 | 보완 내용 |
|---|---|
| Notion 본문 대조 | 본문·소스·하위 검증 문서 26개. 원문을 수정하지 않고 현재 보관본과 비교 |
| 누락 하위 원문 | 13A 재현 원장, 13B compact 로그·2-pass 설명·3환경 로그, 17A/E/F, 18A 총 10개 |
| 과거 실행 출력 | 13A 9개, 13B compact 9개, 13B 2-pass 9개, 14–16 공통 CTest 3개, Font 9개, Deferred 9개: 총 48개 |
| 누락 실행 소스 | 13B compact `main.cpp` 2,840바이트 및 원본 CMake 616바이트. 기존 21조건 2-pass 구현과 별도 보관 |
| HLSL 예제 파일 | 13 Instancing 548바이트, 18 Geometry Pass 502바이트. 교육용 예제로 보존하며 HLSL 컴파일은 미실행 |
| 원본 바이트 차이 | C++·헤더·CMake·HLSL 8개에서 빠진 마지막 LF 1바이트씩 복원. 알고리즘 변경 없음 |
| 15 Frustum | 원문·5,381바이트 C++ 일치. 기존 감사 안내의 모호한 문자 수를 정확한 비교 규약으로 교정 |
| 재현 가능한 무결성 | [source-integrity.json](verification/dx11-13-18/source-integrity.json)에 99개 파일의 바이트 수·SHA-256·원문 경로·블록 인덱스 보관. 67개 파일은 원문 코드 블록에서 재조립해 대조 |

과거 출력의 원본 텍스트에는 원래 실행 경로와 당시 컴파일러 버전이 그대로 남는다. 이를 이번 GitHub Actions 실행이라고 표시하지 않는다. 17A는 전체 29조각 중 현재 조회 가능한 일부만 있고, 18A는 역사적 전체 `main.cpp`가 없음을 원문에서 명시한다. 두 문서는 불완전한 과거 자료로 보존하며 현행 Font/Deferred v2와 구분한다.

## 마지막 LF를 복원한 파일

| 파일 | 이전 → 원문 일치 바이트 |
|---|---:|
| `shared/dx11-13/CMakeLists.txt` | 512 → 513 |
| `shared/dx11-14/geometry_sprite.hlsl` | 698 → 699 |
| `shared/dx11-14/CMakeLists.notionshared.txt` | 626 → 627 |
| `shared/dx11-17/fontlab.hpp` | 3,776 → 3,777 |
| `shared/dx11-17/main.cpp` | 3,735 → 3,736 |
| `shared/dx11-17/CMakeLists.txt` | 432 → 433 |
| `shared/dx11-18/main.cpp` | 6,549 → 6,550 |
| `shared/dx11-18/CMakeLists.txt` | 456 → 457 |

13A CMake의 복원 SHA-256과 Font/Deferred v2 다섯 파일의 원장 FNV64가 원문 기록과 일치한다. 이는 **13A 과거 main.cpp SHA 불일치가 해결됐다는 뜻이 아니다.** 현행 main.cpp는 6,043바이트, SHA-256 `ef9b0cfe2e764adc896a45451f5607fd751ab0c8b523e6477339994ba758a4b8`로 기존과 동일하다. 과거 `fd3839f7c350d1e573cab36a8c3f942f55a8e6c9d4e94b45033e7d3b3b4472ae`의 원본 바이트는 여전히 없다.

## 기존 CI의 C++11 표기 정정

기존 워크플로는 모든 프로그램에 `-DLAB_CXX_STANDARD=11`을 전달했다. 그러나 Font/Deferred v2 원본 CMake는 `CMAKE_CXX_STANDARD`를 사용한다. [기존 GCC11 원시 로그](verification/dx11-13-18/gcc11_run39353407595.raw.log)에서도 두 프로그램의 `LAB_CXX_STANDARD`가 사용되지 않았다는 CMake 경고를 확인했다. 기본값이 20이므로 **이 두 프로그램에 대한 기존 GitHub GCC11 job 성공을 실제 C++11 컴파일 근거로 사용하면 안 된다.** 원시 로그는 고치지 않고 해석을 정정한다.

새 워크플로는 각 원본의 설정 이름을 그대로 따른다. 13B compact에는 `LAB_STD`, Font/Deferred에는 `CMAKE_CXX_STANDARD`, 나머지 프로그램에는 `LAB_CXX_STANDARD`를 전달한다. `compile_commands.json`에서 실제 `-std=c++11`/`-std=c++20`과 Clang sanitizer 옵션을 검사한다. Configure·Build·CTest stdout/stderr를 각각 파일로 저장하고, 실패를 `tee`가 숨기지 않도록 pipefail을 사용한다.

## 테스트 조건 집계

기존 주 실습 7개는 37+21+31+33+124+49+41 = **336개 조건**이다. 추가 compact 실습은 **13개 조건**이지만 2-pass와 검사 목적이 겹치므로 독립 조건 합계를 349개로 확대하지 않는다. GCC C++20, GCC C++11, Clang C++20 ASan/UBSan에서 각 프로그램을 반복 실행한다. 실행 건수는 8개 CTest × 3환경이며, 서로 다른 논리 조건 수와 구분한다.

새 CI 실행 및 공개 배포 증거는 [검증 원장](verification/dx11-13-18/README.md)의 재감사 실행 절에 기록한다. 원본 파일, 커밋, 검증 성공, 실제 공개 배포는 별도 상태로 확인한다.

## 범위와 남은 자료

Windows/MSVC, 실제 Direct3D11 런타임·GPU, HLSL 컴파일, 이미지·성능 실측은 수행하지 않았다. Notion 응답은 `truncated`/`unknown_block_count` 필드를 제공하지 않았다. 조회한 26개 문서는 닫는 태그까지 있고 미확인 블록·이미지·파일 자리표시자가 관찰되지 않았지만, 이를 모든 역사적 첨부가 완전하다는 보증으로 확대하지 않는다.

DX11 상위 원본 노트에는 이미지 첨부와 별도 SIMD 문서가 남아 있다. 현행 개정 단원은 01–18까지이며 확인된 19단원은 없다. 다음 범위는 별도의 **SIMD연산** 원문(`80198348eeb84217ab99fa10f99f21e7`)과 상위 원본 이미지·HLSL 보관함의 이관 대조이다. 이번 배치에서 상위 원문이나 Game Engine Architecture, 예약 작업, 브리핑·언어·사고력·게임 기획·Supabase 저장 구조는 변경하지 않았다.
