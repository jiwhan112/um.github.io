import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';

// Separate basic/advanced pages preserve the old route, memo and all four diagrams.
// No snapshot update or relaxation of the site's existing gates.
for(const variant of [
 {number:'1.1.1',path:'ep-01-01-is-unique',key:'thinking-ctci-1-1-draft',heading:'1.1.1 기본 — 같은 글자가 두 번 나왔을까?',summary:'기본 정답 · 기억 표를 그림으로 따라가기'},
 {number:'1.1.2',path:'ep-01-01-02-is-unique-advanced',key:'thinking-ctci-1-1-2-draft',heading:'1.1.2 심화 — 가장 먼저 다시 나온 글자 찾기',summary:'심화 정답 · 최초 위치 표와 두 번의 순회'},
]) test(`thinking ${variant.number} has one problem and preserves independent memo`, async({page},testInfo)=>{
 const source=JSON.parse(readFileSync(`../thinking-lab/${variant.path}/content.json`,'utf8'));
 expect(source.memoKey).toBe(variant.key);
 await page.addInitScript(()=>{
  localStorage.setItem('thinking-ctci-1-1-draft','// preserved basic memo');
  localStorage.setItem('thinking-ctci-1-1-2-draft','// independent advanced memo');
 });
 const errors:string[]=[];
 page.on('pageerror',error=>errors.push(error.message));
 await page.goto(`thinking-lab/${variant.path}/`);
 await expect(page.getByRole('heading',{name:variant.heading,exact:true}).last()).toBeVisible();
 await expect(page.locator('#section-2 h3')).toHaveCount(1);
 const contract=page.locator('#section-2 details');
 await expect(contract).toHaveCount(1);
 await expect(contract).not.toHaveAttribute('open','');
 await expect(contract.locator('summary')).toHaveText('코드로 풀 때의 입력 조건');
 await contract.locator('summary').click();
 await expect(contract).toHaveAttribute('open','');
 await contract.locator('summary').click();
 await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue(variant.number==='1.1.1'?'// preserved basic memo':'// independent advanced memo');
 await page.getByRole('tab',{name:'힌트',exact:true}).click();
 await expect(page.locator('#section-3 details')).toHaveCount(3);
 await expect(page.locator('#section-3 details[open]')).toHaveCount(0);
 await page.getByRole('tab',{name:'풀이·검증',exact:true}).click();
 const disclosure=page.locator('#section-4 details').filter({has:page.locator('summary').filter({hasText:variant.summary})}).first();
 await expect(page.locator('#section-4 details[open]')).toHaveCount(0);
 await expect(disclosure).not.toHaveAttribute('open','');
 await disclosure.locator(':scope > summary').click();
 const diagrams=page.locator('#section-4 .mermaid-diagram');
 await expect(diagrams).toHaveCount(2);
 for(let i=0;i<2;++i){
  const diagram=diagrams.nth(i);
  await diagram.scrollIntoViewIfNeeded();
  await expect(diagram).toHaveAttribute('data-diagram-state','ready');
  await expect.poll(()=>diagram.locator('.mermaid-canvas img').evaluate((image:HTMLImageElement)=>image.complete&&image.naturalWidth>0)).toBe(true);
  await expect(diagram.locator('.mermaid-source')).toHaveJSProperty('open',false);
  await diagram.screenshot({path:testInfo.outputPath(`thinking-1-1-diagram-${variant.number}-${i+1}.png`)});
 }
 await diagrams.first().getByRole('button',{name:'도식 확대해서 보기'}).click();
 await expect(page.getByRole('dialog')).toBeVisible();
 await expect(page.getByRole('dialog').locator('img')).toBeVisible();
 await page.keyboard.press('Escape');
 await expect(page.getByRole('dialog')).toHaveCount(0);
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1-1-draft'))).toBe('// preserved basic memo');
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1-1-2-draft'))).toBe('// independent advanced memo');
 await page.getByRole('tab',{name:'확장·다음',exact:true}).click();
 const sibling=page.locator('#section-6 a').first();
 await expect(sibling).toHaveAttribute('href',variant.number==='1.1.1'?'/um.github.io/thinking-lab/ep-01-01-02-is-unique-advanced/':'/um.github.io/thinking-lab/ep-01-01-is-unique/');
 await sibling.click();
 await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue(variant.number==='1.1.1'?'// independent advanced memo':'// preserved basic memo');
 await page.goto('thinking-lab/?q=1.1');
 const cards=page.getByTestId('library-card');
 await expect(cards).toHaveCount(2);
 await expect(cards.nth(0)).toContainText('1.1.1');
 await expect(cards.nth(1)).toContainText('1.1.2');
 expect(errors).toEqual([]);
});
