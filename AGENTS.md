# ATLAS NOTE — React 작업 규칙

실제 UI는 `web/src`의 React + TypeScript입니다. `README.md`, `web/README.md`, `automation/README.md`, `automation/publishing.json`을 먼저 읽으세요.

## 콘텐츠 예약 작업
뉴스와 언어는 기존 원본 data JSON을 사용합니다. 사고력은 하루 한 주제의 기본 `X.Y.1`·심화 `X.Y.2` 두 독립 페이지를 한 묶음으로 발행합니다. 신규 원본은 `thinking-lab/ep-XX-YY-slug/content.json`, `thinking-lab/ep-XX-YY-02-slug/content.json`과 `thinking-lab/data/catalog.json`만 변경합니다. 기존 기본 URL·메모·날짜를 보존하며 심화 메모는 독립 키로 둡니다. 다양한 알고리즘·논리·일상·조합/비코딩 문제를 구성하고 게임 배경을 강요하지 않습니다. 풀이에는 이해를 돕는 그림을 넣고 기술적인 코드 입력 조건은 접습니다. 각 episode에 category와 1~8개의 중복 없는 주제 tags를 반드시 기록합니다. 발행 시점 원본과 오늘 자료를 읽어 중복·미래 발행을 피합니다. 언어 카탈로그/진도는 `language-study/scripts/reconcile.py`가 관리하므로 예약 작업이 직접 편집하지 않습니다. AI 실험/오픈소스 분석은 수동입니다.

원본 날짜·기사 순서·기존 localStorage 키·하위 URL·연구 검증 기록을 보존합니다. 기존 `_content.html`은 빌드 입력으로 남겨두되 새 독립 HTML UI를 작성하지 않습니다. 문제의 설명·힌트·코드·테스트는 typed content blocks로 작성하고 정답은 details 블록 안에 둡니다. HTML/CSS/JS로 UI를 새로 만들지 않습니다.

`index.html`, 모든 하위 `index.html`, `assets/react/`, `data/atlas/`, `language-study/sw.js`, `build-info.json` 및 `_maintenance/react-generated-files.json`은 자동 생성 결과입니다. 예약 작업에서 작성하거나 이전 화면으로 덮어쓰지 마세요. catalog에 등록된 경로는 빌드가 생성합니다.

## UI 및 코드 수정
공통 메뉴/테마는 `web/src/components/Shell.tsx`, 디자인 토큰은 `web/src/styles.css`에서 관리합니다. 모든 페이지는 메인과 같은 dark 테마이며 영역별 레이아웃은 다릅니다. 문제 검색/태그 필터는 `DocumentLibrary.tsx`와 `lib/document-library.ts`에 있습니다. 공통 UI는 `components/ui`, 데이터 타입은 `lib/types.ts`, 변환은 `scripts/content.ts`에서 수정합니다. DOM 재배치나 HTML 문자열 주입으로 회귀하지 않습니다.

`cd web && npm ci && npm run check`로 저장소·발행 계약·타입·데이터·기능·화면 검사를 실행합니다. 일반 콘텐츠 발행에서는 소스 컴포넌트·스타일·워크플로·스냅샷을 수정할 수 없습니다. UI 변경 요청인 경우만 후보 diff를 검토해 기준 이미지를 변경합니다. 검사를 끄거나 허용 오차를 높여 실패를 숨기지 마세요.

React Pages validation and publication의 성공 및 공개 URL 검증을 확인합니다. 커밋, 테스트, 배포를 구분해 보고하고 실패 시 오류를 그대로 알립니다. 최신 부모 SHA를 확인하며 force push하지 않습니다. 사용자 localStorage는 삭제하지 않고 언어 캐시 정리 범위는 language-study-*로 제한합니다.

## 파일 정리
`web/dist`, `web/public`, node_modules, 테스트 보고서, 임시 이식 파일은 추적하지 않습니다. 승인된 스냅샷, package-lock, 사용 중인 원본과 Pages 배포 파일은 보존합니다. `old/`·`backup/` 복제 폴더 대신 Git 이력을 사용합니다.

## 게임 기획 이미지 발행
게임 기획 예약 작업은 `game-design/data/catalog.json`, 회차 `content.json`, 회차별 `assets/` 아래 안전한 SVG/PNG/WebP/JPG만 작성합니다. 매 강의의 난점에 실제로 도움이 되는 설명 그림(시간 축·규칙 비교·시스템 관계)을 가능한 1~3개 직접 제작해 `image` 블록의 `src`에 `/game-design/ep-.../assets/...` 형식으로 등록하고, 한글 대체 텍스트와 캡션을 반드시 제공합니다. 외부 이미지/영상의 저작권을 준수하며 불확실한 스크린샷을 퍼오지 않습니다. UI 소스 및 자동 생성물은 예약 작업에서 수정하지 않습니다.
