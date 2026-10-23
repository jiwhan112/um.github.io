/** Public-origin smoke tests, separate from deterministic CI fixture screenshots. */
import {chromium,expect} from '@playwright/test';
import {gotoWithTransientRetry,verifyRouteWithTransientRetry} from './live-navigation.mjs';
import {readFile,mkdir,writeFile} from 'node:fs/promises';
const base='https://jiwhan112.github.io/um.github.io/';
const expected=process.env.EXPECTED_SOURCE;if(!expected)throw Error('EXPECTED_SOURCE is required');
await mkdir('live-check',{recursive:true});
let info;for(let i=0;i<40;i++){
 try{const r=await fetch(base+'build-info.json?check='+Date.now(),{cache:'no-store'});if(r.ok){info=await r.json();if(info.framework==='React'&&info.sourceCommit===expected)break;}}catch{}
 if(i===39)throw Error('Published source commit was not observed on the Pages origin');
 await new Promise(r=>setTimeout(r,10000));
}
const browser=await chromium.launch();const context=await browser.newContext({viewport:{width:1440,height:1000},locale:'ko-KR',timezoneId:'Asia/Seoul',serviceWorkers:'block'});
const routes=JSON.parse(await readFile('dist/data/atlas/routes.json','utf8'));const results=[];
const shots=new Map([['/thinking-lab/','library'],['/','dashboard'],['/daily-briefing/','briefing'],['/language-study/japanese/9/','japanese'],['/thinking-lab/ep-01-04-palindrome-permutation/','thinking'],['/ai-asset-lab/','gallery']]);
try{
 for(const path of routes){
  console.log('Checking public route:',path);
  const result=await verifyRouteWithTransientRetry(context,base+path.slice(1),async(page,errors)=>{
  const response=await page.goto(base+path.slice(1));expect(response?.status()).toBe(200);
  await expect(page.locator('[data-framework=react]')).toBeVisible();await expect(page.locator('main h1').first()).toBeVisible({timeout:30000});
  expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);expect(errors).toEqual([]);
  expect(await page.locator('script[src*="assets/atlas/detail"]').count()).toBe(0);await expect(page.locator('html')).toHaveAttribute('data-theme','dark');await expect(page.locator('.app-shell')).toHaveCSS('background-color','rgb(16, 19, 25)');
  if(shots.has(path)){await page.evaluate(()=>document.fonts.ready);await page.screenshot({path:'live-check/'+shots.get(path)+'.png'});}
  return {path,status:response.status(),title:await page.locator('main h1').first().innerText(),errors};
  },{onFailure:async({page,history,retry})=>{
   const name=path.replace(/[^a-zA-Z0-9_-]/g,'_')||'root';
   await writeFile(`live-check/${name}-diagnostics.json`,JSON.stringify({retryCandidate:retry,attempts:history},null,2));
   await page.screenshot({path:`live-check/${name}-failure-${history.length}.png`});
  }});
  results.push(result);
 }
 const liveCatalog=JSON.parse(await readFile('dist/data/atlas/catalog.json','utf8'));const matching=liveCatalog.documents.filter(d=>d.kind==='thinking'&&d.tags.includes('회문'));
 const library=await context.newPage();await gotoWithTransientRetry(library,base+'thinking-lab/?tag='+encodeURIComponent('회문'));await expect(library.getByTestId('library-card')).toHaveCount(matching.length);await library.getByRole('searchbox',{name:'문제 검색'}).fill('1.4');await library.reload();await expect(library.getByRole('searchbox',{name:'문제 검색'})).toHaveValue('1.4');await expect(library.getByTestId('library-card').first()).toBeVisible();await library.close();
 const page=await context.newPage();await gotoWithTransientRetry(page,base+'language-study/#japanese/9');await expect(page).toHaveURL(/language-study\/japanese\/9\//);await page.reload();await expect(page.getByRole('tab',{name:'문법',exact:true})).toBeVisible();await page.close();
 const mobile=await browser.newContext({viewport:{width:390,height:844},isMobile:true,hasTouch:true,locale:'ko-KR',serviceWorkers:'block'});const mp=await mobile.newPage();await gotoWithTransientRetry(mp,base+'thinking-lab/ep-01-04-palindrome-permutation/');await expect(mp.locator('main h1')).toBeVisible();await mp.getByRole('button',{name:'사이트 메뉴 열기'}).click();await expect(mp.getByRole('dialog')).toBeVisible();await mp.screenshot({path:'live-check/mobile-navigation.png'});await mobile.close();
 await writeFile('live-check/results.json',JSON.stringify({base,info,routes:results,legacyHash:true,mobileNavigation:true,allDark:true,libraryFilter:true},null,2));
 console.log('PUBLIC PAGES VERIFIED:',results.length,'React routes; source',expected);
}finally{await browser.close();}
await import('./check-c01-live.mjs');
