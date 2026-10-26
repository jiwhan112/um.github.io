/** Exact public source bundles and representative native browser downloads; never executes examples. */
import {chromium,expect} from '@playwright/test';
import {readFile,writeFile,mkdir} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {verifyRouteWithTransientRetry} from './live-navigation.mjs';
const base='https://jiwhan112.github.io/um.github.io/';
const expected=process.env.EXPECTED_SOURCE;if(!expected)throw Error('EXPECTED_SOURCE is required');
const manifest=JSON.parse(await readFile('../knowledge/examples/catalog.json','utf8'));
const hash=bytes=>createHash('sha256').update(bytes).digest('hex');
const report={expectedSource:expected,resources:[],downloads:[],notExecuted:['Downloaded programs','Windows/MSVC','GPU/HLSL','Physical mobile device']};
await mkdir('live-check',{recursive:true});
async function get(path){
 for(let attempt=0;attempt<3;attempt++){
  const response=await fetch(base+path,{cache:'no-store',signal:AbortSignal.timeout(30000)});
  if([502,503,504].includes(response.status)&&attempt<2){await response.arrayBuffer();await new Promise(resolve=>setTimeout(resolve,1000*(attempt+1)));continue;}
  expect(response.status,path).toBe(200);return Buffer.from(await response.arrayBuffer());
 }
 throw Error('Unreachable bounded request state');
}
const info=JSON.parse((await get('build-info.json?examples='+Date.now())).toString());
expect(info.sourceCommit).toBe(expected);
// Bounded parallel requests, exact final bytes. A 404 or wrong hash always fails.
for(let start=0;start<manifest.bundles.length;start+=5){
 await Promise.all(manifest.bundles.slice(start,start+5).map(async bundle=>{
  const bytes=await get(bundle.path);expect(hash(bytes),bundle.id).toBe(bundle.sha256);
  expect(bytes.equals(await readFile('../'+bundle.path)),bundle.id).toBe(true);
  report.resources.push({id:bundle.id,path:bundle.path,bytes:bytes.length,sha256:hash(bytes)});
 }));
}
const representativeIds=['c-basics-13','cpp-basics-lab','cpp-advanced-lab','dx9-impl-08','dx11-17-reviewed','python-html'];
for(const id of ['computer-01','os-01','network-01','modern-cpp-01','effective-modern-cpp-01','dx12-01','graphics-tech-01','algorithm-core-01'])if(manifest.bundles.some(bundle=>bundle.id===id))representativeIds.push(id);
const browser=await chromium.launch();
try{
 for(const width of [1440,390]){
  const context=await browser.newContext({viewport:{width,height:width===390?844:1000},isMobile:width===390,hasTouch:width===390,locale:'ko-KR',serviceWorkers:'block',acceptDownloads:true});
  try{
   for(const id of representativeIds){
    const bundle=manifest.bundles.find(item=>item.id===id);expect(bundle,'representative '+id).toBeDefined();
    const slug=bundle.pages[0],url=base+'knowledge/'+slug+'/';
    const result=await verifyRouteWithTransientRetry(context,url,async(page,errors)=>{
     const response=await page.goto(url);expect(response?.status()).toBe(200);
     await expect(page.locator('main h1')).toBeVisible();
     await page.getByRole('tab',{name:'실습·복습',exact:true}).click();
     const link=page.locator('#example-downloads a[href="/um.github.io/'+bundle.path+'"]');
     await expect(link).toBeVisible();
     expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
     await page.screenshot({path:`live-check/example-${id}-${width}.png`});
     const before=page.url();
     const [download]=await Promise.all([page.waitForEvent('download'),link.click()]);
     expect(download.url()).toBe(base+bundle.path);expect(await download.failure()).toBeNull();
     const path=await download.path();expect(path).not.toBeNull();
     const bytes=await readFile(path);expect(hash(bytes)).toBe(bundle.sha256);
     expect(page.url()).toBe(before);expect(errors).toEqual([]);
     return {id,slug,width,url:download.url(),bytes:bytes.length,sha256:hash(bytes)};
    },{onFailure:async({page,history,retry})=>{
     await writeFile(`live-check/example-${id}-${width}-diagnostics.json`,JSON.stringify({retryCandidate:retry,attempts:history},null,2));
     await page.screenshot({path:`live-check/example-${id}-${width}-failure.png`});
    }});
    report.downloads.push(result);
   }
  }finally{await context.close();}
 }
}finally{await browser.close();}
report.resources.sort((a,b)=>a.id.localeCompare(b.id));
await writeFile('live-check/example-downloads.json',JSON.stringify(report,null,2));
console.log('EXAMPLE DOWNLOADS PUBLIC VERIFIED:',report.resources.length,'exact ZIPs;',report.downloads.length,'desktop/mobile native downloads; source',expected);
