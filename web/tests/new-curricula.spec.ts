import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';
import {readFile} from 'node:fs/promises';
import type {Block,Document,DocumentMeta} from '../src/lib/types';
import {compareKnowledgeDocuments} from '../src/lib/knowledge-order';
const catalog=JSON.parse(readFileSync('../knowledge/data/catalog.json','utf8')) as {path:string;title:string;category:string}[];
const groups=[
 {area:'cpp',category:'Modern C++',pattern:/^modern-cpp-\d{2}$/},
 {area:'cpp',category:'Effective Modern C++',pattern:/^effective-modern-cpp-\d{2}$/},
 {area:'windows',category:'DirectX 12',pattern:/^dx12-\d{2}$/},
 {area:'graphics',category:'그래픽스 기술',pattern:/^graphics-tech-\d{2}$/},
 {area:'algorithm',category:'알고리즘',pattern:/^algorithm-core-\d{2}$/}
];
const chapters=catalog.filter(doc=>groups.some(group=>group.pattern.test(doc.path)));
const labels={document:'개념과 원리',practice:'실습·복습',sources:'출처·검증',history:'원문·변경기록'};
function flatten(blocks:Block[]):Block[]{return blocks.flatMap(block=>[block,...('blocks'in block?flatten(block.blocks):block.type==='list'?block.items.flatMap(flatten):[])]);}
function links(value:unknown):string[]{
 if(Array.isArray(value))return value.flatMap(links);
 if(!value||typeof value!=='object')return [];
 const object=value as Record<string,unknown>;
 return [...(typeof object.href==='string'?[object.href]:[]),...Object.values(object).flatMap(links)];
}
test('new curriculum categories expose all authored chapters in study order',async({page})=>{
 await page.goto('knowledge/');await page.getByRole('button',{name:'모두 펼치기'}).click();
 for(const group of groups){
  const expected=catalog.filter(doc=>doc.category===group.category).sort((a,b)=>compareKnowledgeDocuments({category:a.category,route:'/knowledge/'+a.path+'/'} as DocumentMeta,{category:b.category,route:'/knowledge/'+b.path+'/'} as DocumentMeta));
  const area=page.locator('#knowledge-'+group.area);
  const actual=await area.locator('.knowledge-topic').filter({has:page.locator('.knowledge-document-link').filter({hasText:expected[0]?.title??'__not_authored__'})}).locator('.knowledge-document-link').evaluateAll(nodes=>nodes.map(node=>node.getAttribute('href')));
  expect(actual).toEqual(expected.map(doc=>'/um.github.io/knowledge/'+doc.path+'/'));
 }
 // The two excluded tracks must not receive newly written curriculum chapters.
 await expect(page.locator('#knowledge-engine .knowledge-document-link')).toHaveCount(0);
 await expect(page.locator('#knowledge-python .knowledge-document-link')).toHaveCount(0);
});
for(const chapter of chapters)test(`${chapter.path} original course diagrams, code, review and sources`,async({page,request},testInfo)=>{
 const authored=JSON.parse(readFileSync('../knowledge/'+chapter.path+'/content.json','utf8')) as Document;
 const errors:string[]=[];page.on('pageerror',error=>errors.push(error.message));
 await page.goto('knowledge/'+chapter.path+'/');
 await expect(page.locator('main h1')).toHaveText(chapter.title);
 await expect(page.getByRole('tab')).toHaveText(Object.values(labels));
 const concepts=authored.sections.filter(s=>s.tab==='document').flatMap(s=>flatten(s.blocks));
 const pictures=concepts.filter(b=>b.type==='image');expect(pictures.length).toBeGreaterThanOrEqual(2);
 const images=page.locator('.knowledge-illustration img');await expect(images).toHaveCount(pictures.length);
 for(const image of await images.all()){await image.scrollIntoViewIfNeeded();await expect.poll(()=>image.evaluate((node:HTMLImageElement)=>node.complete&&node.naturalWidth>0)).toBe(true);}
 const diagrams=page.locator('.mermaid-diagram');const count=concepts.filter(b=>b.type==='code'&&b.language==='mermaid').length;
 expect(count).toBeGreaterThanOrEqual(1);await expect(diagrams).toHaveCount(count);
 for(const diagram of await diagrams.all()){await diagram.scrollIntoViewIfNeeded();await expect(diagram).toHaveAttribute('data-diagram-state','ready');}
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.screenshot({path:testInfo.outputPath(chapter.path+'-concepts.png'),fullPage:true});
 await page.getByRole('tab',{name:labels.practice,exact:true}).click();
 const review=authored.sections.find(s=>s.id==='review')!;expect(review.tab).toBe('practice');
 const expectedReviews=flatten(review.blocks).filter(b=>b.type==='details').length;expect(expectedReviews).toBeGreaterThanOrEqual(6);
 const reviews=page.locator('#review details');await expect(reviews).toHaveCount(expectedReviews);
 for(const details of await reviews.all())await expect(details).toHaveJSProperty('open',false);
 await reviews.first().locator(':scope > summary').click();await expect(reviews.first()).toHaveJSProperty('open',true);
 await reviews.first().locator(':scope > summary').click();await expect(reviews.first()).toHaveJSProperty('open',false);
 const full=page.locator('#full-code');await expect(full).toBeVisible();
 for(const details of await full.locator('details').all())if(!await details.evaluate(node=>(node as HTMLDetailsElement).open))await details.locator(':scope > summary').click();
 const sourceLinks=full.locator('a[href]');expect(await sourceLinks.count()).toBeGreaterThan(0);
 const displayed=await full.locator('pre code').allTextContents();
 for(const link of await sourceLinks.all()){
  const href=(await link.getAttribute('href'))!;
  expect(href).toMatch(/^\/um\.github\.io\/knowledge\/[^?#]+\.(?:cpp|h|hpp|hlsl|txt|json)$/);
  const source=await readFile('..'+href.replace('/um.github.io',''),'utf8');
  const response=await request.get(href);expect(response.status()).toBe(200);expect(await response.text()).toBe(source);
  expect(displayed).toContain(source);
 }
 if(chapter.path.startsWith('dx12-')){
  // The shared complete Windows projects remain downloadable. Chapter-specific API
  // excerpts are not falsely asserted to be the complete renderer or a GPU test.
  const windows=page.locator('#windows-source');await expect(windows).toBeVisible();
  await expect(windows).toContainText(/미검증|실행하지/);
  const completeFiles=windows.locator('a[href*="/knowledge/shared/dx12-core-lab/project/"]');
  expect(await completeFiles.count()).toBeGreaterThanOrEqual(4);
  for(const link of await completeFiles.all()){
   const href=(await link.getAttribute('href'))!;
   const response=await request.get(href);expect(response.status()).toBe(200);
   expect(await response.body()).toEqual(await readFile('..'+href.replace('/um.github.io','')));
  }
 }
 await expect(page.locator('#example-downloads a[href$=".zip"]')).toBeVisible();
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.screenshot({path:testInfo.outputPath(chapter.path+'-practice.png'),fullPage:true});
 await page.getByRole('tab',{name:labels.sources,exact:true}).click();
 const evidence=[...new Set(links(authored.sections.filter(s=>s.tab==='sources')).filter(h=>h.endsWith('/verification.json')))];expect(evidence.length).toBeGreaterThan(0);
 for(const href of evidence){
  expect(href).toMatch(/^\/knowledge\/[a-zA-Z0-9_./-]+\/verification\.json$/);expect(href.split('/')).not.toContain('..');
  await expect(page.locator(`a[href="/um.github.io${href}"]`).first()).toBeVisible();
  const response=await request.get('/um.github.io'+href);expect(response.status()).toBe(200);expect(await response.body()).toEqual(await readFile('..'+href));
 }
 await page.getByRole('tab',{name:labels.history,exact:true}).click();
 const history=authored.sections.filter(s=>s.tab==='history');expect(history.length).toBeGreaterThan(0);
 for(const section of history)await expect(page.locator('#'+section.id)).toBeVisible();
 expect(JSON.stringify(authored)).not.toMatch(/\/(?:workspace|home|tmp|Users)\/|[A-Z]:[\\/]Users[\\/]|source-note\.md|(?:raw|source)-capture\.json|notion\.com|notion\.so/i);
 await page.goto('knowledge/'+chapter.path+'/#review');await expect(page.locator('#review')).toBeVisible();
 await page.goto('knowledge/');await page.goBack();await expect(page.locator('#review')).toBeVisible();
 await page.goForward();await expect(page.locator('.knowledge-tree')).toBeVisible();
 expect(errors).toEqual([]);
});
