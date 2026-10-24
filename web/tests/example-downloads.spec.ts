import {test,expect,type Page} from '@playwright/test';
import {readFileSync,existsSync} from 'node:fs';
import {readFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import type {Catalog,Document} from '../src/lib/types';

const hash=(bytes:Buffer)=>createHash('sha256').update(bytes).digest('hex');
const catalog=JSON.parse(readFileSync('../knowledge/data/catalog.json','utf8')) as {path:string}[];
type ExampleLink={slug:string;tab:string;section:string;href:string};
const links:ExampleLink[]=[];
function collect(value:unknown,context:Omit<ExampleLink,'href'>){
 if(Array.isArray(value)){for(const child of value)collect(child,context);return;}
 if(!value||typeof value!=='object')return;
 const object=value as Record<string,unknown>;
 if(typeof object.href==='string'&&object.href.startsWith('/knowledge/'))links.push({...context,href:object.href});
 for(const child of Object.values(object))collect(child,context);
}
for(const {path:slug} of catalog){
 const path=`../knowledge/${slug}/content.json`;if(!existsSync(path))continue;
 const content=JSON.parse(readFileSync(path,'utf8')) as {sections:{id:string;tab:string;blocks:unknown[]}[]};
 for(const section of content.sections)collect(section.blocks,{slug,tab:section.tab,section:section.id});
}
const tabLabels:Record<string,string>={document:'개념과 원리',practice:'실습·복습',sources:'출처·검증',history:'원문·변경기록'};
async function clickAndVerifyDownload(page:Page,example:ExampleLink,expectedHash:string){
 await page.goto(`knowledge/${example.slug}/`);
 await page.getByRole('tab',{name:tabLabels[example.tab],exact:true}).click();
 const href='/um.github.io'+example.href;
 const link=page.locator(`#${example.section}`).locator(`a[href="${href}"]`).first();
 await expect(link).toHaveCount(1);
 // Reveal authored collapsed containers through their real controls, not DOM mutation.
 for(const details of await link.locator('xpath=ancestor::details').all()){
  if(!await details.evaluate(node=>(node as HTMLDetailsElement).open))await details.locator(':scope > summary').click();
 }
 await expect(link).toBeVisible();
 const before=page.url();
 const [download]=await Promise.all([page.waitForEvent('download'),link.click()]);
 expect(download.url()).toBe(new URL(href,before).href);
 expect(await download.failure()).toBeNull();
 const path=await download.path();expect(path).not.toBeNull();
 expect(hash(await readFile(path!))).toBe(expectedHash);
 // The preview serves teaching files as octet-stream: a React route interception
 // would change the URL and never emit this byte-verified native download.
 expect(page.url()).toBe(before);
}
for(const extension of ['py','rc','manifest','fx']){
 const example=links.find(link=>link.href.endsWith('.'+extension));
 if(!example)throw Error('Missing authored representative for native .'+extension+' link regression');
 test(`native ${extension} example link downloads original bytes`,async({page,request})=>{
  const bytes=await readFile('..'+example.href);
  const response=await request.get('/um.github.io'+example.href);
  expect(response.status()).toBe(200);expect(hash(await response.body())).toBe(hash(bytes));
  await clickAndVerifyDownload(page,example,hash(bytes));
 });
}
test('native sh teaching link downloads without executing or routing it',async({page,request})=>{
 const compiled=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
 const meta=compiled.documents.find(document=>document.route==='/knowledge/c-basics-04/')!;
 const document=JSON.parse(readFileSync('public/'+meta.file,'utf8')) as Document;
 const href='/knowledge/shared/c-basics-04-05/verify.sh';
 // An actual checked-in teaching script, exposed through a typed-content fixture.
 // No shell code is run: both assertions compare only downloaded bytes.
 document.sections.push({id:'download-regression',title:'다운로드 검증',tab:'sources',blocks:[{type:'paragraph',content:[{kind:'link',text:'검증 스크립트 원문',href}]}]});
 await page.route(`**/${meta.file}`,route=>route.fulfill({json:document}));
 const bytes=await readFile('..'+href),response=await request.get('/um.github.io'+href);
 expect(response.status()).toBe(200);expect(hash(await response.body())).toBe(hash(bytes));
 await clickAndVerifyDownload(page,{slug:'c-basics-04',tab:'sources',section:'download-regression',href},hash(bytes));
});
const manifestPath='../knowledge/examples/catalog.json';
const manifest=existsSync(manifestPath)?JSON.parse(readFileSync(manifestPath,'utf8')) as {version:number;bundles:{id:string;path:string;sha256:string;pages?:string[]}[]}:null;
test('allowlisted example bundles serve their recorded exact bytes',async({request})=>{
  expect(manifest,'approved bundle manifest must exist').not.toBeNull();
  expect(manifest!.version).toBe(1);expect(manifest!.bundles.length).toBeGreaterThan(0);
  expect([...new Set(manifest!.bundles.flatMap(b=>b.pages??[]))].sort()).toEqual(catalog.map(d=>d.path).sort());
  for(const bundle of manifest!.bundles){
   const bytes=await readFile('../'+bundle.path);expect(hash(bytes)).toBe(bundle.sha256);
   const response=await request.get('/um.github.io/'+bundle.path);
   expect(response.status(),bundle.path).toBe(200);expect(hash(await response.body()),bundle.path).toBe(bundle.sha256);
   expect(links.some(link=>link.href==='/'+bundle.path),'authored download link '+bundle.id).toBe(true);
  }
 });
test('example ZIP link triggers a real byte-verified browser download',async({page})=>{
  expect(manifest,'approved bundle manifest must exist').not.toBeNull();
  expect(manifest!.bundles.length).toBeGreaterThan(0);
  const bundle=manifest!.bundles[0],example=links.find(link=>link.href==='/'+bundle.path);
  expect(example,'manifest ZIP must be linked from an authored page').toBeDefined();
  await clickAndVerifyDownload(page,example!,bundle.sha256);
 });


for(const {path:slug} of catalog)test(`example downloads are discoverable on ${slug}`,async({page})=>{
 expect(manifest,'approved bundle manifest must exist').not.toBeNull();
 const bundles=manifest!.bundles.filter(bundle=>bundle.pages?.includes(slug));
 expect(bundles.length,'bundle mapping '+slug).toBeGreaterThan(0);
 await page.goto(`knowledge/${slug}/`);
 await expect(page.locator('main h1')).toBeVisible();
 for(const bundle of bundles){
  const example=links.find(link=>link.slug===slug&&link.href==='/'+bundle.path);
  expect(example,'authored per-page download link '+slug).toBeDefined();
  await page.getByRole('tab',{name:tabLabels[example!.tab],exact:true}).click();
  const link=page.locator(`#${example!.section} a[href="/um.github.io/${bundle.path}"]`).first();
  await expect(link).toHaveCount(1);
  for(const details of await link.locator('xpath=ancestor::details').all()){
   if(!await details.evaluate(node=>(node as HTMLDetailsElement).open))await details.locator(':scope > summary').click();
  }
  await expect(link).toBeVisible();
 }
});
