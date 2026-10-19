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


test('game design diagrams are readable inline and zoomed at desktop/mobile sizes',async({page,isMobile})=>{
 await page.goto('game-design/ep-01-stop-for-big-moments/?tab=implementation');
 const figure=page.locator('.game-design-illustration').first();
 await expect(figure).toBeVisible();
 const trigger=figure.locator('button.image-zoom');
 const img=figure.locator('img');
 await expect(img).toBeVisible();
 await expect.poll(()=>img.evaluate((node:HTMLImageElement)=>node.naturalWidth)).toBeGreaterThan(0);
 const imageWidth=await img.evaluate((node:HTMLImageElement)=>node.getBoundingClientRect().width);
 expect(imageWidth).toBeGreaterThan(isMobile?230:400);
 await trigger.click();
 const dialog=page.getByRole('dialog');
 await expect(dialog).toBeVisible();
 await expect(dialog.locator('img')).toBeVisible();
 const dialogImageWidth=await dialog.locator('img').evaluate((node:HTMLImageElement)=>node.getBoundingClientRect().width);
 expect(dialogImageWidth).toBeGreaterThan(isMobile?230:600);
 await expect(dialog.getByRole('link',{name:'원본 이미지 저장'})).toBeVisible();
});


test('homepage opens the Notion-style knowledge sitemap with searchable chapter tree',async({page,isMobile})=>{
 await page.goto('');
 const entry=page.locator('.dashboard .quick-card').filter({hasText:'프로그래밍 학습'});
 await expect(entry).toBeVisible();
 await entry.click();
 await expect(page).toHaveURL(/\/knowledge\/$/);
 await expect(page.getByRole('heading',{name:'프로그래밍 지식 라이브러리'})).toBeVisible();
 await expect(page.locator('.knowledge-map-sidebar nav a')).toHaveCount(7);
 const tree=page.locator('.knowledge-tree');
 await expect(tree.locator('.knowledge-document-link')).toHaveCount(51);
 const cppTopics=page.locator('#knowledge-cpp .knowledge-topic-row strong');
 await expect(cppTopics).toHaveText(['C 언어','C++ 기초·심화','Modern C++','Effective C++','Effective Modern C++']);
 const windowsTopics=page.locator('#knowledge-windows .knowledge-topic-row strong');
 await expect(windowsTopics).toHaveText(['WinAPI','게임 수학','DirectX 9','DirectX 11','DirectX 12']);
 await expect(page.getByText('집필 시 설계').first()).toBeVisible();
 await expect(page.getByText('신규 집필 예정').first()).toBeVisible();
 await expect(page.locator('.knowledge-recent-list a')).toHaveCount(53);
 const dx11=tree.getByRole('button',{name:/Windows·그래픽스 프로그래밍/});
 await expect(dx11).toHaveAttribute('aria-expanded','true');
 await expect(tree.getByRole('link',{name:/Frustum Culling/})).toBeVisible();
 await page.getByRole('button',{name:'모두 접기'}).click();
 await expect(tree.locator('.knowledge-document-link')).toHaveCount(0);
 await dx11.click();
 await expect(tree.locator('.knowledge-document-link')).toHaveCount(51);
 await page.getByLabel('학습 목차 검색').fill('BeautifulSoup');
 await expect(tree.locator('.knowledge-document-link')).toHaveCount(0);
 await expect(page.locator('.knowledge-recent-list a')).toHaveCount(1);
 await expect(page.locator('.knowledge-recent-list a').first()).toContainText('BeautifulSoup');
 await page.getByRole('button',{name:'목차 검색 초기화'}).click();
 await page.getByRole('button',{name:'모두 펼치기'}).click();
 await expect(tree.locator('.knowledge-document-link')).toHaveCount(51);
 if(isMobile){
  await page.getByRole('button',{name:'사이트 메뉴 열기'}).click();
  await expect(page.getByRole('dialog').getByRole('link',{name:'프로그래밍 학습',exact:true})).toBeVisible();
  await page.keyboard.press('Escape');
 }
 await tree.getByRole('link',{name:/Frustum Culling/}).click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-frustum\/$/);
 await expect(page.locator('main h1')).toContainText('Frustum Culling');
});

test('WinAPI and game math migrated lessons are usable through the sitemap',async({page})=>{
 await page.goto('knowledge/');
 const section=page.locator('#knowledge-windows');
 await expect(section.getByRole('button',{name:/Windows·그래픽스 프로그래밍/})).toHaveAttribute('aria-expanded','true');
 const items=section.locator('.knowledge-document-link');
 await expect(items).toHaveCount(51);
 await expect(section.getByText('WinAPI 01 · 창·핸들·메시지·호출 규약')).toBeVisible();
 await section.getByRole('link',{name:/WinAPI 01/}).click();
 await expect(page).toHaveURL(/\/knowledge\/winapi-01\/$/);
 await expect(page.locator('main h1')).toContainText('WinAPI 01');
 await expect(page.locator('main .prose table')).toHaveCount(1);
 await expect(page.locator('main .prose details')).toHaveCount(2);
 await page.goto('knowledge/dx9-pipeline-03/');
 await expect(page.locator('main h1')).toContainText('DX9 파이프라인 03');
 await expect(page.locator('main .prose table')).toHaveCount(2);
 await expect(page.locator('main .prose details')).toHaveCount(2);
 await page.goto('knowledge/game-math-03/');
 await expect(page.locator('main h1')).toContainText('게임 수학 03');
 await expect(page.locator('main .prose code').first()).toBeVisible();
 await expect(page.locator('main .prose details')).toHaveCount(2);
 await page.goto('knowledge/winapi-04a/');
 await expect(page.locator('main .code-block').filter({hasText:'int main()'})).toBeVisible();
 await expect(page.locator('main .code-block').filter({hasText:'add_test('})).toBeVisible();
 await page.goto('knowledge/winapi-math-lab/');
 await expect(page.locator('main h1')).toContainText('공통 실습 전체 소스');
 await expect(page.locator('main .code-block').filter({hasText:'winapi_demo.cpp'}).first()).toBeVisible();
 await page.goto('knowledge/dx9-pipeline-lab/');
 await expect(page.locator('main h1')).toContainText('공통 실습 전체 소스');
 await expect(page.locator('main .code-block').filter({hasText:'pipeline_trace'}).first()).toBeVisible();
});

test('DX9 implementation chapters and corrected full C++ source are linked inside the atlas',async({page})=>{
 await page.goto('knowledge/');
 const window=page.locator('#knowledge-windows');
 await expect(window.getByRole('link',{name:/DX9 03 · VB\/IB/})).toBeVisible();
 await window.getByRole('link',{name:/DX9 03 · VB\/IB/}).click();
 await expect(page).toHaveURL(/\/knowledge\/dx9-impl-03\/$/);
 await expect(page.locator('main h1')).toContainText('DX9 03');
 await expect(page.getByRole('link',{name:/DX9 03 · 전체 소스/})).toBeVisible().catch(async()=>{
  await expect(page.getByRole('link',{name:/전체 소스·CMake·원본 검증 페이지/}).first()).toBeVisible();
 });
 await page.goto('knowledge/dx9-impl-source-03a/');
 await expect(page.locator('main h1')).toContainText('03A');
 await expect(page.locator('main .code-block').filter({hasText:'#include'}).first()).toBeVisible();
 await expect(page.locator('main .code-block').filter({hasText:'project(dx9_resource_model'}).first()).toBeVisible();
 await page.goto('knowledge/dx9-impl-12/');
 await expect(page.locator('main h1')).toContainText('DX9 12');
 await expect(page.locator('main .code-block').first()).toBeVisible();
});

test('DX11 migrated chapter sitemap and raw diagrams are readable on the website',async({page})=>{
 await page.goto('knowledge/');
 const dx=page.locator('#knowledge-windows');
 await expect(dx.getByRole('link',{name:/DX11 02/})).toBeVisible();
 await dx.getByRole('link',{name:/DX11 02/}).click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-02\/$/);
 await expect(page.locator('main h1')).toContainText('DX11 02');
 await expect(page.locator('main .code-block').filter({hasText:'flowchart'}).first()).toBeVisible();
 await page.goto('knowledge/dx11-06/');
 await expect(page.locator('main h1')).toContainText('DX11 06');
 await expect(page.locator('main details').first()).toBeVisible();
});

test('DX11 01, 07, 08 course chapters and recovered full C++ sources open',async({page})=>{
 await page.goto('knowledge/');
 const win=page.locator('#knowledge-windows');
 await expect(win.getByRole('link',{name:/DX11 01/}).first()).toBeVisible();
 await win.getByRole('link',{name:/DX11 01/}).first().click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-01\/$/);
 await expect(page.locator('main h1')).toContainText('DX11 01');
 await expect(page.locator('main .code-block').filter({hasText:'flowchart'}).first()).toBeVisible();
 await expect(page.getByRole('link',{name:/GitHub Pages 전체 소스 교재에서 보기/})).toBeVisible();
 await page.goto('knowledge/dx11-sources-01/');
 await expect(page.locator('main h1')).toContainText('Device 초기화');
 await expect(page.locator('main .code-block').filter({hasText:'D3D11CreateDeviceAndSwapChain'}).first()).toBeVisible();
 await page.goto('knowledge/dx11-07/');
 await expect(page.locator('main h1')).toContainText('DX11 07');
 await page.goto('knowledge/dx11-08/');
 await expect(page.locator('main h1')).toContainText('DX11 08');
 await page.goto('knowledge/dx11-sources-05-08/');
 await expect(page.locator('main h1')).toContainText('05–08');
 await expect(page.locator('main .code-block').filter({hasText:'add_subdirectory(08_terrain)'}).first()).toBeVisible();
});
