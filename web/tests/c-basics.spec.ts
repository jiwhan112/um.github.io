import {selectLearningGroup,expectLearningRoutes} from './learning-test-helpers';
import { test, expect } from '@playwright/test';
import { readFile } from 'node:fs/promises';
const slugs=['c-basics-01','c-basics-02','c-basics-03'];
const authored=JSON.parse(await readFile('../knowledge/data/catalog.json','utf8')) as {path:string;category:string}[];
const regular=authored.filter(entry=>entry.category==='C 언어'&&/^c-basics-\d+$/.test(entry.path))
 .map(entry=>entry.path).sort((a,b)=>Number(a.slice('c-basics-'.length))-Number(b.slice('c-basics-'.length)));
const course=[...regular.filter(slug=>Number(slug.slice('c-basics-'.length))<=11),'c-language-01',
 ...regular.filter(slug=>Number(slug.slice('c-basics-'.length))>11)];
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
 const position=course.indexOf(slug);
 const expected=[course[position-1],course[position+1]].filter(Boolean).map(s=>`/um.github.io/knowledge/${s}/`);
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
test('C curriculum includes every authored chapter with applied C01 after chapter 11',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'C 언어');
 await expectLearningRoutes(page,course.map(slug=>`/knowledge/${slug}/`));
 await selectLearningGroup(page,'C 언어');
 const topic=page.getByTestId('learning-current-group');
 for(const slug of slugs){
  await topic.locator(`a[href="/um.github.io/knowledge/${slug}/"]`).click();
  await expect(page.locator('main h1')).toContainText('C 기초');
  await page.getByRole('navigation',{name:'학습 위치',exact:true}).getByRole('link',{name:'학습 목차',exact:true}).click();
 }
});
