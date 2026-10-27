import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';

// Additive coverage for the user's 1.1-only two-problem content prototype.
// No snapshot update or relaxation of the site's existing gates.
test('thinking 1.1 two problems preserve memo and render four worked diagrams', async({page},testInfo)=>{
 const source=JSON.parse(readFileSync('../thinking-lab/ep-01-01-is-unique/content.json','utf8'));
 expect(source.memoKey).toBe('thinking-ctci-1-1-draft');
 await page.addInitScript(()=>localStorage.setItem('thinking-ctci-1-1-draft','// preserved first-lesson memo'));
 const errors:string[]=[];
 page.on('pageerror',error=>errors.push(error.message));
 await page.goto('thinking-lab/ep-01-01-is-unique/');
 await expect(page.getByRole('heading',{name:'문제 1 · 기본 — 스킬 슬롯 코드 검사',exact:true})).toBeVisible();
 await expect(page.getByRole('heading',{name:'문제 2 · 확장 — 충돌 위치를 알려 주는 편집기',exact:true})).toBeVisible();
 await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue('// preserved first-lesson memo');
 await page.getByRole('tab',{name:'힌트',exact:true}).click();
 await expect(page.locator('#section-3 details')).toHaveCount(6);
 await expect(page.locator('#section-3 details[open]')).toHaveCount(0);
 await page.getByRole('tab',{name:'풀이·검증',exact:true}).click();
 // Find the two authored disclosures by their exact summaries rather than wrapper markup.
 const basic=page.locator('details').filter({has:page.locator('summary').filter({hasText:'문제 1 정답 · 기억 표를 그림으로 따라가기'})}).first();
 const expanded=page.locator('details').filter({has:page.locator('summary').filter({hasText:'문제 2 정답 · 최초 위치 표와 두 번의 순회'})}).first();
 await expect(basic).not.toHaveAttribute('open','');
 await expect(expanded).not.toHaveAttribute('open','');
 await basic.locator(':scope > summary').click();
 await expanded.locator(':scope > summary').click();
 const diagrams=page.locator('#section-4 .mermaid-diagram');
 await expect(diagrams).toHaveCount(4);
 for(let i=0;i<4;++i){
  const diagram=diagrams.nth(i);
  await diagram.scrollIntoViewIfNeeded();
  await expect(diagram).toHaveAttribute('data-diagram-state','ready');
  await expect.poll(()=>diagram.locator('.mermaid-canvas img').evaluate((image:HTMLImageElement)=>image.complete&&image.naturalWidth>0)).toBe(true);
  await expect(diagram.locator('.mermaid-source')).toHaveJSProperty('open',false);
  await diagram.screenshot({path:testInfo.outputPath(`thinking-1-1-diagram-${i+1}.png`)});
 }
 await diagrams.first().getByRole('button',{name:'도식 확대해서 보기'}).click();
 await expect(page.getByRole('dialog')).toBeVisible();
 await expect(page.getByRole('dialog').locator('img')).toBeVisible();
 await page.keyboard.press('Escape');
 await expect(page.getByRole('dialog')).toHaveCount(0);
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1-1-draft'))).toBe('// preserved first-lesson memo');
 expect(errors).toEqual([]);
});
