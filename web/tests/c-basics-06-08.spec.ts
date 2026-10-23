import {test,expect} from '@playwright/test';
import {readFile} from 'node:fs/promises';
const slugs=['c-basics-06','c-basics-07','c-basics-08'];
for(const [index,slug] of slugs.entries())test(`${slug} C11 four tabs, original source and diagrams`,async({page,request})=>{
 const errors:string[]=[];page.on('pageerror',e=>errors.push(e.message));
 await page.goto(`knowledge/${slug}/`);
 await expect(page.locator('main h1')).toContainText(`C 기초 0${index+6}`);
 await expect(page.getByRole('tab')).toHaveText(['개념과 원리','실습·복습','출처·검증','원문·변경기록']);
 await expect(page.locator('.knowledge-illustration img')).toHaveCount(2);
 for(const image of await page.locator('.knowledge-illustration img').all()){
  await image.scrollIntoViewIfNeeded();
  await expect.poll(()=>image.evaluate((node:HTMLImageElement)=>node.complete&&node.naturalWidth>0)).toBe(true);
 }
 const mermaids=page.locator('.mermaid-diagram');await expect(mermaids).toHaveCount(1);
 await mermaids.first().scrollIntoViewIfNeeded();await expect(mermaids.first()).toHaveAttribute('data-diagram-state','ready');
 await page.getByRole('tab',{name:'실습·복습'}).click();
 await expect(page.locator('#review details')).toHaveCount(6);
 const full=page.locator('#full-code details');await full.locator('summary').click();
 const source=await readFile(`../knowledge/${slug}/main.c`,'utf8');
 await expect(full.locator('pre code')).toHaveText(source);
 const response=await request.get(`/um.github.io/knowledge/${slug}/main.c`);
 expect(response.ok()).toBe(true);expect(await response.text()).toBe(source);
 await page.getByRole('tab',{name:'출처·검증'}).click();
 await expect(page.locator('a[href$="verification.json"]')).toBeVisible();
 await page.getByRole('tab',{name:'원문·변경기록'}).click();
 await expect(page.locator('.prose')).toContainText('Notion 이관 원문이 없으며');
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 expect(errors).toEqual([]);
});
