# ATLAS NOTE · React workspace

Runtime: **React 19 + TypeScript 5 + Vite 7 + React Router 7 + Tailwind CSS 4**.
Shared accessible controls are shadcn/ui-style source components built with Radix Tabs, Dialog and Slot.

```sh
cd web
npm ci
npm run dev            # http://localhost:4173/um.github.io/
npm run check:repo     # reject retired UI and tracked temporary files
npm run test:unit      # validate source content and original storage keys
npm run build          # typecheck → source adapter → Vite → direct-entry HTML
# Ubuntu CI browser + fallback-font dependencies (requires sudo)
bash scripts/install-browser.sh
npm run check          # all checks including publication links and browser tests
```

## Source ownership

- `src/components/Shell.tsx`: one site navigation and search implementation.
- `src/features/`: distinct dashboard, reader, textbook, problem, gallery and archive layouts.
- `src/components/ui/`: controlled Radix-based controls; no legacy DOM manipulation.
- `src/config/supabase.ts`: site-wide public Supabase URL/publishable key; never privileged keys or user tokens.
- `src/components/SyncSettings.tsx`: login, sync status, retry and local backup; no connection values to enter on new devices.
- `src/lib/cloud.ts`: optional owner-only cloud mirror; preserves local keys and same-project sessions. See `SYNC_SETUP.md` for required RLS/owner setup and conflict limitations.
- `src/lib/types.ts`: Zod contracts and TypeScript domain types.
- `src/lib/storage.ts`: adapters preserving existing storage keys.
- `scripts/content.ts`: build-time conversion of JSON, `_content.html` and Markdown into typed blocks. Scripts/styles are excluded. Existing C++ text and research records are preserved. Structured `content.json` is supported.
- `src/components/Content.tsx`: React block renderer; no iframe, raw HTML injection or runtime DOM reparenting.
- `scripts/routes.ts`: a real `index.html` per route, genuine 404 page and scoped offline worker.
- `scripts/publish.mjs`: only generated runtime files are copied to the existing `master / root` Pages source. It refuses to overwrite authoring input or publish against a changed remote head. No force push is used.
- `scripts/check-repository.mjs`: repository hygiene and required source/distribution checks.

The base path is `/um.github.io/`. Existing language hash links redirect to clean routes. Original JSON, PNG and authoring documents keep their paths. `.nojekyll` disables Jekyll processing. Generated files are listed in `_maintenance/react-generated-files.json`; do not edit them manually. `dist/` and `public/` are ignored local build outputs.

## Authoring and publication

News/lessons use the existing JSON data and catalogs. Problem and analysis catalogs point to `_content.html` or validated `content.json`. Experiment evaluation, images and Markdown remain together in the experiment directory. The reconciler updates language catalog/progress from lesson JSON.

Content changes trigger validation and publication. CI requests a Pages build explicitly after publishing because bot commits do not start another push workflow. A failed regression gate leaves the previous compiled release in place. Workflows test fixed source revisions and reject stale publication if a newer source commit arrives.

## Tests, snapshots and recovery

Functional tests use current content. Visual tests use fixed real source examples in `tests/fixtures/`, so scheduled articles are not confused with UI changes. CI never regenerates baselines. For an intentional UI change, inspect screenshot diffs before deliberately running `npx playwright test visual --update-snapshots` in the same browser/font environment.

Retain `jiwhan-briefing-v1` date:index IDs, the original `thinking-ctci-...-draft` raw-string keys, `atlas-reading-v1`, `atlas-language-sections-v1` and `atlas-review-v1`. Never clear localStorage. The worker retires only `language-study-*` caches. TTS uses device voices; the C++ editor is a saved draft editor, not a compiler. Cloud access additionally requires owner-authenticated RLS; a successful deployment is not proof of a real two-device sync.

Use Git history for recovery rather than checked-in backup copies. Restore a complete compatible release and its publication pipeline together, never an isolated old stylesheet. The pre-React recovery branch is `backup/pre-react-20261012`.

## Scheduled publication troubleshooting

Treat source commit, React validation, generated runtime commit, and public-origin verification as separate stages. A successful CPU job or generic Pages deployment alone does not prove the latest content was compiled. Inspect the **React Pages validation and publication** run and compare its tested source SHA with the public `build-info.json` `sourceCommit`.

Knowledge tests compare the complete authored route set with compiled routes (including duplicate detection) and derive C-course membership from the source catalog. Do not freeze a growing catalog's exact count or last chapter: this blocked C06–08 on 2026-10-23. Keep the independent chapter-order checks, existing-course floor, original-source hashes, typed blocks, visual/source checks, and reviewed screenshots intact.

If publication refuses a stale head, re-read `master` and follow the latest queued run instead of force-pushing or repeatedly rebuilding the old SHA. Fix only the failed stage; do not rerun unchanged CPU experiments or rewrite lessons to repair a publication failure. Content authors must not change tests/workflows/UI to bypass a gate; an explicitly authorized maintenance task owns infrastructure fixes. Authentication/permission failures require reporting and appropriate authorization, never switching API routes to evade a denial or disabling the schedule.

Browser setup caches Chromium and downloaded Ubuntu packages by OS, architecture, lockfile and installer hash. Every fresh runner still installs the complete dependency/font set; a cache hit never skips dependencies or verification. Cold installs retain bounded retries and deadlines. The installer uses a temporary copy of apt sources to select official Ubuntu HTTPS archives instead of the slow Azure mirror, preserving repository signing keys, suites, components and normal apt verification; system sources are never edited. A warm cache reduces repeated downloads, but the first cold install still requires reachable package servers. The live-check browser is prepared before publication, so setup failure leaves the previous release untouched.

### Teaching example bundles

Additional ZIP downloads must be explicitly declared in `knowledge/examples/catalog.json`:
`{ "version": 1, "bundles": [{ "id": "lesson-01", "path": "knowledge/examples/lesson-01.zip", "sha256": "<64 lowercase hex>", "files": ["README.md", "main.cpp"], "pages": ["cpp-basics-01"] }] }`.
`pages` is optional; optional `sourceFiles` entries `{path, source, sha256}` record ZIP-relative and repository-relative source provenance. Bundle generation/review owns archive-member verification; the build never extracts or executes archives. The copier verifies exact manifest paths, regular contained files and SHA-256 before copying only the declared bytes. Undeclared ZIPs remain excluded; the original preserved C++ lab archive keeps its existing exception. Native teaching source links (`.py`, `.rc`, `.manifest`, `.fx`, `.sh`, and existing C/C++/HLSL types) use ordinary file links rather than React navigation; downloading does not execute source code.

## 기존 레슨 단어 노트 (2026-11-02)

`/language-study/notebooks/`는 기존 영어·일본어 001–030 원본의 어휘를 통합하는 React 화면입니다. `src/lib/word-notebook.ts`가 빌드 시 원본에서 `data/atlas/word-notebook.json`을 생성합니다. 생성 파일은 직접 편집하지 않습니다. 단어 ID는 언어·표제어·읽기의 결정적 인코딩이며 목록 순서에 의존하지 않습니다.

영어 기사 링크는 학습 지문 기반 출처입니다. 모든 어휘가 기사 원문에 그대로 있다는 뜻이 아닙니다. 같은 표제어의 출처별 뜻·발음은 함께 보존합니다. 원본에 없는 발음·로마자는 추정하지 않고 원문 TTS를 제공합니다. 한자어는 기존 교재에 나타난 한자 포함 어휘이며 일반 언어 빈도가 아닌 등장 레슨 수로 정렬합니다. 원본 레슨과 catalog/progress 및 기존 저장 키는 수정하지 않습니다.
