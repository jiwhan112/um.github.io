# ATLAS NOTE — React 작업 규칙

이 저장소의 실제 UI는 `web/src`의 React + TypeScript 컴포넌트입니다. 먼저 `web/README.md`와 `_maintenance/react-migration.md`를 읽으세요.

## 예약 콘텐츠 발행
뉴스·언어 학습은 기존 data JSON과 카탈로그만 갱신합니다. 사고력/오픈소스 문서는 회차의 `_content.html` 또는 검증 가능한 `content.json`과 카탈로그를 갱신합니다. `_content.html`은 빌드 입력일 뿐, 직접 서비스하는 화면이 아닙니다. 이미지와 평가 자료는 원래 실험 폴더에 저장합니다.

`index.html`, `assets/react/`, `data/atlas/`, `language-study/sw.js`, `build-info.json`은 자동 생성 결과입니다. 예약 작업에서 직접 작성하거나 이전 HTML UI로 덮어쓰지 마세요. 새 상세 문서는 카탈로그에 등록하면 직접 접속용 index.html이 자동 생성됩니다.

## UI 변경
메뉴는 `web/src/components/Shell.tsx`, 기능 화면은 `web/src/features`, 기본 UI는 `web/src/components/ui`, 콘텐츠 타입은 `web/src/lib/types.ts`, 콘텐츠 변환은 `web/scripts/content.ts`에서 관리합니다. 기존 DOM을 이동하거나 HTML 문자열을 주입하는 방식으로 회귀하지 마세요. 페이지별 레이아웃은 다르게 유지합니다.

`cd web && npm ci && npm run check`로 타입·데이터·기능·화면 테스트를 실행합니다. 기준 화면을 자동으로 갱신해 실패를 숨기지 마세요. 의도한 UI 변경일 때만 diff 이미지를 검토하고 기준 화면을 갱신합니다. React Pages validation and publication 워크플로의 성공과 실제 배포 검증을 확인합니다.

기존 localStorage 키, 기사 date:index ID, 하위 URL을 보존합니다. 캐시 정리 범위는 language-study-*로 제한합니다. 메모와 북마크는 삭제하지 않습니다.
