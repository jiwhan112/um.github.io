# 이전 Jekyll 상세 UI 규칙 — 사용 중단

React 전환 이후의 작성/유지보수 규칙은 [react-migration.md](react-migration.md)와 루트 AGENTS.md를 따릅니다.

기존 `_content.html` 지문·코드·평가 기록은 빌드 시 타입이 검증된 콘텐츠 블록으로 변환됩니다. `index.html`은 React 빌드가 생성하므로 Jekyll wrapper나 독립 HTML로 작성하지 않습니다. `_includes/atlas-detail.html`, `assets/atlas/detail.js`와 detail.css는 이전 릴리스 자료이며 새 앱에서 사용하지 않습니다.
