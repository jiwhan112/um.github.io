import { test, expect } from '@playwright/test';
import { readFile } from 'node:fs/promises';
const slugs=['c-basics-01','c-basics-02','c-basics-03'];
for(const [index,slug] of slugs.entries())test(`${slug} opens tabs, images, review answers and exact C sources`,async({page,request},testInfo)=>{
 const errors:string[]=[];page.on('pageerror',e=>errors.push(e.message));
 await page.goto(`knowledge/${slug}/`);
 await expect(page.locator('main h1')).toContainText(`C 기초 0${index+1}`);
 await expect(page.getByRole('tab')).toHaveText(['개념과 원리','실습·복습','출처·검증','원문·변경기록']);
 await expect(page.locator('.knowledge-illustration img')).toHaveCount(2);
 for(const img of await page.locator('.knowledge-illustration img').all()){
  await img.scrollIntoViewIfNeeded();
  await expect.poll(()=>img.evaluate((n:HTMLImageElement)=>n.complete&&n.naturalWidth>0)).toBe(true);
 }
 for(const diagram of await page.locator('.mermaid-diagram').all()){
  await diagram.scrollIntoViewIfNeeded();await expect(diagram).toHaveAttribute('data-diagram-state','ready');
 }
 const adjacent=page.getByRole('navigation',{name:'이전 다음 문서'});
 const expected=[index?slugs[index-1]:null,slugs[index+1]??'c-basics-04'].filter(Boolean).map(s=>`/um.github.io/knowledge/${s}/`);
 expect(await adjacent.getByRole('link').evaluateAll(nodes=>nodes.map(n=>n.getAttribute('href')))).toEqual(expected);
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.evaluate(()=>document.fonts.ready);
 await page.screenshot({path:testInfo.outputPath(`${slug}-concepts.png`),fullPage:true});
 await page.getByRole('tab',{name:'실습·복습'}).click();
 const answers=page.locator('#review details');await expect(answers).toHaveCount(6);
 for(const answer of await answers.all())await expect(answer).toHaveJSProperty('open',false);
 await answers.first().locator('summary').click();await expect(answers.first().locator('p')).toBeVisible();
 const full=page.locator('#full-code details');await full.locator('summary').click();
 const source=await readFile(`../knowledge/${slug}/main.c`,'utf8');
 await expect(full.locator('pre code')).toHaveText(source);
 const response=await request.get(`/um.github.io/knowledge/${slug}/main.c`);
 expect(response.ok()).toBe(true);expect(await response.text()).toBe(source);
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.screenshot({path:testInfo.outputPath(`${slug}-practice.png`),fullPage:true});
 await page.getByRole('tab',{name:'출처·검증'}).click();
 const report=await request.get('/um.github.io/knowledge/shared/c-basics-lab/verification.json');
 expect(report.ok()).toBe(true);expect((await report.json()).status).toBe('pass');
 await expect(page.locator('a[href$="verification.json"]')).toBeVisible();
 await page.getByRole('tab',{name:'원문·변경기록'}).click();
 await expect(page.locator('.prose')).toContainText('Notion 원문 이관본이 아니며');
 expect(errors).toEqual([]);
});
test('C curriculum includes five regular chapters before applied C01',async({page})=>{
 await page.goto('knowledge/');
 const topic=page.locator('#knowledge-cpp .knowledge-topic').filter({has:page.getByText('C 언어',{exact:true})});
 await expect(topic.locator('.knowledge-topic-row')).toContainText('6개 공개');
 expect(await topic.locator('.knowledge-document-link').evaluateAll(nodes=>nodes.map(n=>n.getAttribute('href')))).toEqual([...slugs,'c-basics-04','c-basics-05','c-language-01'].map(s=>`/um.github.io/knowledge/${s}/`));
 for(const slug of slugs){
  await topic.locator(`a[href="/um.github.io/knowledge/${slug}/"]`).click();
  await expect(page.locator('main h1')).toContainText('C 기초');
  await page.getByRole('link',{name:'프로그래밍 학습 목차',exact:true}).click();
 }
});
