import migration from './fixtures/thinking-pair-migration.json' with {type:'json'};
import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';
import type {Catalog,Document,Block} from '../src/lib/types';
const catalog=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
const docs=catalog.documents.filter(d=>d.kind==='thinking'&&!d.number?.startsWith('1.1.'));
function nested(bs:Block[]):Block[]{return bs.flatMap(b=>[b,...('blocks'in b?nested(b.blocks):b.type==='list'?b.items.flatMap(nested):[])]);}
for(const meta of docs)test(`paired reasoning ${meta.number} retains one page and worked diagrams`,async({page},testInfo)=>{
 const doc=JSON.parse(readFileSync('../'+meta.source,'utf8')) as Document;
 const errors:string[]=[];page.on('pageerror',e=>errors.push(e.message));
 await page.addInitScript(({key})=>localStorage.setItem(key,'preserved independent reasoning note'),{key:meta.memoKey!});
 await page.goto(meta.route.replace(/^\//,''));
 await expect(page.locator('main h1')).toHaveText(doc.title);
 await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue('preserved independent reasoning note');
 await page.getByRole('tab',{name:'힌트',exact:true}).click();
 const hintSections=doc.sections.filter(s=>s.tab==='hints');
 const hints=page.locator(hintSections.map(s=>'#'+s.id+' details').join(','));
 await expect(hints).toHaveCount(3);for(const hint of await hints.all())await expect(hint).not.toHaveAttribute('open','');
 await page.getByRole('tab',{name:'풀이·검증',exact:true}).click();
 const solutionSections=doc.sections.filter(s=>s.tab==='solution');
 const scope=page.locator(solutionSections.map(s=>'#'+s.id).join(','));
 await expect(scope.locator('details[open]')).toHaveCount(0);
 // Expand authored explanations to exercise all diagrams rather than merely count JSON.
 for(const disclosure of await scope.locator('details.disclosure').all()){
  if(await disclosure.getAttribute('open')===null)await disclosure.locator(':scope > summary').click();
 }
 const expected=nested(solutionSections.flatMap(s=>s.blocks)).filter(b=>b.type==='code'&&b.language==='mermaid').length;
 expect(expected).toBeGreaterThanOrEqual(2);
 const diagrams=scope.locator('.mermaid-diagram');await expect(diagrams).toHaveCount(expected);
 for(let i=0;i<expected;++i){
  const diagram=diagrams.nth(i);await diagram.scrollIntoViewIfNeeded();
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
