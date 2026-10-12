# ATLAS NOTE — React 작업 규칙

실제 UI는 `web/src/`의 React + TypeScript 컴포넌트입니다. 먼저 루트 `README.md`와 `web/README.md`를 읽으세요.

## 예약 콘텐츠 발행

뉴스·언어 학습은 기존 data JSON과 카탈로그만 갱신합니다. 사고력/오픈소스 문서는 회차의 `_content.html` 또는 검증 가능한 `content.json`과 카탈로그를 갱신합니다. `_content.html`은 빌드 입력이지 배포 UI가 아닙니다. 실험 이미지·평가·보고서는 원래 실험 폴더에 저장합니다.

`index.html`, `assets/react/`, `data/atlas/`, `language-study/sw.js`, `build-info.json`, `_maintenance/react-generated-files.json`은 자동 생성 결과입니다. 예약 작업에서 직접 작성하거나 이전 HTML UI로 덮어쓰지 마세요. 새 회차는 카탈로그에 등록하면 직접 접속용 `index.html`이 생성됩니다.

## UI 변경과 검증

메뉴는 `web/src/components/Shell.tsx`, 기능 화면은 `web/src/features/`, 기본 UI는 `web/src/components/ui/`, 콘텐츠 계약은 `web/src/lib/types.ts`, 변환은 `web/scripts/content.ts`에서 관리합니다. HTML 문자열 주입이나 기존 DOM 재배치로 회귀하지 마세요. 페이지별 레이아웃의 차이는 유지합니다.

`cd web && npm ci && npm run check`를 실행합니다. CI와 같은 Ubuntu 브라우저·글꼴 환경은 `bash scripts/install-browser.sh`로 준비합니다. 화면 기준을 자동 갱신하거나 허용 오차를 늘려 실패를 숨기지 않습니다. 운영 게시 workflow 성공과 실제 URL 검증을 구분해 확인합니다.

기존 localStorage 키, 기사 date:index ID, 하위 URL을 보존합니다. 캐시 정리는 `language-study-*`에만 적용하고 사용자 메모와 북마크를 지우지 않습니다.

## 파일 정리

`npm run check:repo`로 구형 UI·Jekyll 실행 파일과 임시/빌드 캐시의 재유입을 검사합니다. `node_modules`, 로컬 `web/dist`, 테스트 보고서, 작업용 스크린샷은 커밋하지 않습니다. 승인된 `web/tests/*-snapshots/`는 필수 회귀 기준입니다.

파일 삭제 전 콘텐츠 변환기·라우팅·게시기·workflow의 사용 여부를 확인합니다. 저작 원본, lockfile, 실제 배포 파일, 시험 기준을 미사용 파일로 취급하지 않습니다. UI 변경이 없으면 원본 JSON/HTML/Markdown/PNG와 승인 화면을 수정하지 않습니다. 복구에는 Git 이력을 사용하며 별도 백업 파일을 추가하지 않습니다.
