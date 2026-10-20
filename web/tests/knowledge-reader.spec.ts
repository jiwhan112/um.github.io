import { test, expect } from '@playwright/test';

const lesson = 'knowledge/dx9-impl-09/';

test('visual lesson separates learning from archives and retains reading state', async ({ page }, testInfo) => {
  await page.addInitScript(() => {
    localStorage.setItem('atlas-reading-v1', JSON.stringify({ 'knowledge/dx9-impl-09/': true, 'knowledge/dx11-frustum/': true }));
    localStorage.setItem('unrelated-value', 'keep-me');
  });
  await page.goto(lesson);
  await expect(page.getByRole('heading', { name: '잎·연기·불꽃은 왜 다르게 그릴까?', exact: true })).toBeVisible();
  await expect(page.getByLabel('이 문서 학습 완료')).toBeChecked();
  await expect(page.locator('.knowledge-illustration')).toHaveCount(3);
  const size = await page.locator('.knowledge-illustration img').first().boundingBox();
  expect(size!.width).toBeGreaterThan(280);
  await expect(page.getByText('Educational CPU reference model', { exact: false })).toHaveCount(0);
  await expect(page.locator('#part-16')).toHaveCount(0);
  await expect(page.locator('.mermaid-diagram').first()).toHaveAttribute('data-diagram-state', 'ready');
  const adjacent = page.getByRole('navigation', { name: '이전 다음 문서' });
  await expect(adjacent.getByRole('link').nth(0)).toHaveAttribute('href', '/um.github.io/knowledge/dx9-impl-08/');
  await expect(adjacent.getByRole('link').nth(1)).toHaveAttribute('href', '/um.github.io/knowledge/dx9-impl-10/');
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  for (const illustration of await page.locator('.knowledge-illustration img').all()) {
    await illustration.scrollIntoViewIfNeeded();
    await expect.poll(() => illustration.evaluate((node: HTMLImageElement) => node.complete && node.naturalWidth > 0)).toBe(true);
  }
  await page.evaluate(() => window.scrollTo(0, 0));
  await page.evaluate(() => document.fonts.ready);
  await page.screenshot({ path: testInfo.outputPath('dx9-09-lesson.png'), fullPage: true });
  await page.screenshot({ path: testInfo.outputPath('dx9-09-first-screen.png') });
  await page.getByLabel('이 문서 학습 완료').uncheck();
  const state = await page.evaluate(() => JSON.parse(localStorage.getItem('atlas-reading-v1')!));
  expect(state['knowledge/dx9-impl-09/']).toBe(false);
  expect(state['knowledge/dx11-frustum/']).toBe(true);
  expect(await page.evaluate(() => localStorage.getItem('unrelated-value'))).toBe('keep-me');
});

test('alpha lab matches hand-worked linear RGB examples', async ({ page }, testInfo) => {
  await page.goto(lesson);
  const lab = page.getByRole('region', { name: '알파가 바뀌면 색은 어떻게 달라질까?' });
  await lab.scrollIntoViewIfNeeded();
  const output = lab.locator('.blend-swatch-result code');
  await expect(output).toHaveText('(0.40, 0.30, 0.45)');
  await lab.getByLabel('합성 방식').selectOption('premultiplied');
  await expect(output).toHaveText('(0.40, 0.30, 0.45)');
  await expect(lab.locator('.blend-swatch').first()).toContainText('(0.25, 0.00, 0.00)');
  await lab.getByLabel('합성 방식').selectOption('additive');
  await expect(output).toHaveText('(0.45, 0.40, 0.60)');
  await lab.getByRole('slider', { name: '소스 알파 α' }).press('End');
  await expect(output).toHaveText('(1.20, 0.40, 0.60)');
  await lab.getByRole('slider', { name: '소스 알파 α' }).press('Home');
  await expect(output).toHaveText('(0.20, 0.40, 0.60)');
  await lab.getByLabel('합성 방식').selectOption('straight');
  for (let step = 0; step < 25; step++) await lab.getByRole('slider', { name: '소스 알파 α' }).press('ArrowRight');
  await lab.screenshot({ path: testInfo.outputPath('dx9-09-alpha-lab.png') });
});

test('table of contents scrolls and old section hashes open their new tab', async ({ page, isMobile }) => {
  await page.goto(lesson);
  if (isMobile) await page.locator('.knowledge-mobile-toc > summary').click();
  const toc = page.getByRole('navigation', { name: isMobile ? '모바일 단원 목차' : '문서 목차', exact: true });
  await toc.getByRole('link', { name: '6. 사각형을 카메라 쪽으로 돌린다' }).click();
  await expect(page).toHaveURL(/#part-8$/);
  await expect(page.locator('#part-8 > h2')).toBeInViewport();
  await page.getByRole('tab', { name: '실습·복습', exact: true }).click();
  await expect(page.locator('#part-2')).toBeVisible();
  await expect(page).not.toHaveURL(/#part-8/);
  await page.goto(lesson + '?tab=document#part-16');
  await expect(page.getByRole('tab', { name: '원문·변경기록', exact: true })).toHaveAttribute('data-state', 'active');
  await expect(page.locator('#part-16')).toBeInViewport();
  await expect(page.locator('#part-16 details')).not.toHaveAttribute('open');
});

test('Korean teaching source is complete, downloadable and linked beside practice', async ({ page, request }, testInfo) => {
  await page.goto(lesson + '?tab=practice');
  await expect(page.locator('#part-2')).toBeVisible();
  const sourceLink = page.getByRole('link', { name: '한국어 주석 C++ 파일', exact: true });
  await expect(sourceLink).toBeVisible();
  await expect(sourceLink).toHaveAttribute('href', '/um.github.io/knowledge/dx9-impl-09/lab/main.ko.cpp');
  const response = await request.get((await sourceLink.getAttribute('href'))!);
  expect(response.status()).toBe(200);
  const code = await response.text();
  expect(code).toContain('int main()');
  expect(code).toContain('std::stable_sort');
  expect(code).toMatch(/\/\/[^\n]*[가-힣]/);
  expect(code.split('\n').length).toBeGreaterThan(300);
  await expect(page.locator('.mermaid-diagram')).toHaveAttribute('data-diagram-state', 'ready');
  await page.screenshot({ path: testInfo.outputPath('dx9-09-practice.png'), fullPage: true });
});
