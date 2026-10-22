import { test, expect } from '@playwright/test';

test('C01 concept diagrams render on desktop/mobile, with accessible captions and no horizontal clipping', async ({ page }, testInfo) => {
  const errors: string[] = [];
  page.on('pageerror', error => errors.push(error.message));
  await page.goto('knowledge/c-language-01/');
  await expect(page.getByRole('heading', { level: 1 })).toContainText('C01');
  await expect(page.getByRole('tab')).toHaveText(['개념과 원리', '실습·복습', '출처·검증', '원문·변경기록']);
  const figures = page.locator('.knowledge-illustration');
  await expect(figures).toHaveCount(2);
  for (const [i, figure] of (await figures.all()).entries()) {
    await figure.scrollIntoViewIfNeeded();
    const img = figure.locator('img');
    await expect(img).toHaveAttribute('alt', /.+/);
    await expect.poll(() => img.evaluate((node: HTMLImageElement) => node.complete && node.naturalWidth > 0)).toBe(true);
    await figure.screenshot({ path: testInfo.outputPath('c01-illustration-' + (i + 1) + '.png') });
  }
  const mermaids = page.locator('.mermaid-diagram');
  await expect(mermaids).toHaveCount(2);
  for (const [i, diagram] of (await mermaids.all()).entries()) {
    await diagram.scrollIntoViewIfNeeded();
    await expect(diagram).toHaveAttribute('data-diagram-state', 'ready');
    await expect(diagram.locator('.mermaid-source')).toHaveJSProperty('open', false);
    await expect.poll(() => diagram.locator('.mermaid-canvas img').evaluate((node: HTMLImageElement) => node.complete && node.naturalWidth > 0)).toBe(true);
    await diagram.screenshot({ path: testInfo.outputPath('c01-mermaid-' + (i + 1) + '.png') });
  }
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  await page.screenshot({ path: testInfo.outputPath('c01-concepts-full.png'), fullPage: true });
  expect(errors).toEqual([]);
});

test('C01 practice, downloadable Korean C and C++ sources, and original archive all remain accessible', async ({ page, request }, testInfo) => {
  await page.goto('knowledge/c-language-01/');
  await page.getByRole('tab', { name: '실습·복습' }).click();
  const questions = page.locator('#review details');
  await expect(questions).toHaveCount(12);
  await expect(questions.first()).not.toHaveAttribute('open');
  await questions.first().locator('summary').click();
  await expect(questions.first()).toHaveAttribute('open', '');
  await expect(questions.first()).toContainText('정수 두 항목의 변환');
  await page.getByRole('tab', { name: '출처·검증' }).click();
  for (const file of ['ko/c11/main.c', 'ko/c01b/main.cpp', 'original/c11/main.c', 'original/c01b/main.cpp']) {
    const path = '/um.github.io/knowledge/shared/c01-lab/' + file;
    await expect(page.locator('a[href="' + path + '"]')).toBeVisible();
    const response = await request.get(path);
    expect(response.status()).toBe(200);
    expect(await response.text()).toContain('int main(');
    if (file.startsWith('ko/')) expect(await response.text()).toMatch(/\/\/[^\n]*[가-힣]/);
  }
  await page.getByRole('tab', { name: '원문·변경기록' }).click();
  const original = await request.get('/um.github.io/knowledge/c-language-01/source-notion.md');
  expect(original.status()).toBe(200);
  expect(await original.text()).toContain('scanf');
  await page.screenshot({ path: testInfo.outputPath('c01-originals.png'), fullPage: true });
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
});
