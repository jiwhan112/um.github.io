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
