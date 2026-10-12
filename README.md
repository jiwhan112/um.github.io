# ATLAS NOTE

게임·AI 브리핑, 영어·일본어 학습, 사고력 훈련, AI 리소스 실험을 위한 React 워크스페이스입니다.

- 운영 사이트: https://jiwhan112.github.io/um.github.io/
- UI: React · TypeScript · React Router · Vite · Tailwind CSS, Radix 기반 공통 컴포넌트
- 배포: 기존 GitHub Pages `master / root` 유지. 검사를 통과한 빌드 결과만 게시합니다.

## 파일을 수정할 위치

| 목적 | 위치 |
| --- | --- |
| 공통 메뉴·검색 | `web/src/components/Shell.tsx` |
| 페이지별 화면 | `web/src/features/` |
| 버튼·탭·대화상자 | `web/src/components/ui/` |
| 스타일·타입·저장 상태 | `web/src/styles.css`, `web/src/lib/` |
| 뉴스 원본 | `daily-briefing/data/` |
| 언어 레슨·카탈로그 | `language-study/data/` |
| 언어 카탈로그 재정리 | `language-study/scripts/reconcile.py` |
| 문제·오픈소스 원본 | 각 회차의 `_content.html` 또는 `content.json`, `data/catalog.json` |
| 실험 원본·결과물 | `ai-asset-lab/experiments/`의 HTML, Markdown, JSON, PNG 등 |
| 테스트·승인 화면 | `web/tests/` |
| 빌드·검증·게시 도구 | `web/scripts/`, `.github/workflows/` |

`_content.html`은 콘텐츠 변환기가 읽는 저작 원본입니다. 이전 UI로 실행되지 않으며, 과거 문서에 포함된 CSS/JS는 변환 결과에서 제외됩니다. 코드 예제, 기존 풀이 저장 키와 연구 기록 보존을 위해 원본은 삭제하지 않습니다.

## 자동 생성되지만 저장소에 필요한 파일

`index.html` 및 각 하위 경로의 `index.html`, `404.html`, `assets/react/`, `data/atlas/`, `build-info.json`, `favicon.svg`, `robots.txt`, `sitemap.xml`, `language-study/sw.js`, `.nojekyll`은 실제 Pages가 사용하는 배포 파일입니다. `_maintenance/react-generated-files.json`은 게시기가 관리하는 파일 목록입니다.

이 파일들은 직접 편집하지 마세요. `web/dist/`는 로컬 중간 결과로 Git에서 제외되지만, 위 파일들은 현재 `master / root` 배포 방식에 필요합니다. 소스와 배포본이 모두 있다고 해서 중복 미사용 파일인 것은 아닙니다.

## 개발·검증

```sh
cd web
npm ci
npm run dev
# 검증 서버와 같은 브라우저/글꼴 환경 준비: Ubuntu + sudo 필요
bash scripts/install-browser.sh
npm run check
```

`npm run check`는 저장소 정리 규칙, 콘텐츠·상태 단위 검사, TypeScript, 빌드, 게시 경로 검사, 데스크톱·모바일 기능/오프라인/화면 회귀 검사를 실행합니다. 기준 스크린샷은 테스트를 통과시키려고 자동 교체하지 않습니다.

## 운영 workflow

- **React validation**: PR 및 작업 브랜치 검증. 운영 게시 권한이 없습니다.
- **React Pages validation and publication**: `master` 변경을 검사하고 게시한 뒤 실제 사이트의 직접 접속을 확인합니다.
- **Reconcile language lessons**: 레슨 JSON에서 카탈로그·진도 인덱스를 재정리합니다.

콘텐츠 예약 작업은 원본과 카탈로그만 변경합니다. 사용자 메모·북마크의 localStorage 키와 기존 URL은 유지합니다. 상세 개발 규칙은 `web/README.md`, 자동 작업 규칙은 `AGENTS.md`를 참조하세요.

삭제한 구형 UI 및 전환 도구는 Git 이력에서 복구할 수 있습니다. 저장소 안에 별도 `old/`, `backup/` 복사본을 남기지 않습니다. 기존 복구용 브랜치와 커밋 이력은 변경하지 않습니다.
