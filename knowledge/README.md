# ATLAS NOTE — 프로그래밍 지식 라이브러리

- [학습 목차·향후 집필 설계 v2](CURRICULUM.md): **사이트맵과 새 교재 집필 계획**을 기록합니다. Notion 이관 지시·완료 원장이 아닙니다.
- [현재 공개 교재 색인](data/catalog.json): DX11 Frustum, 알고리즘 LIS, Python BeautifulSoup 총 3개 시범 교재의 실제 페이지·원본 출처만 관리합니다.

## 기존 프로토타입 보존

2026-10-19에 작성한 시범 교재 3개의 독립 원본은 각 폴더 `source-notion.md`, 렌더링 블록은 `content.json`이며, 실행 예제는 `main.cpp` 또는 `main.py`입니다. **이미 게시된 URL과 내용은 이번 목차 개정으로 변경하지 않습니다.** 특히 Python/알고리즘 시범 교재는 새 커리큘럼의 확정 단원으로 집계하지 않습니다.

향후 실제 새 콘텐츠를 게시할 때에만 `knowledge/data/catalog.json`에 문서를 등록하고, 동일한 구조의 `content.json`과 전체 소스를 추가합니다. 빌드 어댑터가 `data/atlas` 문서 색인과 `/knowledge/` 진입 경로를 생성합니다.

기존 언어·브리핑·사고력 데이터와 localStorage/Supabase 키는 건드리지 않습니다. 과거 Notion CPU 검증은 그 당시의 기록일 뿐, GitHub 이관본의 새 Windows·GPU 검증을 의미하지 않습니다. Notion 이미지가 누락되었을 때 추측해서 대체하지 않으며, GitHub Pages는 공개 사이트입니다.
