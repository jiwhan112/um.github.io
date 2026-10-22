# ATLAS NOTE · C·Effective C++ 교재 완성 예약 — 최신 작업 계약

수정일: 2026-10-22. [진도 원장](data/visual-textbook-progress.json)과 [현재 승인 목차](CURRICULUM.md)를 우선한다. 이 계약은 이전의 전체 프로그래밍 분야 무기한 개편·C 기초 03 이후 피드백 대기·예약 비활성 상태를 대체한다. 과거 기록은 Git 이력에 남겨 둔다.

## 정확한 유한 범위

- 대상: **C 기초 04–14 신규 C11 독립 교재**를 순서대로 완결하고 **Effective C++ 01–06**의 최신 Notion 개정 자료를 읽기 전용 대조하여 독립 시각 교재로 개편한다. 이 17개만 최종 종료 판정에 포함한다.
- 04 조건·반복 / 05 함수·범위·수명 / 06 배열·문자열 / 07 포인터 / 08 배열·포인터·함수 포인터 / 09 구조체·열거형·공용체 / 10 동적 메모리·소유권 / 11 안전 입력·오류 / 12 파일 입출력·직렬화 / 13 전처리·다중 파일 / 14 통합 실습·디버깅.
- Effective 01 항목1–12 / 02 항목13–25 / 03 항목26–31 / 04 항목32–40 / 05 항목41–48 / 06 항목49–55. 목차: https://app.notion.com/p/8178c17f593b4f4e83a0b42544b8fe2b .
- 이미 공개 검증한 C 기초 01–03, C 응용 C01, C++ 기초 01–05 및 심화 01–06은 보존·재작성 금지. C 응용 C01은 정규 11 이후 참고한다. 기존 cpp-advanced-lab 원본·한국어 주석·CPU 기록은 중복 복사·실행하지 않는다.
- WinAPI/게임 수학/DX9/DX11/CS/Modern C++/Effective Modern C++ 및 신규 다른 과정과 Game Engine Architecture 노트는 범위 밖이다.

## 실행 순서

1. master SHA, 전 배치 원장·카탈로그·실제 파일·최종 Actions 및 공개 build-info를 필요한 범위만 확인한다. 성공한 동일 SHA CI와 공개 증거를 재사용한다. 열린 작업/PR/유효 active lease가 있으면 동시 편집하지 않는다.
2. 1회에 품질을 지킬 수 있는 연속 단원을 작성한다. 기존 typed content blocks, **개념/실습/출처/원문 이력 4탭**, 학습용 SVG, 실제 Mermaid 렌더링, 한국어 주석 전체 소스, 표·접힌 답을 사용한다. 원본·검증·미검증을 구분한다. Notion은 읽기만 수행한다.
3. 배치 내용과 연계 파일을 작성 완료한 뒤 **실제 변경한 코드만** 해당 표준의 대표 C11/C++20 빌드·실행 및 필요 sanitizer를 묶어 검증한다. 같은 공유 프로그램, 미변경 단원 및 무관한 DX 코드는 다시 실행하지 않는다.
4. 콘텐츠·원장·목차·카탈로그·관련 테스트를 **최신 master tree 기반 atomic create_tree/create_commit/update_ref(expected_sha)**로 함께 커밋하고 강제 푸시하지 않는다. 공통 UI·워크플로·생성 산출물은 건드리지 않는다. 브랜치 보호가 거부하면 우회하지 않고 허용된 PR 경로를 쓴다. 미완성 원고를 저장할 때만 `textbook-work/` 전용 브랜치를 쓰며 완성 전 카탈로그 등록 금지.
5. master의 기존 `React Pages validation and publication`이 유일한 종합 웹 테스트와 배포 게이트다. 오류가 있으면 변경 항목을 수정한 뒤 해당 실패 검사를 다시 통과한다. 기준 스크린샷·허용 오차·검사 자체를 완화하지 않는다.
6. workflow 성공 후 공개 build-info의 sourceCommit, 새 문서 JSON·전체 코드·그림과 대표 PC/모바일 화면을 확인한다. 단계는 작성/CPU 검증/커밋/CI/Pages 공개를 분리하고, 불명확하면 미검증으로 남긴다. 여러 동등한 전체 경로 순회를 반복하지 않는다.
7. 모든 C04–14 및 Effective01–06의 누락 없는 공개를 확인한 뒤 이 예약 ID `6ad87837d4688191818e709038a6c0c4`만 비활성화한다. 일시적 쓰기 오류는 진도와 재개 지점에 기록하고 다른 예약은 건드리지 않는다.

## 이전 검증된 기준

PR21 C 기초 01–03: source `08724feac20f586112c4fbfc735cb403cd5d5955`, [Pages 검사](https://github.com/jiwhan112/um.github.io/actions/runs/39756113957) success, 공개 166경로/표본 C 관련 25경로, 475 passed·1 intentional skip. 이는 새 교재의 실행 성공을 뜻하지 않는다.

첫 신규 배치 C04–05의 실제 Linux GCC 14.2 C11 strict+UBSan 실행과 SHA-256은 [검증 JSON](shared/c-basics-04-05/verification.json)에 기록한다. 웹 CI/배포 확인 전에는 완료로 표시하지 않는다.

데일리 브리핑·언어·사고력·게임 기획·AI 실험, Supabase 및 진도/북마크/읽음/메모 저장 키는 수정하지 않는다.
