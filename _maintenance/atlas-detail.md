# ATLAS NOTE 상세 화면 · 작성/유지보수 규칙

## 콘텐츠와 화면 분리

사고력 문제 4개, 오픈소스 분석 2개, 포션 실험 상세 1개의 기존 HTML은 각 디렉터리의 `_content.html`에 보존되어 있습니다. 해당 파일의 지문·코드·힌트·평가 기록을 수정하세요. `index.html`은 Jekyll wrapper이므로 이전 독립 HTML로 덮어쓰지 않습니다.

신규 상세 페이지도 동일하게 `_content.html`에 원본 문서를 만들고, 아래 wrapper를 `index.html`로 사용합니다. 기존 HTML과 같은 `<!doctype html>` 표기와 `<main>` 루트를 사용하세요.

```liquid
---
layout: null
---
{%- capture atlas_source -%}{% include_relative _content.html %}{%- endcapture -%}
{% include atlas-detail.html source=atlas_source prefix="../../" %}
```

문제·오픈소스 회차는 prefix `../../`, 포션 실험 깊이는 `../../../../`입니다. `_includes/atlas-detail.html`이 빌드 시 상세 CSS/JS를 넣습니다. 하위 폴더에서 직접 열어도 별도 fetch나 iframe 없이 전체 문서가 제공됩니다.

## 페이지별 UI

- 사고력: 원래 section 및 textarea/save 노드를 이동합니다. 기존 메모 localStorage 키와 ID를 바꾸지 않습니다. 정답 details는 기본 접힘 상태를 유지합니다.
- 실험: 생성 결과와 검증 범위는 원본 기록을 유지합니다. 보고서는 `report/`, 프롬프트는 `prompts/prompts-v1/`로 연결합니다. 원본 PNG/JSON 경로는 그대로입니다.
- Markdown 보고서: front matter `layout: atlas-report`를 사용합니다.
- 언어 레슨: JSON 카탈로그와 레슨 데이터 구조를 유지합니다. `assets/atlas/detail.js`가 English/Japanese 실제 데이터를 렌더링합니다. 예: `language-study/#japanese/9`.

## 검증 체크리스트

1. 카탈로그의 모든 회차 URL을 열어 왼쪽 메뉴, 내용 탭, 이전/다음 이동을 확인합니다.
2. 문제 정답이 자동 노출되지 않는지, 원래 메모 키로 저장되는지 확인합니다.
3. 언어 예문/어휘/문법이 JSON과 일치하고 발음·해석이 기본 숨김인지 확인합니다.
4. 작은 화면에서 가로 오버플로가 없는지 확인합니다.
5. GitHub Pages 빌드/배포 완료 후 github-pages artifact의 최종 HTML에 assets/atlas/detail.css와 detail.js가 들어 있는지 확인합니다.
6. UI 변경만 할 때 JSON/PNG/연구 결과 내용을 변경하지 않습니다.

서비스 워커 갱신은 `language-study-` 캐시만 정리합니다. 사용자 메모·북마크·다른 섹션 캐시를 삭제하지 않습니다.
