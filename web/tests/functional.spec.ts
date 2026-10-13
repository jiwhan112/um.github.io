import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';
import type {Catalog} from '../src/lib/types';
const catalog=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
const routes=JSON.parse(readFileSync('public/data/atlas/routes.json','utf8')) as string[];
for(const route of routes){test('direct route '+route,async({page,isMobile,request})=>{
 const errors:string[]=[];page.on('pageerror',e=>errors.push(e.message));
 const response=await request.get('/um.github.io'+route);expect(response.status()).toBe(200);expect(await response.text()).toContain('id="root"');expect(await response.text()).not.toContain('atlas-detail');
 await page.goto('/um.github.io'+route);await expect(page.locator('[data-framework=react]')).toBeVisible();await expect(page.locator('main h1').first()).toBeVisible();
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBeTruthy();
 if(!isMobile)await expect(page.locator('.sidebar')).toBeVisible();else await expect(page.getByRole('button',{name:'사이트 메뉴 열기'})).toBeVisible();
 expect(errors).toEqual([]);
});}
test('legacy language link redirects and survives refresh',async({page})=>{await page.goto('language-study/#japanese/9');await expect(page).toHaveURL(/\/language-study\/japanese\/9\//);await expect(page.getByRole('tab',{name:'문법',exact:true})).toBeVisible();await page.reload();await expect(page.getByRole('tab',{name:'문법',exact:true})).toBeVisible();});
test('old bookmarks and read flags remain, filter persists, other local data untouched',async({page,isMobile})=>{
 const id=catalog.dates[0]+':0';await page.addInitScript(({id})=>{localStorage.setItem('jiwhan-briefing-v1',JSON.stringify({saved:[id],read:[id]}));localStorage.setItem('unrelated-value','keep-me');},{id});await page.goto('daily-briefing/');if(isMobile)await page.locator('.article-list-item').first().click();await expect(page.getByRole('button',{name:'북마크됨',exact:true})).toBeVisible();await expect(page.getByRole('button',{name:'읽음 완료',exact:true})).toBeVisible();await page.getByRole('tab',{name:'간단 요약',exact:true}).click();await expect(page.locator('.summary-list')).toBeVisible();expect(await page.evaluate(()=>localStorage.getItem('unrelated-value'))).toBe('keep-me');
});
test('bookmark changes persist after reload',async({page,isMobile})=>{await page.goto('daily-briefing/');if(isMobile)await page.locator('.article-list-item').first().click();await page.getByRole('button',{name:'북마크',exact:true}).click();await page.reload();if(isMobile)await page.locator('.article-list-item').first().click();await expect(page.getByRole('button',{name:'북마크됨',exact:true})).toBeVisible();});
test('problem memo uses original key; solution remains closed',async({page})=>{
 const doc=catalog.documents.find(d=>d.number==='1.4'&&d.kind==='thinking')!;await page.addInitScript(()=>{if(localStorage.getItem('thinking-ctci-1.4-draft')===null)localStorage.setItem('thinking-ctci-1.4-draft','// original memo');});await page.goto('/um.github.io'+doc.route);await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue('// original memo');await page.getByRole('tab',{name:'풀이·검증',exact:true}).click();await expect(page.locator('details[open]')).toHaveCount(0);await page.getByLabel('C++11 풀이 작성').fill('// edited memo');await page.getByRole('button',{name:'풀이 저장',exact:true}).click();expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1.4-draft'))).toBe('// edited memo');await page.reload();await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue('// edited memo');
});
test('language controls, preserved progress and browser TTS arguments',async({page})=>{
 await page.addInitScript(()=>{localStorage.setItem('atlas-language-sections-v1',JSON.stringify({'japanese:9:lang-grammar':true}));(window as unknown as {__spoken:unknown[]}).__spoken=[];Object.defineProperty(window,'speechSynthesis',{configurable:true,value:{cancel(){},speak(u:SpeechSynthesisUtterance){(window as unknown as {__spoken:unknown[]}).__spoken.push({text:u.text,lang:u.lang,rate:u.rate});}}});});
 await page.goto('language-study/japanese/9/');await expect(page.getByRole('checkbox',{name:'문법',exact:true})).toBeChecked();await page.getByRole('tab',{name:'예문',exact:true}).click();await expect(page.locator('.translation')).toHaveCount(0);await page.getByRole('button',{name:'해석',exact:true}).first().click();await expect(page.locator('.translation').first()).toContainText('저는 물을 마십니다.');await page.getByRole('button',{name:'발음',exact:true}).first().click();await expect(page.locator('.pronunciation').first()).toContainText('와타시와');await page.getByRole('button',{name:'문장 1 듣기',exact:true}).click();expect(await page.evaluate(()=>(window as unknown as {__spoken:{lang:string}[]}).__spoken[0].lang)).toBe('ja-JP');
});
test('asset modal and report routes',async({page})=>{await page.goto('ai-asset-lab/');await page.getByRole('button',{name:'체력 포션 확대',exact:true}).click();await expect(page.getByRole('dialog')).toBeVisible();await expect(page.getByRole('dialog').locator('img')).toHaveJSProperty('naturalWidth',128);await page.keyboard.press('Escape');await expect(page.getByRole('dialog')).toHaveCount(0);await page.getByRole('button',{name:'3D 모델',exact:true}).click();await expect(page.getByText('이 분야의 결과물은 아직 없습니다.')).toBeVisible();});
test('search and mobile navigation',async({page,isMobile})=>{await page.goto('');await expect(page.locator('main h1')).toBeVisible();await page.keyboard.press('Control+k');await page.getByLabel('통합 검색').fill('Palindrome');await expect(page.locator('.search-results a').first()).toBeVisible();await page.locator('.search-results a').first().click();await expect(page).toHaveURL(/palindrome-permutation/);if(isMobile){await page.getByRole('button',{name:'사이트 메뉴 열기'}).click();await page.getByRole('dialog').getByRole('link',{name:'언어 학습',exact:true}).click();await expect(page).toHaveURL(/language-study/);}});
test('missing route is HTTP 404 with a helpful app screen',async({page})=>{const r=await page.goto('not-a-real-document/');expect(r?.status()).toBe(404);await expect(page.getByText('페이지를 찾을 수 없습니다.',{exact:true})).toBeVisible();});

test('lesson list and sidebar reflect all checked sections across refresh',async({page,isMobile})=>{
 await page.goto('language-study/japanese/9/');
 const steps=page.locator('.lesson-checks input[type="checkbox"]');
 await expect(steps.first()).toBeVisible();
 const count=await steps.count();expect(count).toBeGreaterThanOrEqual(4);
 for(let i=0;i<count;i++)await steps.nth(i).check();
 await expect(page.locator('.lesson-complete')).toContainText('학습 완료');
 await page.goto('language-study/?language=japanese');
 const lesson=page.locator('.lesson-list a[href$="/japanese/9/"]');
 await expect(lesson.locator('.lesson-status')).toContainText('완료');
 if(isMobile){await page.getByRole('button',{name:'사이트 메뉴 열기'}).click();await expect(page.getByRole('dialog').locator('.chapter-nav a[href$="/japanese/9/"] .chapter-complete')).toBeVisible();await page.keyboard.press('Escape');}else await expect(page.locator('.sidebar .chapter-nav a[href$="/japanese/9/"] .chapter-complete')).toBeVisible();
 await page.reload();
 await expect(page.locator('.lesson-list a[href$="/japanese/9/"] .lesson-status')).toContainText('완료');
});
test('review center separates due, overdue and completed states',async({page})=>{
 await page.goto('language-study/?view=review');
 await expect(page.getByRole('heading',{name:'복습 센터'})).toBeVisible();
 await expect(page.locator('.review-filters button')).toHaveCount(4);
 await page.getByRole('button',{name:/밀린 복습/}).click();
 await expect(page).toHaveURL(/review=overdue/);
 await page.getByRole('button',{name:/전체 배정/}).click();
 await expect(page.locator('.review-grid .panel').first()).toBeVisible();
 await page.locator('.review-grid .panel').first().getByRole('button',{name:'복습 완료'}).click();
 await page.getByRole('button',{name:/완료 [0-9]+/}).click();
 await expect(page.locator('.review-grid .panel').first()).toBeVisible();
});
test('cloud defaults need only login and leave local records untouched',async({page})=>{
 await page.goto('');
 await page.getByRole('button',{name:'기록 동기화'}).click();
 const dialog=page.getByRole('dialog');
 await expect(dialog).toContainText('학습자 번호 1');
 await expect(dialog).toContainText('서버가 기본 설정되어 있습니다');
 await expect(dialog.locator('input')).toHaveCount(0);
 await expect(dialog.getByRole('button',{name:'GitHub 로그인',exact:true})).toBeVisible();
 await dialog.getByRole('button',{name:'지금 동기화'}).click();
 await expect(dialog.locator('.sync-status')).toContainText('로그인 필요');
 expect(await page.evaluate(()=>localStorage.getItem('atlas-reading-v1'))).toBeNull();
 await expect(dialog.getByRole('button',{name:'로컬 백업'})).toBeVisible();
});
