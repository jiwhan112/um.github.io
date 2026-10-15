/** Verify the real GitHub Pages HTTP response and exact contents for each game-design image.
 * Run only after live-smoke.mjs confirms the published source commit.
 * Writes web/live-check/game-design-assets.json even on failure.
 */
import { createHash } from 'node:crypto';
import { readFile, mkdir, writeFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const repo = fileURLToPath(new URL('../../', import.meta.url));
const origin = 'https://jiwhan112.github.io/um.github.io/';
const reportPath = join(repo, 'web/live-check/game-design-assets.json');
const mediaTypes = {
  svg: 'image/svg+xml', png: 'image/png', webp: 'image/webp',
  jpg: 'image/jpeg', jpeg: 'image/jpeg',
};
const digest = bytes => createHash('sha256').update(bytes).digest('hex');
const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));

function collectImages(value, images = []) {
  if (Array.isArray(value)) {
    for (const item of value) collectImages(item, images);
  } else if (value && typeof value === 'object') {
    if (value.type === 'image') images.push(value);
    for (const item of Object.values(value)) collectImages(item, images);
  }
  return images;
}

async function inspect(episode, image) {
  const src = image.src;
  const item = { episode: episode.number, src, url: null, status: null, contentType: null, passed: false };
  try {
    const prefix = '/game-design/' + episode.path + 'assets/';
    if (typeof src !== 'string' || !src.startsWith(prefix) ||
        !/^\/game-design\/ep-[a-z0-9-]+\/assets\/[a-z0-9-]+\.(svg|png|webp|jpg|jpeg)$/.test(src)) {
      throw new Error('Invalid episode image path: ' + String(src));
    }
    if (typeof image.alt !== 'string' || !image.alt.trim() ||
        typeof image.caption !== 'string' || !image.caption.trim()) {
      throw new Error('Image requires nonempty alt and caption');
    }
    const extension = src.slice(src.lastIndexOf('.') + 1);
    const local = await readFile(join(repo, src.slice(1)));
    const expectedHash = digest(local);
    item.expectedSha256 = expectedHash;
    const url = new URL(src.slice(1), origin);
    item.url = url.href;

    let lastError;
    for (let attempt = 1; attempt <= 6; attempt++) {
      item.attempts = attempt;
      try {
        const checkUrl = new URL(url.href);
        checkUrl.searchParams.set('asset_check', Date.now() + '-' + attempt);
        const response = await fetch(checkUrl, {
          cache: 'no-store', signal: AbortSignal.timeout(12000),
          headers: { Accept: mediaTypes[extension] },
        });
        item.status = response.status;
        item.contentType = response.headers.get('content-type');
        if (response.status !== 200) throw new Error('HTTP ' + response.status);
        if ((item.contentType || '').toLowerCase().split(';')[0].trim() !== mediaTypes[extension]) {
          throw new Error('Expected ' + mediaTypes[extension] + ', got ' + item.contentType);
        }
        const actualHash = digest(Buffer.from(await response.arrayBuffer()));
        item.actualSha256 = actualHash;
        if (actualHash !== expectedHash) throw new Error('Deployed bytes differ from repository source');
        item.passed = true;
        console.log('PASS ' + item.status + ' ' + src + ' sha256=' + actualHash.slice(0, 12));
        return item;
      } catch (error) {
        lastError = error;
        if (attempt < 6) await sleep(5000);
      }
    }
    throw lastError;
  } catch (error) {
    item.error = error?.message || String(error);
    console.error('FAIL ' + String(src) + ': ' + item.error);
    return item;
  }
}

const results = [];
let fatal;
try {
  const catalog = JSON.parse(await readFile(join(repo, 'game-design/data/catalog.json'), 'utf8'));
  if (!Array.isArray(catalog.episodes) || !catalog.episodes.length) throw new Error('Game-design catalog missing or empty');
  const entries = [];
  for (const episode of catalog.episodes) {
    if (!/^ep-[a-z0-9-]+\/$/.test(episode.path)) throw new Error('Unsafe catalog path: ' + episode.path);
    const data = JSON.parse(await readFile(join(repo, 'game-design', episode.path, 'content.json'), 'utf8'));
    for (const image of collectImages(data.sections)) entries.push({ episode, image });
  }
  if (!entries.length) throw new Error('No game-design image blocks to verify');
  // Limit concurrency without skipping any linked image.
  for (let i = 0; i < entries.length; i += 4) {
    results.push(...await Promise.all(entries.slice(i, i + 4).map(({ episode, image }) => inspect(episode, image))));
  }
  if (results.some(item => !item.passed)) throw new Error(results.filter(item => !item.passed).length + '/' + results.length + ' live images failed verification');
  console.log('PUBLIC GAME-DESIGN IMAGES VERIFIED: ' + results.length + ' images across ' + catalog.episodes.length + ' episodes');
} catch (error) {
  fatal = error?.message || String(error);
  console.error('PUBLIC IMAGE VERIFICATION FAILED:', fatal);
  process.exitCode = 1;
} finally {
  await mkdir(dirname(reportPath), { recursive: true });
  await writeFile(reportPath, JSON.stringify({
    origin, checkedAt: new Date().toISOString(),
    passed: !fatal, total: results.length, results, error: fatal || null,
  }, null, 2) + '\n');
}
