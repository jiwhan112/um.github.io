import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';
import {readFile} from 'node:fs/promises';
import type {Block,Document} from '../src/lib/types';
const catalog=JSON.parse(readFileSync('../knowledge/data/catalog.json','utf8')) as {path:string;title:string;category:string}[];
const groups=[{category:'컴퓨터 구조',pattern:/^computer-0[12]$/},{category:'운영체제',pattern:/^os-0[1-6]$/},{category:'네트워크',pattern:/^network-0[1-5]$/}];
const chapters=catalog.filter(doc=>groups.some(group=>group.pattern.test(doc.path)));
const labels={document:'개념과 원리',practice:'실습·복습',sources:'출처·검증',history:'원문·변경기록'};
function flatten(blocks:Block[]):Block[]{return blocks.flatMap(block=>[block,...('blocks'in block?flatten(block.blocks):block.type==='list'?block.items.flatMap(flatten):[])]);}
test('CS curriculum exposes exactly the authored approved chapters',async({page})=>{
 await page.goto('knowledge/');
 const area=page.locator('#knowledge-cs');
 await area.getByRole('button',{name:/컴퓨터 과학/}).click();
 for(const group of groups){
  const expected=chapters.filter(doc=>group.pattern.test(doc.path));
  for(const doc of expected)expect(doc.category).toBe(group.category);
  const topic=area.locator('.knowledge-topic').filter({has:page.getByText(group.category,{exact:true})});
  await expect(topic).toHaveCount(1);
  expect(await topic.locator('.knowledge-document-link').evaluateAll(nodes=>nodes.map(node=>node.getAttribute('href')))).toEqual(expected.map(doc=>'/um.github.io/knowledge/'+doc.path+'/'));
 }
});
for(const chapter of chapters)test(`${chapter.path} CS visual lesson, original files and tab navigation`,async({page,request},testInfo)=>{
 const authored=JSON.parse(readFileSync('../knowledge/'+chapter.path+'/content.json','utf8')) as Document;
 const errors:string[]=[];page.on('pageerror',error=>errors.push(error.message));
 await page.goto('knowledge/'+chapter.path+'/');
 await expect(page.locator('main h1')).toHaveText(chapter.title);
 await expect(page.getByRole('tab')).toHaveText(Object.values(labels));
 const blocks=authored.sections.filter(s=>s.tab==='document').flatMap(s=>flatten(s.blocks));
 const imageCount=blocks.filter(b=>b.type==='image').length;expect(imageCount).toBeGreaterThanOrEqual(2);
 const images=page.locator('.knowledge-illustration img');await expect(images).toHaveCount(imageCount);
 for(const image of await images.all()){await image.scrollIntoViewIfNeeded();await expect.poll(()=>image.evaluate((node:HTMLImageElement)=>node.complete&&node.naturalWidth>0)).toBe(true);}
 const diagrams=page.locator('.mermaid-diagram');const diagramCount=blocks.filter(b=>b.type==='code'&&b.language==='mermaid').length;
 expect(diagramCount).toBeGreaterThanOrEqual(1);await expect(diagrams).toHaveCount(diagramCount);
 for(const diagram of await diagrams.all()){await diagram.scrollIntoViewIfNeeded();await expect(diagram).toHaveAttribute('data-diagram-state','ready');}
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.screenshot({path:testInfo.outputPath(chapter.path+'-concepts.png'),fullPage:true});
 await page.getByRole('tab',{name:labels.practice,exact:true}).click();
 const reviewSection=authored.sections.find(s=>s.id==='review')!;expect(reviewSection.tab).toBe('practice');
 const reviewCount=flatten(reviewSection.blocks).filter(b=>b.type==='details').length;expect(reviewCount).toBeGreaterThanOrEqual(6);
 const reviews=page.locator('#review details');await expect(reviews).toHaveCount(reviewCount);
 for(const review of await reviews.all())await expect(review).toHaveJSProperty('open',false);
 await reviews.first().locator(':scope > summary').click();await expect(reviews.first()).toHaveJSProperty('open',true);
 await reviews.first().locator(':scope > summary').click();await expect(reviews.first()).toHaveJSProperty('open',false);
 const full=page.locator('#full-code');await expect(full).toBeVisible();
 for(const details of await full.locator('details').all())if(!await details.evaluate(n=>(n as HTMLDetailsElement).open))await details.locator(':scope > summary').click();
 const sourceLinks=full.locator('a[href]');expect(await sourceLinks.count()).toBeGreaterThan(0);
 for(const link of await sourceLinks.all()){
  const href=await link.getAttribute('href');expect(href).toMatch(/^\/um\.github\.io\/knowledge\/[^?#]+\.(?:c|cpp|h|hpp|py|sh|txt|json)$/);
  const source=await readFile('..'+href!.replace('/um.github.io',''),'utf8'),response=await request.get(href!);
  expect(response.status()).toBe(200);expect(await response.text()).toBe(source);expect(await full.locator('pre code').allTextContents()).toContain(source);
 }
 await expect(page.locator('#example-downloads a[href$=".zip"]')).toBeVisible();
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.screenshot({path:testInfo.outputPath(chapter.path+'-practice.png'),fullPage:true});
 await page.getByRole('tab',{name:labels.sources,exact:true}).click();
 const verification=page.locator('a[href$="verification.json"]').first();await expect(verification).toBeVisible();
 const verificationHref=await verification.getAttribute('href');
 expect(verificationHref).toMatch(/^\/um\.github\.io\/knowledge\/[a-zA-Z0-9_./-]+\/verification\.json$/);
 expect(verificationHref!.split('/')).not.toContain('..');
 const verificationResponse=await request.get(verificationHref!);expect(verificationResponse.status()).toBe(200);
 expect(await verificationResponse.body()).toEqual(await readFile('..'+verificationHref!.replace('/um.github.io','')));
 await page.getByRole('tab',{name:labels.history,exact:true}).click();
 const historySections=authored.sections.filter(section=>section.tab==='history');expect(historySections.length).toBeGreaterThan(0);
 for(const section of historySections){
  expect(section.blocks.length).toBeGreaterThan(0);
  const history=page.locator('#'+section.id);await expect(history).toBeVisible();
  await expect(history.locator('p').first()).toBeVisible();
  expect((await history.innerText()).trim().length).toBeGreaterThan(20);
 }
 // The new CS lessons publish technical evidence, not private-source attribution.
 const authoredText=JSON.stringify(authored);
 expect(authoredText).not.toMatch(/notion|노션/i);
 expect(authoredText).not.toMatch(/(?<![a-f0-9])[a-f0-9]{32}(?![a-f0-9])|[a-f0-9]{8}(?:-[a-f0-9]{4}){3}-[a-f0-9]{12}/i);
 expect(authoredText).not.toMatch(/\/(?:workspace|home|tmp|Users)\/|[A-Z]:[\\/]Users[\\/]|source-note\.md|(?:raw|source)-capture\.json/i);
 // Reload a direct section hash, then real Back/Forward across a separate route.
 await page.goto('knowledge/'+chapter.path+'/#review');await expect(page.locator('#review')).toBeVisible();
 await page.goto('knowledge/');await page.goBack();await expect(page.locator('#review')).toBeVisible();
 await page.goForward();await expect(page.locator('.knowledge-tree')).toBeVisible();
 expect(errors).toEqual([]);
});
