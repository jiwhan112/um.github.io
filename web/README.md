# ATLAS NOTE · React workspace

Production frontend: **React 19 + TypeScript 5 + Vite 7 + React Router 7 + Tailwind CSS 4**.
Accessible primitives are maintained shadcn/ui-style components composed from Radix Tabs, Dialog and Slot.

```sh
cd web
npm ci
npm run dev        # http://localhost:5173/um.github.io/
npm run test:unit  # content/state invariants
npm run build     # typecheck → validate/extract source content → Vite → real route files
npx playwright install chromium
npm run test:e2e   # desktop/mobile behavior + approved screenshot baselines
```

## Architecture
- `src/components/Shell.tsx`: one navigation and search implementation across all routes.
- `src/features/`: independent dashboard, article reader, language textbook, problem workspace, gallery and archive layouts.
- `src/components/ui/`: Radix-based controlled primitives; no legacy DOM manipulation.
- `src/lib/types.ts`: Zod source/build contracts and TypeScript domain types.
- `src/lib/storage.ts`: validated storage adapters preserving the user's original keys.
- `scripts/content.ts`: build-time adapter from existing JSON and `_content.html`/Markdown to typed content blocks. Scripts/styles are excluded; C++ text and source files are preserved. Adjacent `content.json` is supported for future structured authoring.
- `src/components/Content.tsx`: explicit React block renderer; **no iframe, raw HTML injection or runtime DOM reparenting**.
- `scripts/routes.ts`: emits a real `index.html` for each route, plus a genuine 404 page and scoped PWA worker.

Base path is `/um.github.io/`. Legacy language hash bookmarks redirect to clean routes. Existing source JSON, PNG and authoring documents remain at their original repository paths. `dist` is generated; never edit compiled bundles.

## Tests and snapshots
Functional tests use the complete current corpus. Visual tests freeze a small set of actual source examples in `tests/fixtures`, so a new scheduled article is not mistaken for a design regression. Snapshot regeneration requires a deliberate reviewed change: `npx playwright test visual --update-snapshots`. CI never updates baselines automatically.

TTS relies on the user's installed browser/OS voices. The C++ editor is a saved draft editor, **not a remote compiler**. Data stays in the user's browser; no account sync was added.
