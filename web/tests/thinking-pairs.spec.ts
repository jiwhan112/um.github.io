import migration from './fixtures/thinking-pair-migration.json' with {type:'json'};
import {test,expect} from '@playwright/test';
import {readFileSync,writeFileSync} from 'node:fs';
import type {Catalog,Document,Block} from '../src/lib/types';
const catalog=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
const docs=catalog.documents.filter(d=>d.kind==='thinking'&&!d.number?.startsWith('1.1.'));
test.afterEach(async({page},testInfo)=>{
 if(testInfo.status===testInfo.expectedStatus)return;
 try{
  const state=await page.evaluate(()=>({url:location.href,activeElement:document.activeElement?.outerHTML,
   selectedTab:document.querySelector('[role=tab][aria-selected=true]')?.textContent,
   disclosures:[...document.querySelectorAll('details.disclosure')].map(d=>({title:d.querySelector('summary')?.textContent,open:d.hasAttribute('open')})),
   diagrams:[...document.querySelectorAll('.mermaid-diagram')].map(d=>({state:d.getAttribute('data-diagram-state'),visible:(d as HTMLElement).checkVisibility(),rect:d.getBoundingClientRect().toJSON(),ancestors:(()=>{const a=[];for(let p=d.parentElement;p;p=p.parentElement)if(p.tagName==='DETAILS')a.push({title:p.querySelector('summary')?.textContent,open:p.hasAttribute('open')});return a;})()}))}));
  writeFileSync(testInfo.outputPath('paired-error-state.json'),JSON.stringify(state,null,2));
 }catch{/* Preserve the original test failure even if its browser was already closed. */}
});

function nested(bs:Block[]):Block[]{return bs.flatMap(b=>[b,...('blocks'in b?nested(b.blocks):b.type==='list'?b.items.flatMap(nested):[])]);}
for(const meta of docs)test(`paired reasoning ${meta.number} retains one page and worked diagrams`,async({page},testInfo)=>{
 const doc=JSON.parse(readFileSync('../'+meta.source,'utf8')) as Document;
 const errors:string[]=[];page.on('pageerror',e=>errors.push(e.message));
 await page.addInitScript(({key})=>localStorage.setItem(key,'preserved independent reasoning note'),{key:meta.memoKey!});
 await page.goto(meta.route.replace(/^\//,''));
 await expect(page.locator('main h1')).toHaveText(doc.title);
 await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue('preserved independent reasoning note');
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.screenshot({path:testInfo.outputPath(`thinking-pair-page-${meta.number}.png`),fullPage:true});
 await page.getByRole('tab',{name:'힌트',exact:true}).click();
 const hintSections=doc.sections.filter(s=>s.tab==='hints');
 const hints=page.locator(hintSections.map(s=>'#'+s.id+' details').join(','));
 await expect(hints).toHaveCount(3);for(const hint of await hints.all())await expect(hint).not.toHaveAttribute('open','');
 await page.getByRole('tab',{name:'풀이·검증',exact:true}).click();
 const solutionSections=doc.sections.filter(s=>s.tab==='solution');
 const scope=page.locator(solutionSections.map(s=>'#'+s.id).join(','));
 await expect(scope.locator('details[open]')).toHaveCount(0);
 const expected=nested(solutionSections.flatMap(s=>s.blocks)).filter(b=>b.type==='code'&&b.language==='mermaid').length;
 expect(expected).toBeGreaterThanOrEqual(2);
 const diagrams=scope.locator('.mermaid-diagram');await expect(diagrams).toHaveCount(expected);
 // Rendered images change layout asynchronously. Settle them before disclosure interaction.
 for(const diagram of await diagrams.all()){
  await expect(diagram).toHaveAttribute('data-diagram-state','ready');
  await expect.poll(()=>diagram.locator('.mermaid-canvas img').evaluate((img:HTMLImageElement)=>img.complete&&img.naturalWidth>0)).toBe(true);
 }
 // Keyboard activation is a real user action and avoids pointer targets moving after a diagram loads.
 for(const disclosure of await scope.locator('details.disclosure').all()){
  if(await disclosure.getAttribute('open')===null){
   await disclosure.locator(':scope > summary').focus();
   await disclosure.locator(':scope > summary').press('Enter');
  }
  await expect(disclosure).toHaveAttribute('open','');
 }
 for(let i=0;i<expected;++i){
  const diagram=diagrams.nth(i);await expect(diagram).toBeVisible();await diagram.scrollIntoViewIfNeeded();
  await expect(diagram).toHaveAttribute('data-diagram-state','ready');
  await expect.poll(()=>diagram.locator('.mermaid-canvas img').evaluate((img:HTMLImageElement)=>img.complete&&img.naturalWidth>0)).toBe(true);
  await expect(diagram.locator('.mermaid-source')).not.toHaveAttribute('open','');
  await diagram.screenshot({path:testInfo.outputPath(`thinking-pair-diagram-${meta.number}-${i+1}.png`)});
 }
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 expect(await page.evaluate(key=>localStorage.getItem(key),meta.memoKey!)).toBe('preserved independent reasoning note');
 expect(errors).toEqual([]);
});

for(const original of migration.topics.filter(t=>t.topic!=='1.1'))test(`original thinking ${original.topic} hash links select visible sections`,async({page})=>{
 for(const id of original.sectionIds){
  await page.goto('thinking-lab/'+original.path+'#'+id);
  await expect(page.locator('#'+id)).toBeVisible();
 }
});
