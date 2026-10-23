import { test, expect } from '@playwright/test';
import { readFile } from 'node:fs/promises';

const catalog = JSON.parse(await readFile('../knowledge/data/catalog.json', 'utf8')) as {path:string;category:string;title:string}[];
const chapters = catalog.filter(d => d.category === 'Effective C++');

test('Effective C++ topic exposes every published chapter in catalog order', async ({page}) => {
  await page.goto('knowledge/');
  const topic = page.locator('#knowledge-cpp .knowledge-topic').filter({has:page.getByText('Effective C++', {exact:true})});
  await expect(topic).toHaveCount(1);
  const expected = chapters.map(d => `/um.github.io/knowledge/${d.path}/`);
  expect(await topic.locator('.knowledge-document-link').evaluateAll(nodes => nodes.map(n => n.getAttribute('href')))).toEqual(expected);
  if (chapters.length) await expect(topic.locator('.knowledge-topic-row')).toContainText(`${chapters.length}개 공개`);
});

const visualChapters = catalog.filter(d => d.category === 'Effective C++' || /^c-basics-(09|1[0-4])$/.test(d.path));
for (const chapter of visualChapters) test(`${chapter.path} renders all visual textbook tabs`, async ({page,request}, testInfo) => {
  const errors:string[] = [];
  page.on('pageerror', error => errors.push(error.message));
  await page.goto(`knowledge/${chapter.path}/`);
  await expect(page.locator('main h1')).toHaveText(chapter.title);
  await expect(page.getByRole('tab')).toHaveText(['개념과 원리','실습·복습','출처·검증','원문·변경기록']);
  const images = page.locator('.knowledge-illustration img');
  await expect(images).toHaveCount(2);
  for (const image of await images.all()) {
    await image.scrollIntoViewIfNeeded();
    await expect.poll(() => image.evaluate((node:HTMLImageElement) => node.complete && node.naturalWidth > 0)).toBe(true);
  }
  const diagrams = page.locator('.mermaid-diagram');
  expect(await diagrams.count()).toBeGreaterThan(0);
  for (const diagram of await diagrams.all()) {
    await diagram.scrollIntoViewIfNeeded();
    await expect(diagram).toHaveAttribute('data-diagram-state','ready');
  }
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  await page.screenshot({path:testInfo.outputPath(`${chapter.path}-concepts.png`),fullPage:true});
  await page.getByRole('tab',{name:'실습·복습'}).click();
  const reviews = page.locator('#review details');
  const authored = JSON.parse(await readFile(`../knowledge/${chapter.path}/content.json`, 'utf8')) as {sections:{id:string;blocks:{type:string}[]}[]};
  const reviewCount = authored.sections.find(section => section.id === 'review')!.blocks.filter(block => block.type === 'details').length;
  expect(reviewCount).toBeGreaterThanOrEqual(6);
  // A tab click schedules a React/router transition; wait for its actual authored content.
  await expect(reviews).toHaveCount(reviewCount);
  for (const review of await reviews.all()) await expect(review).toHaveJSProperty('open',false);
  await reviews.first().locator('summary').click();
  await expect(reviews.first()).toHaveJSProperty('open',true);
  const fullCode = page.locator('#full-code, #complete-code');
  await expect(fullCode).toHaveCount(1);
  expect(await fullCode.locator('details').count()).toBeGreaterThan(0);
  for (const code of await fullCode.locator('details').all()) await code.locator('summary').click();
  const sourceLinks = fullCode.locator('a[href]');
  expect(await sourceLinks.count()).toBeGreaterThan(0);
  for (const link of await sourceLinks.all()) {
    const href = await link.getAttribute('href');
    expect(href).toMatch(/^\/um\.github\.io\/knowledge\/.*\.(?:c|cpp|h|hpp)$/);
    const source = await readFile('..' + href!.replace('/um.github.io',''), 'utf8');
    const response = await request.get(href!);
    expect(response.ok()).toBe(true);
    expect(await response.text()).toBe(source);
    expect(await fullCode.locator('pre code').allTextContents()).toContain(source);
  }
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  await page.screenshot({path:testInfo.outputPath(`${chapter.path}-practice.png`),fullPage:true});
  await page.getByRole('tab',{name:'출처·검증'}).click();
  const verification = chapter.category === 'Effective C++' ? 'effective-cpp-01-06/verification.json' : 'c-basics-09-14/verification.json';
  await expect(page.locator(`a[href$="${verification}"]`)).toBeVisible();
  await page.getByRole('tab',{name:'원문·변경기록'}).click();
  if (chapter.category === 'Effective C++') await expect(page.locator('a[href$="source-notion.md"]')).toBeVisible();
  else await expect(page.locator('.prose')).toContainText('Notion');
  expect(errors).toEqual([]);
});
