/** C01 public-origin checks; fresh browser contexts never sign in or write learning data. */
import { chromium, expect } from '@playwright/test';
import { readFile, readdir, mkdir, writeFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { join } from 'node:path';

const base = 'https://jiwhan112.github.io/um.github.io/';
const expected = process.env.EXPECTED_SOURCE;
if (!expected) throw Error('EXPECTED_SOURCE is required');
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
await mkdir('live-check', { recursive: true });
const report = { base, expectedSource: expected, resources: [], screens: [], unverified: [
  'Windows/MSVC', 'GPU/Direct3D/HLSL execution', 'ABI/performance/concurrency measurements',
  'Historical Notion file-block attachment binary', 'Physical mobile device', 'Authenticated Supabase two-device sync'
] };
async function exact(path, localPath = '../' + path) {
  const response = await fetch(base + path + '?c01=' + Date.now(), { cache: 'no-store' });
  expect(response.status, path).toBe(200);
  const actual = Buffer.from(await response.arrayBuffer());
  const local = await readFile(localPath);
  expect(hash(actual), path).toBe(hash(local));
  report.resources.push({ path, bytes: actual.length, sha256: hash(actual), status: response.status });
}
async function walk(path) {
  for (const item of await readdir('../' + path, { withFileTypes: true })) {
    const child = join(path, item.name);
    if (item.isDirectory()) await walk(child); else await exact(child);
  }
}
const infoResponse = await fetch(base + 'build-info.json?c01=' + Date.now(), { cache: 'no-store' });
expect(infoResponse.status).toBe(200);
report.buildInfo = await infoResponse.json();
expect(report.buildInfo.sourceCommit).toBe(expected);
await exact('data/atlas/docs/knowledge--c-language-01.json', 'dist/data/atlas/docs/knowledge--c-language-01.json');
await exact('data/atlas/catalog.json', 'dist/data/atlas/catalog.json');
await walk('knowledge/c-language-01');
await walk('knowledge/shared/c01-lab');

const browser = await chromium.launch();
try {
  for (const viewport of [{ width: 1440, height: 1000 }, { width: 390, height: 844 }]) {
    const mobile = viewport.width === 390;
    const context = await browser.newContext({ viewport, isMobile: mobile, hasTouch: mobile,
      locale: 'ko-KR', timezoneId: 'Asia/Seoul', serviceWorkers: 'block' });
    const page = await context.newPage();
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    const response = await page.goto(base + 'knowledge/c-language-01/');
    expect(response.status()).toBe(200);
    await expect(page.getByRole('heading', { level: 1 })).toContainText('C01');
    await expect(page.getByRole('tab')).toHaveText(['개념과 원리', '실습·복습', '출처·검증', '원문·변경기록']);
    const concepts = JSON.parse(await readFile('../knowledge/c-language-01/content.json', 'utf8'));
    for (const section of concepts.sections.filter(s => s.tab === 'document')) {
      await expect(page.locator('#' + section.id)).toContainText(section.title);
    }
    const toc = page.getByRole('navigation', { name: mobile ? '모바일 단원 목차' : '문서 목차', exact: true });
    if (mobile) await page.locator('.knowledge-mobile-toc summary').click();
    await expect(toc.getByRole('link')).toHaveCount(6);
    await toc.getByRole('link').nth(1).click();
    await expect(page).toHaveURL(/#scanf$/);
    await expect(page.locator('.knowledge-illustration')).toHaveCount(2);
    for (const image of await page.locator('.knowledge-illustration img').all()) {
      await image.scrollIntoViewIfNeeded();
      await expect.poll(() => image.evaluate(node => node.complete && node.naturalWidth > 0)).toBe(true);
    }
    const diagrams = page.locator('.mermaid-diagram');
    await expect(diagrams).toHaveCount(2);
    for (const [i, diagram] of (await diagrams.all()).entries()) {
      await diagram.scrollIntoViewIfNeeded();
      await expect(diagram).toHaveAttribute('data-diagram-state', 'ready');
      await expect.poll(() => diagram.locator('.mermaid-canvas img').evaluate(node => node.complete && node.naturalWidth > 0)).toBe(true);
      await diagram.screenshot({ path: `live-check/c01-${viewport.width}-mermaid-${i + 1}.png` });
    }
    // Real line breaks in all three authored excerpts, rather than a displayed backslash-n.
    for (const id of ['scanf', 'overflow', 'parse']) {
      const block = page.locator('#' + id + ' pre').first();
      expect(await block.innerText()).toContain('\n');
    }
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
    await page.screenshot({ path: `live-check/c01-${viewport.width}-concepts.png`, fullPage: true });
    await page.getByRole('tab', { name: '실습·복습' }).click();
    const questions = page.locator('#review details');
    await expect(questions).toHaveCount(12);
    for (const question of await questions.all()) {
      await expect(question).not.toHaveAttribute('open');
      await question.locator('summary').click();
      await expect(question).toHaveAttribute('open', '');
      await question.locator('summary').click();
    }
    await page.getByRole('tab', { name: '출처·검증' }).click();
    for (const file of ['ko/c11/main.c', 'ko/c01b/main.cpp', 'original/c11/main.c', 'original/c01b/main.cpp', 'original/c11/CMakeLists.txt', 'original/c01b/CMakeLists.txt']) {
      const path = '/um.github.io/knowledge/shared/c01-lab/' + file;
      const link = page.locator(`a[href="${path}"]`);
      await expect(link).toBeVisible();
      // Retrieve the full linked source with the browser context's HTTP client.
      const download = await context.request.get(new URL(await link.getAttribute('href'), base).href);
      expect(download.status()).toBe(200);
      const downloaded = await download.body();
      expect(hash(downloaded)).toBe(hash(await readFile('../knowledge/shared/c01-lab/' + file)));
    }
    await page.getByRole('tab', { name: '원문·변경기록' }).click();
    await expect(page.getByRole('link', { name: 'C01 Notion 원본 보존 마크다운', exact: true })).toBeVisible();
    await expect(page.getByRole('link', { name: 'C01B Notion 원본 보존 마크다운', exact: true })).toBeVisible();
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
    expect(errors).toEqual([]);
    report.screens.push({ viewport, tabs: 4, svg: 2, mermaid: 2, review: 12, downloads: 6, pageErrors: errors, horizontalClipping: false });
    await context.close();
  }
  report.completedAt = new Date().toISOString();
  await writeFile('live-check/c01-results.json', JSON.stringify(report, null, 2));
  console.log('C01 PUBLIC VERIFIED:', report.resources.length, 'exact resources; desktop and mobile; source', expected);
} finally { await browser.close(); }
