# DX9 렌더링 파이프라인 01–05 본문 이관 · 2026-10-19

원본: https://app.notion.com/p/3e66577f53a549fe99a94ac26f68a685

- Direct3D 9 학습 문서 다섯 개의 텍스트·표·코드 조각·Mermaid 원본·접힌 문답을 GitHub Pages용 typed JSON으로 변환했습니다. `source-notion.md`에 각 Notion 원문을 보관합니다.
- 다섯 자식 문서에는 이미지·파일·미해석 블록이 없었습니다. 사용자 라이브러리의 `dx9_pipeline_sources_v1.md`로 **전체 CPU 실습·HLSL 참고·CMake 등 12개 파일**을 복원했으며 파일별 Git blob이 원본과 일치합니다. [소스·새 CPU 검증 원장](SOURCE_RESTORE_AND_CPU_20261019.md)을 참고하세요. 원본 `dx9_pipeline_lab_v1.zip` 바이너리와 Notion 검증 첨부는 아직 복구·바이트 검증하지 않았습니다.
- 노트의 296 논리 조건은 **과거 Notion 기록**입니다. GitHub 이관 후에도 Linux GCC C++20과 Clang C++20 ASan·UBSan에서 5/5개의 CPU 실행 대상을 새로 CMake/CTest로 검증해 통과했으며, [새 원시 CI 로그](verification/shared/)는 구분해 보관했습니다. 동일한 검사 조건을 컴파일러마다 새로운 독립 조건으로 중복 집계하지 않습니다.
- Windows/MSVC·D3D9·GPU, 셰이더 실기기 검증 미실시. Mermaid 및 LaTeX는 복사 가능한 원본 코드로만 보존합니다.

| 이름 | 원본 | 절 수 | 코드/수식 블록 | 표 | 접힌 문답 | 원문 문자 수 |
|---|---|---:|---:|---:|---:|---:|
| dx9-pipeline-01 | https://app.notion.com/p/3f8a99703f408146bf8fd6f6f9024a0d | 8 | 4 | 3 | 2 | 7147 |
| dx9-pipeline-02 | https://app.notion.com/p/3f8a99703f40817a8aabd25d7443f2ba | 9 | 8 | 2 | 2 | 6580 |
| dx9-pipeline-03 | https://app.notion.com/p/3f8a99703f4081b98c00f5601b7d0c54 | 9 | 4 | 2 | 2 | 7000 |
| dx9-pipeline-04 | https://app.notion.com/p/3f8a99703f408106aba0d3b6a9b101b0 | 10 | 4 | 2 | 2 | 7163 |
| dx9-pipeline-05 | https://app.notion.com/p/3f8a99703f408105948ddf79d5139bb1 | 9 | 4 | 4 | 2 | 7585 |
