# DX9 구현 01–12 · 학습 본문 이관 원장 (2026-10-19)

Notion 원본 대표 페이지: https://app.notion.com/p/bdcccf3ce6d34930827bf0a519b4fa78

- 각 단원 전체 본문을 `source-notion.md`로 보관하고 표·접기·Mermaid·코드 블록은 `content.json`으로 변환했습니다.
- DX9 03/04는 독립 Notion 페이지가 아닌 **원본 대표 페이지 끝의 개정 섹션**을 고유 시작 문구 기준으로 정확하게 분리했습니다. 앞선 레거시 메모 50K 문자는 본문으로 중복 발행하지 않고 원본 Notion에 보존했습니다.
- D3D9/MSVC/GPU 실기기 검증은 **수행하지 않았으며**, 기존 Notion 기록의 GCC/Clang 검증은 신규 GitHub CI 검증과 구별합니다.
- 문서의 미해석 블록은 없었습니다. 원본 file 블록은 실제 바이너리 파일·해시 확인 없이 가용하다고 간주하지 않으며 본문에 첨부 미회수 안내를 표시했습니다. 09·10·11 및 01 등에 남아 있습니다.
- 직접 소스 보관본 01A·02A·03A·04A·05A·06A·07B·11A의 전체 파일은 GitHub의 독립 C++/CMake 소스 파일과 별도 전체 소스 문서로 저장하고 관련 장의 말미에 링크했습니다. [새 CPU 테스트 21개 실행 구성](DX9_IMPLEMENTATION_CPU_VERIFICATION_20261019.md)의 원시 GitHub Actions 로그도 21개 영구 보관했습니다. **대상 별 Windows GUI/Direct3D9/GPU와 원본 첨부 바이너리는 여전히 미검증**으로 둡니다.

| 단원 | Notion 원본 ID | 원문 글자 | 렌더 절 수 | 코드 블록 | 접힌 문답 | 미회수 file 블록 |
|---|---|---:|---:|---:|---:|---|
| dx9-impl-01 | 3f8a99703f408135a849f7c66e4d54ad | 5547 | 12 | 2 | 3 | dx9_com_owner_h.txt, dx9_com_owner_tests_cpp.txt, dx9_device_validation.txt |
| dx9-impl-02 | 3f8a99703f408172837fe0d04c131e68 | 4898 | 9 | 5 | 0 | - |
| dx9-impl-03 | bdcccf3ce6d34930827bf0a519b4fa78 | 5483 | 11 | 0 | 3 | - |
| dx9-impl-04 | bdcccf3ce6d34930827bf0a519b4fa78 | 5750 | 9 | 3 | 3 | - |
| dx9-impl-05 | 3f8a99703f408171ab68e3ed5b265f01 | 2035 | 6 | 1 | 0 | - |
| dx9-impl-06 | 3f8a99703f40817cbe12e85790420aab | 3142 | 6 | 1 | 2 | - |
| dx9-impl-07 | 3f8a99703f4081e3b908de685a551c23 | 5838 | 8 | 1 | 3 | - |
| dx9-impl-08 | 3f8a99703f40811ca79ae3e8a74806d7 | 12064 | 9 | 6 | 4 | - |
| dx9-impl-09 | 3f8a99703f4081ba8b8ae2fc6650bde2 | 15285 | 49 | 35 | 2 | dx9_09c_integrated_cpu_full_sources.md |
| dx9-impl-10 | 3f8a99703f408155a685d66590c67b7a | 7093 | 9 | 5 | 3 | dx9_shader10_cpu_main.txt |
| dx9-impl-11 | 3fba99703f40816eb333f270aad46a46 | 13356 | 13 | 4 | 5 | dx9_frustum11_v1_1_main.cpp.txt, dx9_frustum11_v1_1_build_and_validation.md |
| dx9-impl-12 | 3fba99703f4081bfbad0d047f8a8d58f | 20480 | 10 | 6 | 2 | - |
