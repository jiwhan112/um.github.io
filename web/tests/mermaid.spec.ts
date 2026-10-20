import { test, expect, type Locator, type Page } from '@playwright/test';
import { readFileSync } from 'node:fs';
import type { Block, Catalog, Document } from '../src/lib/types';

const catalog = JSON.parse(readFileSync('public/data/atlas/catalog.json', 'utf8')) as Catalog;
const meta = catalog.documents.find((document) => document.route === '/knowledge/dx11-02/')!;
const source = JSON.parse(readFileSync(`public/${meta.file}`, 'utf8')) as Document;
const example = 'flowchart TD\naccTitle: 버퍼 사용 순서\naccDescr: CPU 입력을 버퍼에 기록한 다음 GPU가 읽습니다.\nA["CPU 입력"] --> B["버퍼 기록"]\nB --> C["GPU 읽기"]\n';

type UrlAuditWindow = Window & {
  __mermaidUrls: { created: string[]; revoked: string[] };
  __releaseMermaidFonts?: () => void;
};

function mermaidBlocks(blocks: Block[]): string[] {
  return blocks.flatMap((block) => {
    if (block.type === 'code') return block.language.trim().toLowerCase() === 'mermaid' ? [block.text] : [];
    if ('blocks' in block) return mermaidBlocks(block.blocks);
    if (block.type === 'list') return block.items.flatMap(mermaidBlocks);
    return [];
  });
}

async function patchDocument(page: Page, document: Document, file: string) {
  await page.route(`**/${file}`, (route) => route.fulfill({ json: document }));
  await page.goto(`/um.github.io${document.route}`);
}

async function fixture(page: Page, codes: string[], additionalBlocks: Block[] = []) {
  const document: Document = {
    ...source,
    sections: [
      { id: 'diagram-fixture', title: '도식 확인', tab: 'document', blocks: [...codes.map((text): Block => ({ type: 'code', language: 'mermaid', text })), ...additionalBlocks] },
      { id: 'diagram-practice', title: '실습 확인', tab: 'practice', blocks: [{ type: 'paragraph', content: [{ text: '다른 탭의 학습 내용' }] }] },
    ],
  };
  await patchDocument(page, document, meta.file);
}

async function auditObjectUrls(page: Page, delayFonts = false) {
  await page.addInitScript(({ delayFonts }) => {
    const audit = { created: [] as string[], revoked: [] as string[] };
    (window as unknown as UrlAuditWindow).__mermaidUrls = audit;
    const create = URL.createObjectURL.bind(URL);
    const revoke = URL.revokeObjectURL.bind(URL);
    URL.createObjectURL = (blob) => {
      const url = create(blob);
      if (blob instanceof Blob && blob.type === 'image/svg+xml') audit.created.push(url);
      return url;
    };
    URL.revokeObjectURL = (url) => { audit.revoked.push(url); revoke(url); };
    if (delayFonts) {
      const fontsReady = document.fonts.ready;
      const blocked = new Promise<FontFaceSet>((resolve) => {
        (window as unknown as UrlAuditWindow).__releaseMermaidFonts = () => { void fontsReady.then(resolve); };
      });
      Object.defineProperty(document.fonts, 'ready', { value: blocked });
    }
  }, { delayFonts });
}

async function svgDetails(image: Locator) {
  return image.evaluate(async (node: HTMLImageElement) => {
    const text = await fetch(node.src).then((response) => response.text());
    const svg = new DOMParser().parseFromString(text, 'image/svg+xml');
    const root = svg.documentElement;
    return {
      id: root.id,
      text,
      foreignObjects: svg.querySelectorAll('foreignObject').length,
      activeElements: svg.querySelectorAll('script,iframe,object,embed,image,a').length,
      eventAttributes: [...root.querySelectorAll('*')].flatMap((element) => [...element.attributes].filter((attribute) => /^on/i.test(attribute.name)).map((attribute) => attribute.name)),
      labels: [...svg.querySelectorAll('text')].map((node) => node.textContent).join(' '),
    };
  });
}

async function expectWholeDiagramFits(image: Locator) {
  const size = await image.evaluate((node: HTMLImageElement) => {
    const canvas = node.parentElement!;
    const bounds = canvas.getBoundingClientRect();
    const box = node.getBoundingClientRect();
    return {
      imageWidth: box.width, imageHeight: box.height,
      canvasWidth: canvas.clientWidth, canvasHeight: canvas.clientHeight,
      horizontalOverflow: canvas.scrollWidth - canvas.clientWidth,
      verticalOverflow: canvas.scrollHeight - canvas.clientHeight,
      contained: box.left >= bounds.left - 1 && box.top >= bounds.top - 1 && box.right <= bounds.right + 1 && box.bottom <= bounds.bottom + 1,
    };
  });
  expect(size.imageWidth).toBeGreaterThan(0);
  expect(size.imageHeight).toBeGreaterThan(0);
  expect(size.imageWidth).toBeLessThanOrEqual(size.canvasWidth);
  expect(size.imageHeight).toBeLessThanOrEqual(size.canvasHeight);
  expect(size.horizontalOverflow).toBeLessThanOrEqual(1);
  expect(size.verticalOverflow).toBeLessThanOrEqual(1);
  expect(size.contained, 'The full diagram image must fit inside its inline canvas without cropping.').toBe(true);
}

test('existing Mermaid becomes a real diagram while original source stays exact and collapsed', async ({ page }, testInfo) => {
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  await page.goto(`/um.github.io${meta.route}`);
  const originals = source.sections.filter((section) => section.tab === 'document').flatMap((section) => mermaidBlocks(section.blocks));
  expect(originals.length).toBeGreaterThan(0);
  const diagrams = page.locator('.mermaid-diagram');
  await expect(diagrams).toHaveCount(originals.length);
  for (let i = 0; i < originals.length; i++) {
    const diagram = diagrams.nth(i);
    await expect(diagram).toHaveAttribute('data-diagram-state', 'ready');
    const image = diagram.locator('.mermaid-canvas img');
    await expect(image).toHaveAttribute('src', /^blob:/);
    await expect.poll(() => image.evaluate((node: HTMLImageElement) => node.naturalWidth)).toBeGreaterThan(0);
    await expectWholeDiagramFits(image);
    await expect(image).toHaveAttribute('alt', /학습 도식|도식/);
    const original = diagram.locator('.mermaid-source');
    await expect(original).toHaveJSProperty('open', false);
    await expect(original.locator('pre')).not.toBeVisible();
    await original.locator('summary').click();
    expect(await original.locator('code').textContent()).toBe(originals[i]);
    await original.locator('summary').click();
    const svg = await svgDetails(image);
    expect(svg.foreignObjects).toBe(0);
    expect(svg.activeElements).toBe(0);
    expect(svg.eventAttributes).toEqual([]);
    expect(svg.labels.length).toBeGreaterThan(0);
  }
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  await page.goto('knowledge/dx9-impl-09/');
  const actualDiagram = page.locator('.mermaid-diagram');
  await expect(actualDiagram).toHaveAttribute('data-diagram-state', 'ready');
  const actualImage = actualDiagram.locator('.mermaid-canvas img');
  await expect.poll(() => actualImage.evaluate((node: HTMLImageElement) => node.naturalWidth)).toBeGreaterThan(0);
  await expectWholeDiagramFits(actualImage);
  await actualImage.scrollIntoViewIfNeeded();
  await expect(actualImage).toBeInViewport({ ratio: 1 });
  await actualDiagram.screenshot({ path: testInfo.outputPath('dx9-09-mermaid-whole-flow.png') });
  expect(errors).toEqual([]);
});

test('every existing knowledge Mermaid source renders as a loaded diagram', async ({ page, isMobile }, testInfo) => {
  test.setTimeout(120_000);
  // 문법·SVG 호환은 같은 Chromium에서 한 번 전수 검사합니다. 크기·확대·접근성은 위·아래 테스트에서 모바일도 검사합니다.
  test.skip(isMobile, 'The complete source corpus is checked on desktop; mobile interaction and layout have separate coverage.');
  const corpus = catalog.documents.filter((document) => document.kind === 'knowledge').flatMap((entry) => {
    const document = JSON.parse(readFileSync(`public/${entry.file}`, 'utf8')) as Document;
    return document.sections.flatMap((section) => mermaidBlocks(section.blocks).map((code, sectionIndex) => ({
      route: entry.route, section: section.id, sectionIndex: sectionIndex + 1, code,
    })));
  });
  expect(corpus.length, 'The existing migration contains at least 75 Mermaid blocks, including collapsed history and source sections.').toBeGreaterThanOrEqual(75);
  testInfo.annotations.push({ type: 'corpus', description: `${corpus.length} original Mermaid blocks from every knowledge document and tab` });
  const fixtureMeta = catalog.documents.find((document) => document.route === '/knowledge/dx9-impl-09/')!;
  const document = JSON.parse(readFileSync(`public/${fixtureMeta.file}`, 'utf8')) as Document;
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  await patchDocument(page, {
    ...document,
    sections: [{ id: 'mermaid-corpus', title: '기존 Mermaid 전체 호환 검사', tab: 'document', blocks: corpus.map(({ code }) => ({ type: 'code', language: 'mermaid', text: code })) }],
  }, fixtureMeta.file);
  const diagrams = page.locator('.mermaid-diagram');
  await expect(diagrams).toHaveCount(corpus.length);
  await expect.poll(async () => diagrams.evaluateAll((nodes, origins) => nodes.flatMap((node, index) => {
    const state = node.getAttribute('data-diagram-state');
    const image = node.querySelector<HTMLImageElement>('.mermaid-canvas img');
    if (state === 'ready' && image?.complete && image.naturalWidth > 0) return [];
    return [{
      index: index + 1,
      ...origins[index],
      state,
      message: (node.querySelector('.mermaid-error pre')?.textContent || (state === 'ready' ? 'SVG image has not loaded' : 'Rendering is pending')).slice(0, 1200),
    }];
  }), corpus.map(({ route, section, sectionIndex }) => ({ route, section, sectionIndex }))), {
    timeout: 90_000,
    intervals: [250, 500, 1000],
    message: 'Every original Mermaid block must become ready and load its SVG. Failures identify the corpus index, source route, section, and renderer error.',
  }).toEqual([]);
  expect(errors).toEqual([]);
});

test('diagram zoom supports reading, scale controls, Escape and focus return', async ({ page }) => {
  await fixture(page, [example]);
  const diagram = page.locator('.mermaid-diagram');
  await expect(diagram).toHaveAttribute('data-diagram-state', 'ready');
  await expect(diagram.locator('.mermaid-canvas img')).toHaveAttribute('alt', '버퍼 사용 순서. CPU 입력을 버퍼에 기록한 다음 GPU가 읽습니다.');
  const description = diagram.locator('.mermaid-description');
  await description.locator('summary').click();
  await expect(description.locator('p')).toHaveText('CPU 입력을 버퍼에 기록한 다음 GPU가 읽습니다.');
  const trigger = diagram.getByRole('button', { name: '도식 확대해서 보기' });
  await trigger.click();
  const dialog = page.getByRole('dialog', { name: '버퍼 사용 순서 확대' });
  await expect(dialog).toBeVisible();
  const image = dialog.locator('img');
  await expect.poll(() => image.evaluate((node: HTMLImageElement) => node.naturalWidth)).toBeGreaterThan(0);
  await expect(dialog.getByRole('button', { name: '화면에 맞춤', exact: true })).toHaveAttribute('aria-pressed', 'true');
  await dialog.getByRole('button', { name: '100%', exact: true }).click();
  const width = await image.evaluate((node) => node.getBoundingClientRect().width);
  await dialog.getByRole('button', { name: '150%', exact: true }).click();
  await expect(dialog.getByRole('button', { name: '150%', exact: true })).toHaveAttribute('aria-pressed', 'true');
  const enlarged = await image.evaluate((node) => node.getBoundingClientRect().width);
  expect(enlarged).toBeGreaterThan(width * 1.45);
  expect(enlarged).toBeLessThan(width * 1.55);
  await expect(dialog.getByRole('link', { name: '도식 저장', exact: true })).toHaveAttribute('download', 'atlas-note-diagram.svg');
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  await page.keyboard.press('Escape');
  await expect(dialog).toHaveCount(0);
  await expect(trigger).toBeFocused();
  await expect(diagram.locator('.mermaid-source')).toHaveJSProperty('open', false);
});

test('duplicate diagrams have separate IDs and release images when their tab unmounts', async ({ page }) => {
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  await auditObjectUrls(page);
  await fixture(page, [example, example]);
  const diagrams = page.locator('.mermaid-diagram');
  await expect(page.locator('.mermaid-diagram[data-diagram-state="ready"]')).toHaveCount(2);
  const ids = await Promise.all([0, 1].map(async (index) => (await svgDetails(diagrams.nth(index).locator('.mermaid-canvas img'))).id));
  expect(ids.every((id) => id.startsWith('atlas-mermaid-'))).toBe(true);
  expect(new Set(ids).size).toBe(2);
  const urls = await page.evaluate(() => (window as unknown as UrlAuditWindow).__mermaidUrls.created.slice());
  expect(urls).toHaveLength(2);
  await page.getByRole('tab', { name: '실습·복습', exact: true }).click();
  await expect(diagrams).toHaveCount(0);
  await expect.poll(() => page.evaluate((urls) => urls.every((url) => (window as unknown as UrlAuditWindow).__mermaidUrls.revoked.includes(url)), urls)).toBe(true);
  await page.getByRole('tab', { name: '개념과 원리', exact: true }).click();
  await expect(page.locator('.mermaid-diagram[data-diagram-state="ready"]')).toHaveCount(2);
  const newUrls = await diagrams.locator('.mermaid-canvas img').evaluateAll((nodes) => nodes.map((node) => (node as HTMLImageElement).src));
  expect(newUrls.every((url) => !urls.includes(url))).toBe(true);
  await expect(diagrams.locator('.mermaid-render-stage > *')).toHaveCount(0);
  expect(errors).toEqual([]);
});

test('leaving a tab before asynchronous rendering completes creates no stale images', async ({ page }) => {
  const errors: string[] = [];
  page.on('pageerror', (error) => errors.push(error.message));
  await auditObjectUrls(page, true);
  await fixture(page, [example]);
  await expect(page.locator('.mermaid-diagram')).toHaveAttribute('data-diagram-state', 'loading');
  await page.getByRole('tab', { name: '실습·복습', exact: true }).click();
  await expect(page.locator('.mermaid-diagram')).toHaveCount(0);
  await page.evaluate(() => (window as unknown as UrlAuditWindow).__releaseMermaidFonts?.());
  await expect(page.getByText('다른 탭의 학습 내용')).toBeVisible();
  await page.getByRole('tab', { name: '개념과 원리', exact: true }).click();
  await expect(page.locator('.mermaid-diagram')).toHaveAttribute('data-diagram-state', 'ready');
  const audit = await page.evaluate(() => (window as unknown as UrlAuditWindow).__mermaidUrls);
  expect(audit.created).toHaveLength(1);
  expect(audit.revoked).toHaveLength(0);
  await expect(page.locator('.mermaid-render-stage > *')).toHaveCount(0);
  expect(errors).toEqual([]);
});

test('invalid syntax is an explicit error with unchanged original text and a retry action', async ({ page }) => {
  const invalid = 'flowchart TD\nA["닫히지 않은 원문\n';
  await fixture(page, [invalid]);
  const diagram = page.locator('.mermaid-diagram');
  await expect(diagram).toHaveAttribute('data-diagram-state', 'error');
  await expect(diagram.getByRole('alert')).toContainText('도식을 표시하지 못했습니다.');
  await expect(diagram.getByRole('button', { name: '도식 확대해서 보기' })).toBeDisabled();
  const original = diagram.locator('.mermaid-source');
  await original.locator('summary').click();
  expect(await original.locator('code').textContent()).toBe(invalid);
  await diagram.getByRole('button', { name: '다시 시도' }).click();
  await expect(diagram).toHaveAttribute('data-diagram-state', 'error');
  await expect(diagram.locator('.mermaid-canvas img')).toHaveCount(0);
  expect(await original.locator('code').textContent()).toBe(invalid);
});

test('document directives cannot enable HTML labels or click handlers', async ({ page }) => {
  const dialogs: string[] = [];
  page.on('dialog', (dialog) => { dialogs.push(dialog.message()); void dialog.dismiss(); });
  await page.addInitScript(() => {
    (window as unknown as Window & { mermaidUnsafeCallback: () => void }).mermaidUnsafeCallback = () => alert('unexpected diagram callback');
  });
  const guarded = '%%{init: {"securityLevel":"loose","htmlLabels":true,"flowchart":{"htmlLabels":true}}}%%\nflowchart TD\nA["<b>입력</b>"] --> B["결과"]\nclick A call mermaidUnsafeCallback()\n';
  await fixture(page, [guarded]);
  const diagram = page.locator('.mermaid-diagram');
  await expect(diagram).toHaveAttribute('data-diagram-state', 'ready');
  const svg = await svgDetails(diagram.locator('.mermaid-canvas img'));
  expect(svg.foreignObjects).toBe(0);
  expect(svg.activeElements).toBe(0);
  expect(svg.eventAttributes).toEqual([]);
  expect(svg.text).not.toContain('mermaidUnsafeCallback');
  await diagram.locator('.mermaid-canvas img').click();
  expect(dialogs).toEqual([]);
});

test('external image input is rejected before rendering and remains available as source', async ({ page }) => {
  const resourceRequests: string[] = [];
  await page.route('https://mermaid-resource.invalid/**', (route) => {
    resourceRequests.push(route.request().url());
    return route.abort();
  });
  const external = 'flowchart TD\nA@{ img: "https://mermaid-resource.invalid/diagram.svg", label: "외부 그림", h: 100 }\nA --> B["결과"]\n';
  await fixture(page, [external]);
  const diagram = page.locator('.mermaid-diagram');
  await expect(diagram).toHaveAttribute('data-diagram-state', 'error');
  await expect(diagram.getByRole('alert')).toContainText('도식을 표시하지 못했습니다.');
  await diagram.locator('.mermaid-error summary').click();
  await expect(diagram.locator('.mermaid-error pre')).toContainText('외부 리소스');
  await diagram.locator('.mermaid-source > summary').click();
  expect(await diagram.locator('.mermaid-source code').textContent()).toBe(external);
  expect(resourceRequests).toEqual([]);
  await expect(diagram.locator('.mermaid-render-stage > *')).toHaveCount(0);
});

test('knowledge illustrations switch from fit to real original size without overflowing the page', async ({ page, isMobile }) => {
  await fixture(page, [], [{
    type: 'image', src: '/knowledge/dx9-impl-09/assets/alpha-modes.svg',
    alt: '알파 테스트와 블렌딩 비교', caption: '같은 입력을 두 처리 방식으로 비교합니다.',
  }]);
  const figure = page.locator('.knowledge-illustration');
  await expect(figure).toBeVisible();
  const trigger = figure.getByRole('button', { name: '알파 테스트와 블렌딩 비교 확대' });
  await trigger.click();
  const dialog = page.getByRole('dialog', { name: '알파 테스트와 블렌딩 비교' });
  const viewport = dialog.locator('.knowledge-image-viewport');
  const image = viewport.locator('img');
  await expect.poll(() => image.evaluate((node: HTMLImageElement) => node.naturalWidth)).toBeGreaterThanOrEqual(640);
  await expect(dialog.getByRole('button', { name: '화면에 맞춤', exact: true })).toHaveAttribute('aria-pressed', 'true');
  await dialog.getByRole('button', { name: '원본 크기', exact: true }).click();
  await expect(dialog.getByRole('button', { name: '원본 크기', exact: true })).toHaveAttribute('aria-pressed', 'true');
  const sizes = await image.evaluate((node: HTMLImageElement) => ({ width: node.getBoundingClientRect().width, natural: node.naturalWidth }));
  expect(sizes.width).toBeCloseTo(sizes.natural, 0);
  await expect(viewport).toHaveCSS('overflow-x', 'auto');
  if (isMobile) expect(await viewport.evaluate((node) => node.scrollWidth > node.clientWidth)).toBe(true);
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(true);
  await dialog.getByRole('button', { name: '화면에 맞춤', exact: true }).click();
  expect(await image.evaluate((node) => node.getBoundingClientRect().width <= node.parentElement!.clientWidth)).toBe(true);
  await expect(dialog.getByRole('link', { name: '원본 이미지 저장' })).toHaveAttribute('href', /alpha-modes\.svg$/);
  await page.keyboard.press('Escape');
  await expect(dialog).toHaveCount(0);
  await expect(trigger).toBeFocused();
});
