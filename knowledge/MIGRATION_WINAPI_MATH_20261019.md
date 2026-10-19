# Windows·그래픽스 프로그래밍 — 첫 본문 이관 (2026-10-19)

- 범위: WinAPI 01–04·04A, 게임 수학 01–05. 기존 DX11·LIS·Python 3개 시범 교재 원본은 건드리지 않음.
- 이관 방식: 연결된 Notion 읽기 도구에서 해당 페이지 10개의 원본 본문을 조회하고, GitHub `source-notion.md`에 원문을 보존. React `content.json`에 문단·코드·Mermaid 원본 코드·표·접힌 Q&A를 변환.
- 각 문서의 원본 URL은 `knowledge/data/catalog.json`과 서두에서 확인. 최종 상태는 GitHub 커밋/CI/라이브 검사에 따라 별도 보고.
- 제한: WinAPI/게임 수학 설명에 등장하는 공통 `game_math_winapi_lab_v1.zip`의 전체 바이너리·GUI/MFC 소스는 Notion 첨부에서 확보되지 않아 아직 이전 완료라고 표기하지 않음. Win04A CPU `main.cpp` 4701B와 `CMakeLists.txt` 543B는 연결된 개별 네이티브 원본에서 FNV64와 길이를 대조해 저장함.
- WinAPI/게임 수학 하위 10개 본문에는 Notion 이미지·파일 블록이 없고, 미해석 블록도 보이지 않았음. 부모 개요 페이지의 텍스트 첨부는 이관 대상 범위로 기록하되 실제 파일 바이트 검증 전까지 미완료.
- Mermaid 그래프는 현재 React의 code 블록으로 원본을 제공(그림 자체의 자동 렌더링 보장 안 함). LaTeX 원본도 code 블록으로 제공. 향후 표시 기능 개선 가능.
- 검증 구분: 본문 마이그레이션 ≠ CPU 새 실행 ≠ Windows/MSVC/D3D/GPU 검증. 

## 조회·변환 원장

| GitHub 경로 | Notion 페이지 | 절 | 코드 블록 | 표 | Mermaid | 접기 | 원문 문자 수 |
|---|---|---:|---:|---:|---:|---:|---:|
| winapi-01 | https://app.notion.com/p/3f8a99703f408108847ded19eeeed81f | 9 | 4 | 1 | 1 | 2 | 5596 |
| winapi-02 | https://app.notion.com/p/3f8a99703f408137ac5ed11df7858a5f | 8 | 2 | 0 | 1 | 2 | 4947 |
| winapi-03 | https://app.notion.com/p/3f8a99703f4081a59040d64ecde49946 | 9 | 2 | 1 | 1 | 2 | 4947 |
| winapi-04 | https://app.notion.com/p/3f8a99703f40812fbfd4e2c48de2e58b | 10 | 2 | 4 | 1 | 3 | 8323 |
| winapi-04a | https://app.notion.com/p/3fda99703f4081b18c16d94ceb844473 | 7 | 2 | 1 | 1 | 3 | 5200 |
| game-math-01 | https://app.notion.com/p/3f8a99703f40815c894affc335c2e205 | 8 | 6 | 1 | 1 | 2 | 5080 |
| game-math-02 | https://app.notion.com/p/3f8a99703f40811686bfe7d95d9bd027 | 8 | 5 | 1 | 1 | 2 | 4885 |
| game-math-03 | https://app.notion.com/p/3f8a99703f40814e905fe501f75cddc9 | 8 | 6 | 1 | 1 | 2 | 4872 |
| game-math-04 | https://app.notion.com/p/3f8a99703f4081b1b3adeacf535ee6da | 10 | 2 | 1 | 1 | 2 | 5579 |
| game-math-05 | https://app.notion.com/p/3f8a99703f40814bb111e208530bf6b7 | 9 | 4 | 1 | 2 | 2 | 5340 |
