# C++ 기초 04~05 · 시각 교재와 전체 실습 소스

작업 기준일: 2026-10-21. 이전 01~03 배치의 원장은 [별도 보존](CPP_FOUNDATIONS_01_03_20261021.md)한다.

## 작업 범위와 이어지는 순서

사용자가 승인한 시각 교재 형식으로 Notion 기초 과정의 마지막 두 단원을 구성한다. Notion은 읽기 전용으로 사용한다.

| 단원 | 교재 경로 | 전체 한국어 주석 소스 |
| --- | --- | --- |
| 04 배열·문자열·사용자 타입·표준 도구 | [C++ 기초 04](https://jiwhan112.github.io/um.github.io/knowledge/cpp-basics-04/) | `containers.cpp`, `buffer.cpp` |
| 05 파일·분할 빌드·디버깅·통합 실습 | [C++ 기초 05](https://jiwhan112.github.io/um.github.io/knowledge/cpp-basics-05/) | `inventory.hpp`, `inventory.cpp`, `inventory_main.cpp` |

기존 75개 카탈로그에 두 본문을 추가하는 배치다. 단원 본문은 기초 01~05의 5개이며, 원본 프로그램·한국어판·그림·검증 로그를 각각 새 단원으로 세지 않는다. 사이트맵의 미집필 과정도 문서 수에 포함하지 않는다.

이 배치의 다음 원본은 **[C++ 심화 01 · 클래스·불변식·생성·소멸](https://app.notion.com/p/3f8a99703f4081e19a90c9b9c7e38f7e)**이다. 심화 과정 목차도 읽기 전용으로 재조회해 순서를 확인했다.

## 본문·원본 무결성

각 `source-notion.md`에는 최신 fetch가 반환한 enhanced Markdown의 `<content>` 내부 본문과 마지막 LF를 저장했다. 조상 페이지 wrapper는 제외했다. 페이지 수정 시각, 응답 기준 시각, 바이트 수와 해시는 각 capture JSON에 기록한다. 도구가 제공하지 않은 `truncated`·`unknown_block_count`는 `null`이며, 알 수 없는 블록이 0개라고 주장하지 않는다.

| 원본 | UTF-8 바이트 | SHA-256 |
| --- | ---: | --- |
| 04 본문 | 19,017 | `efe77af7793c39f488d6005466d16aebb688fdefc0c86fd883bcb03a0fa5acea` |
| 05 본문 | 17,143 | `f18ff42031d09b08856aba5f6448f87448729a0ed8759f76e7a64bfed3b09b33` |
| 공통 실습 첨부 Markdown | 19,602 | `c48d34d0e978b43f3e1b81d86e1e902052e6c5f8536c6b372c2fae8a4d2ed761` |

기초 목차 본문은 이전 보존본과 바이트가 같았다. 공통 첨부도 다시 다운로드해 기존 파일과 바이트 일치를 확인했다. 원본 첨부의 16개 C++·헤더·CMake·부정 예제 파일과 이전 CI 원시 로그는 변경하지 않았다.

새 한국어 주석판은 설명 주석과 읽기 쉬운 줄 배치만 추가한다. [무결성 검사기](shared/cpp-basics-lab/verify_sources.py)는 전체 8개 소스·헤더에서 전처리 지시문과 매크로 확장 전 C++ 토큰을 비교한다. `__LINE__`에 따른 진단 위치는 각 파일의 실제 줄 번호를 따르며, 이 검사는 모든 C++ 프로그램의 의미 동등성을 증명하는 일반 검사기가 아니다.

| 이번에 추가한 파일 | 코드 토큰 수 | 정적 CHECK 호출 지점 |
| --- | ---: | ---: |
| `ko/containers.cpp` | 764 | 23 |
| `ko/buffer.cpp` | 434 | 5 |
| `ko/inventory.hpp` | 87 | 0 |
| `ko/inventory.cpp` | 565 | 0 |
| `ko/inventory_main.cpp` | 728 | 19 |

파일별 바이트와 SHA-256은 [source-integrity.json](shared/cpp-basics-lab/source-integrity.json)에 보관한다. 전체 소스는 각 교재의 접기 블록과 실제 파일 링크에서 같은 내용으로 읽을 수 있다.

## 학습 설명의 범위

04는 저장 공간의 소유자와 뷰, `size`와 `capacity`, 재할당과 무효화, 2차원 인덱스, 문자열 코드 단위, 사용자 타입, 정렬 비교자, 난수 엔진과 분포를 연결한다. `IntBuffer`의 두 배 성장 정책과 `std::vector`의 표준 보장은 구분한다. 난수 범위 검사나 고정 시드 재현을 통계적 균등성·모든 라이브러리의 같은 분포 수열 보장으로 확대하지 않는다.

05는 현재 작업 디렉터리, 스트림 오류, 파일 자원 수명, INV1 포맷, 전체 검증 후 로드 결과 반환, 분할 빌드·링크·디버깅을 연결한다. `Load`의 실제 처리 순서와 `std::quoted`의 비인용 토큰 허용을 설명한다. 저장 함수의 스트림 상태 검사, 호출자의 `close` 검사, 원자적 교체, 내구성은 서로 다른 보장이다. 예제는 마지막 두 보장을 구현하지 않는다. 파일 복사 검사는 크기 일치를 확인하며 바이트 전체 비교를 수행한 것으로 표시하지 않는다.

## 현재 실행 근거

로컬 GCC 13.3.0 C++20 직접 빌드에서 원본 6개와 한국어판 6개가 모두 통과했고, 같은 작업 디렉터리에서 얻은 stdout 6쌍은 정규화 없이 바이트가 일치했다. 컴파일 거부 4개와 엄격한 C++11 원본 6개 거부 진단도 기록했다. [로컬 원시 출력 41개와 해시 원장](shared/cpp-basics-lab/verification/local-gcc-04-05-20261021/README.md)을 보존한다. 로컬에는 Clang·CMake·CTest가 없어 해당 구성은 아래 GitHub CI에서 별도로 실행했다.

웹 단위 검사 28/28, TypeScript·React production build, 저장소·발행 계약, 152개 직접 진입 경로와 내부 링크 검사가 통과했다. 첫 단위 검사에서 05의 표 안 링크를 문단으로만 찾는 테스트 오류를 발견해, 표 셀을 포함한 콘텐츠 링크를 재귀 검사하도록 수정했다. 전체 코드와 배포 파일의 바이트 대조는 유지했다. 새 [C++ CI 실행 39547021595](https://github.com/jiwhan112/um.github.io/actions/runs/39547021595)은 커밋 `10a5e43c015231d326e26a5b16ba39887a64aa84`에서 다섯 작업 모두 성공했다. GCC 13.3.0 및 Clang 18.1.3 + ASan/UBSan, CMake 3.31.6에서 각각 Configure·Build와 **정상 CTest 12/12**, 원본·한국어판 stdout 6쌍 일치를 확인했다. 컴파일 거부 4개를 각 컴파일러에서, GCC C++11 원본 6개 거부를, Clang의 의도적 오류 2개 진단을 정상 CTest와 분리해 확인했다.

GitHub artifact 5개의 ZIP SHA-256을 대조한 뒤 원시 파일 **67개, 185,553바이트**를 보관했다. [새 실행 원장](shared/cpp-basics-lab/verification/github-39547021595/README.md)과 [artifact·파일별 해시 manifest](shared/cpp-basics-lab/verification/github-39547021595/artifact-manifest.json)에서 추적한다. 이전 [39532381043 실행](https://github.com/jiwhan112/um.github.io/actions/runs/39532381043)의 9/9 결과는 원본 6개와 당시 한국어판 3개에 대한 과거 근거이며, 이번 한국어판 전체 6개의 새 실행 결과로 재사용하지 않는다.

같은 소스 커밋의 [웹 CI 실행 39547021478](https://github.com/jiwhan112/um.github.io/actions/runs/39547021478)은 **PC·모바일 421개 통과, 의도적 제외 1개, 실패·불안정 재시도 0개**로 완료됐다. 모든 교재·탭의 Mermaid 86개를 렌더링했으며, 모바일의 전체 Mermaid 중복 검사 1개만 제외하고 모바일의 개별 도식·배치·상호작용 검사는 별도로 수행했다. [PR 웹 검사 39547072660](https://github.com/jiwhan112/um.github.io/actions/runs/39547072660)도 성공했다.

웹 artifact ZIP의 SHA-256(`2c781cdfb09bf896b98be3987b4dfcd54c1e8ad61ac3238f7aaf05ecb63dcc5b`)을 확인하고, 04·05 각각의 PC·모바일 첫 화면·전체 본문·한국어 전체 소스 캡처를 교차 검토했다. SVG 6개와 본문 Mermaid 4개의 라벨·분기·화살표, 표의 줄바꿈, 코드 상자 내부 스크롤을 확인했다. 04 첫 소제목의 모바일 한 글자 줄바꿈은 제목을 줄여 개선했고, 05 번역 단위 SVG의 왼쪽 두 화살촉은 결합 경로를 나눠 수정했다. 이 두 수정과 검증 근거 추가 뒤에는 최종 커밋의 CI 결과를 다시 확인하고 병합한다.

## 검사 수와 검증 한계

- 원본 프로그램의 런타임 CHECK 횟수는 `types` 15, `flow` 15, `memory` 19, `containers` 49, `buffer` 104, `inventory` 19다. 01~03은 49회, 04~05는 172회, 전체는 221회다.
- 반복문에서 같은 호출 지점이 여러 번 실행된다. 정적 호출 지점 수, 런타임 횟수, 독립 기능 테스트 수를 혼동하지 않는다. 원본·한국어판·GCC·Clang의 반복 실행은 새 독립 조건이 아니다.
- 실습 팩은 C++20 기준이며 C++11 호환판이 아니다. 엄격한 C++11 컴파일 거부 진단은 호환 빌드 성공과 구분한다.
- 컴파일 거부 4개와 의도적 sanitizer 진단 2개는 정상 CTest 프로그램에서 분리한다.
- Windows/MSVC, Direct3D·GPU·HLSL 실행은 미검증이다. 이 배치는 해당 환경의 실행 증거를 추가하지 않는다.
- 과거 ZIP 및 과거 원시 실행 로그는 회수하지 못했다. 원본 Notion에 기록된 2026-10-12 실행 요약과 새 GitHub CI 원시 로그를 분리해 보관한다.

## 보존과 공개 배포 판정

기존 React 화면·저장 키·Supabase 설정과 브리핑·언어·사고력·게임 기획 자료는 변경하지 않는다. 기존 C++ 01~03 및 모든 공개 교재 경로를 유지한다. Notion 원문과 예약 작업도 수정하지 않는다.

커밋, PR 검사, master 병합, React Pages 검증·게시, 실제 공개 URL/JSON/소스 바이트 대조는 서로 다른 단계다. 교재 파일이나 PR이 존재한다는 사실만으로 공개 배포를 완료했다고 판단하지 않는다. 최종 배포 상태는 병합 커밋의 Actions와 실제 `build-info.json`의 `sourceCommit`을 함께 확인한다.
