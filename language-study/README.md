# Language Study PWA

- `index.html`: single app shell
- `app.js`, `style.css`: UI/TTS/review presentation
- `data/catalog.json`: lesson index
- `data/progress.json`: independent English/Japanese levels and review schedule
- `data/english/*.json`, `data/japanese/*.json`: lesson data only
- `manifest.webmanifest`, `sw.js`: PWA/offline shell

Important: Google Drive stores the source files but does not serve them as a PWA website. Deploy this folder to an HTTPS static host (e.g. GitHub Pages / Cloudflare Pages) for installable mobile use.
