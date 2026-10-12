/** Validate tracked source and Pages runtime ownership, not just file extensions. */
import { execFileSync } from 'node:child_process';
import { existsSync, readFileSync } from 'node:fs';
import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '../..');
const tracked = execFileSync('git', ['ls-files', '-z'], { cwd: root, encoding: 'utf8' })
  .split('\0').filter(Boolean);
const errors = [];
// Original _content.html/Markdown may contain ignored historical styles or scripts.
// Preserve those authoring inputs; reject only the retired executable UI files.
const retired = new Set([
  '_config.yml', 'learning-lab.css',
  'daily-briefing/app.js', 'daily-briefing/style.css', 'daily-briefing/atlas-theme.css',
  'language-study/app.js', 'language-study/style.css', 'language-study/atlas-theme.css',
  'language-study/manifest.webmanifest', 'language-study/README.md',
  'thinking-lab/atlas-lesson.css', 'thinking-lab/atlas-theme.css',
  'ai-asset-lab/atlas-theme.css',
  '.github/workflows/react-bootstrap.yml', '.github/workflows/react-migration-check.yml',
  '_maintenance/atlas-detail.md', '_maintenance/react-migration.md',
]);
const retiredDirs = ['_includes/', '_layouts/', '_posts/', 'assets/atlas/', '.migration/'];
const temporary = /(^|\/)(node_modules|playwright-report|test-results|blob-report|coverage|live-check|render-environment)\/|^web\/(dist|public|\.cache)\/|(^|\/)(\.DS_Store|Thumbs\.db)$|\.(orig|rej|bak)$/;
for (const path of tracked) {
  if (retired.has(path) || retiredDirs.some(dir => path.startsWith(dir))) errors.push('Retired file: ' + path);
  if (temporary.test(path)) errors.push('Temporary file is tracked: ' + path);
}
const required = [
  '.nojekyll', 'index.html', '404.html', 'web/package.json', 'web/package-lock.json',
  'web/src/main.tsx', 'web/src/App.tsx', 'web/scripts/content.ts', 'web/scripts/publish.mjs',
  'web/tests/functional.spec.ts', 'web/tests/visual.spec.ts', 'web/tests/pwa.spec.ts',
  '.github/workflows/react-check.yml', '.github/workflows/react-pages.yml',
  '.github/workflows/language-study-reconcile.yml', 'language-study/scripts/reconcile.py',
  '_maintenance/react-generated-files.json',
];
for (const path of required) {
  if (!tracked.includes(path) || !existsSync(join(root, path))) errors.push('Required file missing: ' + path);
}
// The currently published route files and source corpus must still be available
// while a replacement is being built. Never classify them as unused duplicates.
const manifestPath = join(root, '_maintenance/react-generated-files.json');
if (existsSync(manifestPath)) {
  const manifest = JSON.parse(readFileSync(manifestPath, 'utf8'));
  if (!Array.isArray(manifest.files) || !manifest.files.length) errors.push('Invalid generated-files manifest');
  for (const path of manifest.files || []) {
    if (typeof path !== 'string' || path.startsWith('/') || path.split('/').includes('..')) {
      errors.push('Unsafe manifest path: ' + path); continue;
    }
    if (!tracked.includes(path) || !existsSync(join(root, path))) errors.push('Published file missing: ' + path);
    if (path.endsWith('.html') && existsSync(join(root, path))) {
      const html = readFileSync(join(root, path), 'utf8');
      if (!html.includes('/um.github.io/assets/react/') || /assets\/atlas\/detail|atlas-theme\.css|learning-lab\.css/.test(html)) errors.push('Non-React runtime: ' + path);
    }
  }
}
const catalogPath = join(root, 'data/atlas/catalog.json');
if (existsSync(catalogPath)) {
  const catalog = JSON.parse(readFileSync(catalogPath, 'utf8'));
  for (const doc of catalog.documents || []) {
    if (!tracked.includes(doc.source) || !existsSync(join(root, doc.source))) errors.push('Authoring source missing: ' + doc.source);
  }
}
if (errors.length) throw new Error(errors.join('\n'));
console.log(`Repository hygiene passed: ${tracked.length} tracked files; source, tests and published routes preserved.`);
