import { test, expect } from '@playwright/test';

const chapters = [
  { slug: 'cpp-basics-01', source: 'types', review: 'review', previous: null, next: 'cpp-basics-02' },
  { slug: 'cpp-basics-02', source: 'flow', review: 'review', previous: 'cpp-basics-01', next: 'cpp-basics-03' },
  { slug: 'cpp-basics-03', source: 'memory', review: 'practice-review', previous: 'cpp-basics-02', next: null },
];

for (const chapter of chapters) test(`${chapter.slug} connects visual concepts, practice, exact sources and adjacent lessons`, async ({ page, request }, testInfo) => {
  const errors: string[] = [];
  page.on('pageerror', error => errors.push(error.message));
  await page.goto(`knowledge/${chapter.slug}/`);
  await expect(page.getByRole('heading', { level: 1 })).toContainText('C++ 기초');
  await expect(page.getByRole('tab')).toHaveText(['개념과 원리', '실습·복습', '출처·검증', '원문·변경기록']);
  expect(await page.locator('.knowledge-illustration img').count()).toBeGreaterThanOrEqual(2);
  for (const illustration of await page.locator('.knowledge-illustration img').all()) {
    await illustration.scrollIntoViewIfNeeded();
    await expect.poll(() => illustration.evaluate((node: HTMLImageElement) => node.complete && node.naturalWidth > 0)).toBe(true);
  }
  expect(await page.locator('.mermaid-diagram').count()).toBeGreaterThan(0);
  for (const diagram of await page.locator('.mermaid-diagram').all()) {
    await diagram.scrollIntoViewIfNeeded();
    await expect(diagram).toHaveAttribute('data-diagram-state', 'ready');
    await expect(diagram.locator('.mermaid-source')).toHaveJSProperty('open', false);
    await expect.poll(() => diagram.locator('.mermaid-canvas img').evaluate((node: HTMLImageElement) => node.complete && node.naturalWidth > 0)).toBe(true);
  }
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  const adjacent = page.getByRole('navigation', { name: '이전 다음 문서' });
  const expected = [chapter.previous, chapter.next].filter(Boolean).map(slug => `/um.github.io/knowledge/${slug}/`);
  expect(await adjacent.getByRole('link').evaluateAll(nodes => nodes.map(node => node.getAttribute('href')))).toEqual(expected);
  await page.evaluate(() => document.fonts.ready);
  await page.evaluate(() => scrollTo(0, 0));
  await page.screenshot({ path: testInfo.outputPath(`${chapter.slug}-first-screen.png`) });
  await page.screenshot({ path: testInfo.outputPath(`${chapter.slug}-lesson.png`), fullPage: true });

  await page.getByRole('tab', { name: '실습·복습' }).click();
  const question = page.locator(`#${chapter.review} .disclosure`).first();
  await expect(question).toHaveJSProperty('open', false);
  await question.locator('summary').click();
  await expect(question).toHaveJSProperty('open', true);
  await expect(question.locator('p').first()).toBeVisible();
  await page.getByRole('tab', { name: '출처·검증' }).click();
  const korean = page.locator(`a[href="/um.github.io/knowledge/shared/cpp-basics-lab/ko/${chapter.source}.cpp"]`).first();
  await expect(korean).toBeVisible();
  const sourceResponse = await request.get((await korean.getAttribute('href'))!);
  expect(sourceResponse.ok()).toBe(true);
  const sourceText = await sourceResponse.text();
  expect(sourceText).toMatch(/int main\s*\(/);
  expect(sourceText).toMatch(/[가-힣]/);
  const original = await request.get(`/um.github.io/knowledge/shared/cpp-basics-lab/original/src/${chapter.source}.cpp`);
  expect(original.ok()).toBe(true);
  expect(await original.text()).toContain('Report()');
  const capture = await request.get(`/um.github.io/knowledge/${chapter.slug}/source-notion-capture.json`);
  expect(capture.ok()).toBe(true);
  expect((await capture.json()).sha256).toMatch(/^[a-f0-9]{64}$/);
  await page.getByRole('tab', { name: '원문·변경기록' }).click();
  await expect(page.locator('.prose')).toContainText('Notion');
  await expect(page.locator('.prose')).toContainText('C++11');
  expect(errors).toEqual([]);
});

test('bitmask operations distinguish clearing from toggling and preserve existing storage', async ({ page }, testInfo) => {
  await page.addInitScript(() => {
    localStorage.setItem('atlas-reading-v1', JSON.stringify({ 'knowledge/dx11-frustum/': true }));
    localStorage.setItem('jiwhan-briefing-v1', JSON.stringify({ saved: ['2026-10-12:0'], read: [] }));
    localStorage.setItem('unrelated-value', 'keep-me');
  });
  await page.goto('knowledge/cpp-basics-01/');
  const lab = page.getByRole('region', { name: '켜기·끄기·뒤집기는 어떻게 다를까?' });
  await lab.scrollIntoViewIfNeeded();
  const decimal = lab.getByLabel('현재 flags의 십진수 값');
  const binary = lab.getByLabel('현재 flags의 이진수 값');
  await expect(decimal).toHaveText('0');
  await lab.getByRole('button', { name: '켜기 |=' }).click();
  await expect(decimal).toHaveText('4');
  await lab.getByRole('button', { name: '켜기 |=' }).click();
  await expect(decimal).toHaveText('4');
  await lab.getByRole('button', { name: '끄기 &= ~' }).click();
  await expect(decimal).toHaveText('0');
  await lab.getByRole('button', { name: '끄기 &= ~' }).click();
  await expect(decimal).toHaveText('0');
  await lab.getByRole('button', { name: '뒤집기 ^=' }).click();
  await expect(decimal).toHaveText('4');
  await lab.getByRole('button', { name: '뒤집기 ^=' }).click();
  await expect(decimal).toHaveText('0');
  await lab.getByRole('button', { name: '비트 7 선택 · 값 128' }).click();
  await lab.getByRole('button', { name: '켜기 |=' }).click();
  await expect(decimal).toHaveText('128');
  await lab.getByRole('button', { name: '비트 0 선택 · 값 1' }).click();
  await lab.getByRole('button', { name: '켜기 |=' }).click();
  await expect(decimal).toHaveText('129');
  await lab.getByRole('button', { name: '비트 7 선택 · 값 128' }).click();
  await lab.getByRole('button', { name: '끄기 &= ~' }).click();
  await expect(decimal).toHaveText('1');
  await expect(binary).toHaveText('00000001');
  const bit2 = lab.getByRole('button', { name: '비트 2 선택 · 값 4' });
  await bit2.focus();
  await bit2.press('Space');
  await expect(bit2).toHaveAttribute('aria-pressed', 'true');
  const set = lab.getByRole('button', { name: '켜기 |=' });
  await set.focus();
  await set.press('Enter');
  await expect(decimal).toHaveText('5');
  await expect(lab.getByRole('status').last()).toContainText('마지막 실행 · 비트 2 켜기');
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  await lab.screenshot({ path: testInfo.outputPath('cpp-bitmask-interaction.png') });
  await lab.getByRole('button', { name: '처음 상태로' }).click();
  await expect(decimal).toHaveText('0');
  await expect(bit2).toHaveAttribute('aria-pressed', 'true');
  const saved = await page.evaluate(() => ({ reading: localStorage.getItem('atlas-reading-v1'), briefing: localStorage.getItem('jiwhan-briefing-v1'), other: localStorage.getItem('unrelated-value') }));
  expect(JSON.parse(saved.reading!)).toEqual({ 'knowledge/dx11-frustum/': true });
  expect(JSON.parse(saved.briefing!)).toEqual({ saved: ['2026-10-12:0'], read: [] });
  expect(saved.other).toBe('keep-me');
});

test('C++ curriculum starts at the three authored foundation lessons', async ({ page }) => {
  await page.goto('knowledge/');
  const cpp = page.locator('#knowledge-cpp');
  await expect(cpp.locator('.knowledge-document-link')).toHaveCount(3);
  expect(await cpp.locator('.knowledge-document-link').evaluateAll(nodes => nodes.map(node => node.getAttribute('href')))).toEqual(chapters.map(chapter => `/um.github.io/knowledge/${chapter.slug}/`));
  await cpp.locator('.knowledge-document-link').first().click();
  await expect(page).toHaveURL(/\/knowledge\/cpp-basics-01\/$/);
});
