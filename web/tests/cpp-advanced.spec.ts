import { test, expect } from '@playwright/test';
import { createHash } from 'node:crypto';

const chapters=[
  {slug:'cpp-advanced-01',sources:['lifetime.cpp','check.hpp'],entry:'lifetime.cpp',previous:'cpp-basics-05',next:'cpp-advanced-02'},
  {slug:'cpp-advanced-02',sources:['ownership.cpp','transaction.cpp'],entry:'ownership.cpp',previous:'cpp-advanced-01',next:null},
];

for(const chapter of chapters)test(`${chapter.slug} renders lifetime or ownership visuals and connects exact full sources`,async({page,request},testInfo)=>{
  const errors:string[]=[];
  page.on('pageerror',error=>errors.push(error.message));
  await page.goto(`knowledge/${chapter.slug}/`);
  await expect(page.getByRole('heading',{level:1})).toContainText('C++ 심화');
  await expect(page.getByRole('tab')).toHaveText(['개념과 원리','실습·복습','출처·검증','원문·변경기록']);
  await expect(page.locator('.knowledge-illustration')).toHaveCount(3);
  for(const [index,figure] of (await page.locator('.knowledge-illustration').all()).entries()){
    await figure.scrollIntoViewIfNeeded();
    await expect.poll(()=>figure.locator('img').evaluate((node:HTMLImageElement)=>node.complete&&node.naturalWidth>0)).toBe(true);
    await figure.screenshot({path:testInfo.outputPath(`${chapter.slug}-figure-${index+1}.png`)});
  }
  const diagrams=page.locator('.mermaid-diagram');
  expect(await diagrams.count()).toBeGreaterThan(0);
  for(const [index,diagram] of (await diagrams.all()).entries()){
    await diagram.scrollIntoViewIfNeeded();
    await expect(diagram).toHaveAttribute('data-diagram-state','ready');
    await expect(diagram.locator('.mermaid-source')).toHaveJSProperty('open',false);
    await expect.poll(()=>diagram.locator('.mermaid-canvas img').evaluate((node:HTMLImageElement)=>node.complete&&node.naturalWidth>0)).toBe(true);
    if(chapter.slug==='cpp-advanced-01'){
      const labels=await diagram.locator('.mermaid-canvas img').evaluate(async(node:HTMLImageElement)=>{
        const svg=new DOMParser().parseFromString(await fetch(node.src).then(response=>response.text()),'image/svg+xml');
        return [...svg.querySelectorAll('text')].map(label=>label.textContent).join(' ');
      });
      expect(labels).toContain('피해량이 음수인가?');
      expect(labels).toContain('피해량이 현재 체력 이상?');
    }
    await diagram.screenshot({path:testInfo.outputPath(`${chapter.slug}-mermaid-${index+1}.png`)});
  }
  const adjacent=page.getByRole('navigation',{name:'이전 다음 문서'});
  expect(await adjacent.getByRole('link').evaluateAll(nodes=>nodes.map(node=>node.getAttribute('href')))).toEqual(
    [chapter.previous,chapter.next].filter(Boolean).map(slug=>`/um.github.io/knowledge/${slug}/`),
  );
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
  await page.evaluate(()=>document.fonts.ready);
  await page.evaluate(()=>scrollTo(0,0));
  await page.screenshot({path:testInfo.outputPath(`${chapter.slug}-first-screen.png`)});
  await page.screenshot({path:testInfo.outputPath(`${chapter.slug}-lesson.png`),fullPage:true});
  await page.getByRole('tab',{name:'실습·복습'}).click();
  const question=page.locator('#review .disclosure').first();
  await expect(question).toHaveJSProperty('open',false);
  await question.locator('summary').click();
  await expect(question).toHaveJSProperty('open',true);
  await expect(question.locator('p').first()).toBeVisible();
  await page.getByRole('tab',{name:'출처·검증'}).click();
  await expect(page.locator('.prose')).toContainText('C++11');
  for(const file of chapter.sources){
    const path=`/um.github.io/knowledge/shared/cpp-advanced-lab/ko/${file}`;
    await expect(page.locator(`a[href="${path}"]`).first()).toBeVisible();
    const response=await request.get(path);
    expect(response.ok()).toBe(true);
    expect(await response.text()).toMatch(/\/\/.*[가-힣]/);
    expect((await request.get(`/um.github.io/knowledge/shared/cpp-advanced-lab/original/src/${file}`)).ok()).toBe(true);
  }
  const fullSource=page.locator('.disclosure').filter({has:page.locator('summary').filter({hasText:chapter.entry})}).first();
  await expect(fullSource).toHaveJSProperty('open',false);
  await fullSource.locator('summary').click();
  await expect(fullSource.locator('pre code')).toBeVisible();
  await expect(fullSource.locator('pre code')).toContainText('int main(');
  await expect(fullSource.locator('pre code')).toContainText(/\/\/.*[가-힣]/);
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
  await fullSource.locator('summary').scrollIntoViewIfNeeded();
  await page.screenshot({path:testInfo.outputPath(`${chapter.slug}-korean-source.png`)});
  await fullSource.locator('summary').click();
  const zip=await request.get('/um.github.io/knowledge/shared/cpp-advanced-lab/archive/cpp_advanced_effective_lab_v1.zip');
  expect(zip.ok()).toBe(true);
  expect(createHash('sha256').update(await zip.body()).digest('hex')).toBe('a97a4927e5d3472ad055dbffbad0e14ab72c4c81f6ea13a1d72b771d64d339ba');
  const capture=await request.get(`/um.github.io/knowledge/${chapter.slug}/source-notion-capture.json`);
  expect(capture.ok()).toBe(true);
  expect((await capture.json()).sha256).toMatch(/^[a-f0-9]{64}$/);
  await page.getByRole('tab',{name:'원문·변경기록'}).click();
  await expect(page.locator('.prose')).toContainText('Notion');
  await expect(page.locator(`a[href="/um.github.io/knowledge/${chapter.slug}/source-notion.md"]`).first()).toBeVisible();
  expect(errors).toEqual([]);
});
