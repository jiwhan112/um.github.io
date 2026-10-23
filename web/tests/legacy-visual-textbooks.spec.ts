import {test,expect} from '@playwright/test';
import {readFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import rollout from './fixtures/legacy-visual-rollout.json' with {type:'json'};
import type {Block,Document} from '../src/lib/types';
type Chapter = (typeof rollout.chapters)[number] & {sourceArchives?:{path:string;sha256:string}[];sourceViews?:{sourceLibrary:string;sourceTab:string;sourceSection:string;displayedCodeFiles:string[]}[]};
const chapters:Chapter[]=rollout.chapters;
const labels={document:'개념과 원리',practice:'실습·복습',sources:'출처·검증',history:'원문·변경기록'};
function nested(blocks:Block[]):Block[]{return blocks.flatMap(b=>[b,...('blocks'in b?nested(b.blocks):b.type==='list'?b.items.flatMap(nested):[])]);}
async function expandContent(page:import('@playwright/test').Page){
 // Open authored disclosure ancestry in document order; Mermaid source controls stay untouched.
 for(const details of await page.locator('main .prose details.disclosure').all()){
  if(!await details.evaluate(node=>(node as HTMLDetailsElement).open))await details.locator(':scope > summary').click();
 }
}
for(const chapter of chapters)test(`${chapter.slug} every approved visual tab and source is usable`,async({page,request},testInfo)=>{
 test.setTimeout(120_000);
 const errors:string[]=[];page.on('pageerror',error=>errors.push(error.message));
 const authored=JSON.parse(await readFile(`../knowledge/${chapter.slug}/content.json`,'utf8')) as Document;
 await page.goto(`knowledge/${chapter.slug}/`);
 await expect(page.locator('main h1')).toHaveText(authored.title);
 await expect(page.getByRole('tab')).toHaveText(Object.values(labels));
 let totalImages=0,totalDiagrams=0;
 for(const [tab,label] of Object.entries(labels)){
  await page.getByRole('tab',{name:label,exact:true}).click();
  const sections=authored.sections.filter(section=>section.tab===tab);expect(sections.length).toBeGreaterThan(0);
  await expect(page.locator('#'+sections[0].id)).toBeVisible();
  if(tab==='practice'){
   const sourceReview=sections.find(section=>section.id===chapter.reviewSection)!;
   const expected=sourceReview.blocks.filter(block=>block.type==='details').length;expect(expected).toBeGreaterThanOrEqual(6);
   const reviews=page.locator('#'+chapter.reviewSection+' > details.disclosure');
   await expect(reviews).toHaveCount(expected);
   for(const review of await reviews.all()){
    await expect(review).toHaveJSProperty('open',false);await review.locator(':scope > summary').click();
    await expect(review).toHaveJSProperty('open',true);await expect(review.locator('p,table,pre').first()).toBeVisible();
    await review.locator(':scope > summary').click();await expect(review).toHaveJSProperty('open',false);
   }
  }
  for(const disclosure of await page.locator('main .prose details.disclosure').all())await expect(disclosure).toHaveJSProperty('open',false);
  await expandContent(page);
  const blocks=nested(sections.flatMap(section=>section.blocks));
  const images=page.locator('main .knowledge-illustration img');
  const expectedImages=blocks.filter(block=>block.type==='image');await expect(images).toHaveCount(expectedImages.length);
  for(const image of expectedImages){expect(image.alt.trim()).not.toBe('');expect(image.caption?.trim()).toBeTruthy();}
  for(const image of await images.all()){
   await image.scrollIntoViewIfNeeded();await expect(image).toHaveAttribute('alt',/\S/);
   await expect.poll(()=>image.evaluate((node:HTMLImageElement)=>node.complete&&node.naturalWidth>0)).toBe(true);
   expect(await image.evaluate(async(node:HTMLImageElement)=>{try{await node.decode();return true;}catch{return false;}})).toBe(true);
  }
  const diagrams=page.locator('main .mermaid-diagram');
  const expectedDiagrams=blocks.filter(block=>block.type==='code'&&block.language==='mermaid');await expect(diagrams).toHaveCount(expectedDiagrams.length);
  for(const diagram of await diagrams.all()){
   await diagram.scrollIntoViewIfNeeded();await expect(diagram).toHaveAttribute('data-diagram-state','ready');
   await expect.poll(()=>diagram.locator('.mermaid-canvas img').evaluate((node:HTMLImageElement)=>node.complete&&node.naturalWidth>0)).toBe(true);
  }
  totalImages+=expectedImages.length;totalDiagrams+=expectedDiagrams.length;
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
  // Full source libraries can exceed image dimensions: capture a viewport artifact there,
  // while every authored image/diagram and document overflow is still checked above.
  const fullPage=await page.evaluate(()=>document.documentElement.scrollHeight<20000);
  await page.screenshot({path:testInfo.outputPath(`${chapter.slug}-${tab}.png`),fullPage});
 }
 expect(totalImages).toBe(chapter.images);expect(totalDiagrams).toBeGreaterThan(0);
 const archives=chapter.sourceArchives??[{path:chapter.sourceFile,sha256:chapter.sourceSha256}];
 for(const archive of archives){
  const original=await request.get('/um.github.io/'+archive.path);expect(original.ok()).toBe(true);
  expect(createHash('sha256').update(await original.body()).digest('hex')).toBe(archive.sha256);
 }
 for(const href of chapter.koFiles){
  const disk=await readFile('..'+href);const response=await request.get('/um.github.io'+href);expect(response.ok()).toBe(true);
  expect(await response.body()).toEqual(disk);
 }
 // Explicit displayed-code mapping keeps metadata downloads distinct and supports multiple tabs/libraries.
 const views=chapter.sourceViews??[{sourceLibrary:chapter.sourceLibrary??chapter.slug,sourceTab:'practice' as const,sourceSection:chapter.slug==='winapi-04a'?'github-source':'file-1',displayedCodeFiles:chapter.koFiles.filter(href=>/(?:\.(?:cpp|hpp|h|rc|manifest)$|\/CMakeLists\.txt$)/.test(href))}];
 for(const view of views){
  await page.goto(`knowledge/${view.sourceLibrary}/`);
  expect(Object.keys(labels)).toContain(view.sourceTab);
  await page.getByRole('tab',{name:labels[view.sourceTab as keyof typeof labels],exact:true}).click();
  await expect(page.locator('#'+view.sourceSection)).toBeVisible();await expandContent(page);
  const rendered=await page.locator(chapter.sourceViews?'#'+view.sourceSection+' pre code':'main pre code').allTextContents();
  for(const href of view.displayedCodeFiles){
   expect(chapter.koFiles).toContain(href);
   expect(rendered,'complete displayed source '+href).toContain(await readFile('..'+href,'utf8'));
  }
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 }
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 expect(errors).toEqual([]);
});
