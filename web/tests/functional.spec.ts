import {selectLearningGroup,expectLearningRoutes,revealLearningRoute,learningRows,learningPageSize} from './learning-test-helpers';
import {compareKnowledgeDocuments} from '../src/lib/knowledge-order';
import legacyVisualRollout from './fixtures/legacy-visual-rollout.json' with {type:'json'};
import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';
import type {Block,Document,Catalog} from '../src/lib/types';
const legacyTabLabels={document:'개념과 원리',practice:'실습·복습',sources:'출처·검증',history:'원문·변경기록'};
function legacyBlocks(blocks:Block[]):Block[]{return blocks.flatMap(b=>[b,...('blocks'in b?legacyBlocks(b.blocks):b.type==='list'?b.items.flatMap(legacyBlocks):[])]);}
async function revealLegacy(page:import('@playwright/test').Page,slug:string,kind:'section'|'code',needle:string){
 if(!legacyVisualRollout.chapters.some(chapter=>chapter.slug===slug))return;
 const authored=JSON.parse(readFileSync('../knowledge/'+slug+'/content.json','utf8')) as Document;
 const section=authored.sections.find(section=>kind==='section'?section.id===needle:legacyBlocks(section.blocks).some(b=>b.type==='code'&&b.language!=='mermaid'&&b.text.includes(needle)));
 expect(section,'approved content contract '+slug+' '+needle).toBeTruthy();
 await page.getByRole('tab',{name:legacyTabLabels[section!.tab as keyof typeof legacyTabLabels],exact:true}).click();
 await expect(page.locator('#'+section!.id)).toBeVisible();
 if(kind==='code'){
  const code=page.locator('#'+section!.id+' .code-block').filter({hasText:needle}).first();
  await expect(code).toHaveCount(1);
  for(const disclosure of await code.locator('xpath=ancestor::details').all()){
   if(await disclosure.getAttribute('open')===null)await disclosure.locator(':scope > summary').click();
   await expect(disclosure).toHaveAttribute('open','');
  }
  await expect(code).toBeVisible();
 }
}
const catalog=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
const routes=JSON.parse(readFileSync('public/data/atlas/routes.json','utf8')) as string[];
const knowledgeDocs=catalog.documents.filter(doc=>doc.kind==='knowledge');
const windowsCategories=['WinAPI','게임 수학','DirectX 9','DirectX 11','DirectX 12'];
const windowsCount=knowledgeDocs.filter(doc=>windowsCategories.includes(doc.category)).length;
const cCount=knowledgeDocs.filter(doc=>doc.category==='C 언어').length;
const courseDocuments=(category:string)=>knowledgeDocs.filter(doc=>doc.category===category).sort(compareKnowledgeDocuments);
const courseRoutes=(category:string)=>courseDocuments(category).map(doc=>doc.route);
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
 const doc=catalog.documents.find(d=>d.number==='1.4.1'&&d.kind==='thinking')!;await page.addInitScript(()=>{if(localStorage.getItem('thinking-ctci-1.4-draft')===null)localStorage.setItem('thinking-ctci-1.4-draft','// original memo');});await page.goto('/um.github.io'+doc.route);await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue('// original memo');await page.getByRole('tab',{name:'풀이·검증',exact:true}).click();await expect(page.locator('details[open]')).toHaveCount(0);await page.getByLabel('C++11 풀이 작성').fill('// edited memo');await page.getByRole('button',{name:'풀이 저장',exact:true}).click();expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1.4-draft'))).toBe('// edited memo');await page.reload();await expect(page.getByLabel('C++11 풀이 작성')).toHaveValue('// edited memo');
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


test('homepage opens the grouped knowledge curriculum with every authored route searchable',async({page,isMobile})=>{
 await page.goto('');
 const entry=page.locator('.dashboard .quick-card').filter({hasText:'프로그래밍 학습'});
 await expect(entry).toBeVisible();await entry.click();
 await expect(page).toHaveURL(/\/knowledge\/$/);
 await expect(page.locator('main h1')).toContainText('프로그래밍');
 const categories=[...new Set(knowledgeDocs.map(doc=>doc.category))];
 const actualGroups=await page.getByTestId('learning-group-nav').locator('button[data-group]').evaluateAll(nodes=>nodes.map(node=>node.getAttribute('data-group')));
 expect(actualGroups.slice().sort()).toEqual(categories.slice().sort());
 const cppOrder=['C 언어','C++ 기초·심화','Modern C++','Effective C++','Effective Modern C++'];
 expect(actualGroups.filter(category=>cppOrder.includes(category!))).toEqual(cppOrder);
 expect(actualGroups.filter(category=>windowsCategories.includes(category!))).toEqual(windowsCategories);
 const plan=page.locator('details.learning-plan');await expect(plan).not.toHaveAttribute('open');await plan.locator('summary').click();
 await expect(plan.locator('.learning-plan-grid section')).toHaveCount(7);await expect(plan.locator('.learning-plan-grid li')).toHaveCount(17);
 await expect(plan.locator('#knowledge-engine')).toContainText('신규 집필 예정');await expect(plan.locator('#knowledge-python')).toContainText('신규 집필 예정');
 await expect(plan.locator('a')).toHaveCount(0);await plan.locator('summary').click();
 const seen:string[]=[];
 for(const category of categories){
  await selectLearningGroup(page,category);
  seen.push(...await expectLearningRoutes(page,courseRoutes(category)));
 }
 expect(seen.slice().sort()).toEqual(knowledgeDocs.map(doc=>'/um.github.io'+doc.route).sort());
 expect(new Set(seen).size).toBe(knowledgeDocs.length);
 // Plans do not invent published documents; the existing Python prototype is still reachable.
 expect(knowledgeDocs.filter(doc=>/^\/knowledge\/engine-|^\/knowledge\/python-(?:core|basics)-/.test(doc.route))).toEqual([]);
 await page.getByRole('searchbox',{name:'문서 검색',exact:true}).fill('BeautifulSoup');
 await expect(learningRows(page)).toHaveCount(1);await expect(learningRows(page).first()).toContainText('BeautifulSoup');
 await page.getByRole('button',{name:'필터 초기화',exact:true}).click();
 await selectLearningGroup(page,'DirectX 11');
 if(isMobile){
  await page.getByRole('button',{name:'사이트 메뉴 열기'}).click();
  await expect(page.getByRole('dialog').getByRole('link',{name:'프로그래밍 학습',exact:true})).toBeVisible();
  await page.keyboard.press('Escape');
 }
 const frustum=await revealLearningRoute(page,'/knowledge/dx11-frustum/',courseRoutes('DirectX 11'));
 await frustum.click();await expect(page).toHaveURL(/\/knowledge\/dx11-frustum\/$/);
 await expect(page.locator('main h1')).toContainText('Frustum Culling');
});

test('WinAPI and game math migrated lessons are usable through the sitemap',async({page})=>{
 await page.goto('knowledge/');
 const seen:string[]=[];
 for(const category of windowsCategories){await selectLearningGroup(page,category);seen.push(...await expectLearningRoutes(page,courseRoutes(category)));}
 expect(seen).toHaveLength(windowsCount);
 expect(seen.slice().sort()).toEqual(knowledgeDocs.filter(doc=>windowsCategories.includes(doc.category)).map(doc=>'/um.github.io'+doc.route).sort());
 await selectLearningGroup(page,'WinAPI');
 const section=page.getByTestId('learning-current-group');
 await expect(section.getByText('WinAPI 01 · 창·핸들·메시지·호출 규약')).toBeVisible();
 await section.getByRole('link',{name:/WinAPI 01/}).click();
 await expect(page).toHaveURL(/\/knowledge\/winapi-01\/$/);
 await expect(page.locator('main h1')).toContainText('WinAPI 01');
 if(legacyVisualRollout.chapters.some(chapter=>chapter.slug==='winapi-01')){
  await expect(page.getByRole('tab')).toHaveText(['개념과 원리','실습·복습','출처·검증','원문·변경기록']);
  // Exact original table/answer payload preservation is checked against the frozen rollout fixture.
  // Every new visual/review is exercised in legacy-visual-textbooks.spec.ts.
 }else{
  await expect(page.locator('main .prose table')).toHaveCount(1);
  await expect(page.locator('main .prose details.disclosure')).toHaveCount(2);
 }
 await page.goto('knowledge/dx9-pipeline-03/');
 await expect(page.locator('main h1')).toContainText('DX9 파이프라인 03');
 if(legacyVisualRollout.chapters.some(chapter=>chapter.slug==='dx9-pipeline-03')){
  await expect(page.getByRole('tab')).toHaveText(Object.values(legacyTabLabels));
 }else{
 await expect(page.locator('main .prose table')).toHaveCount(2);
 await expect(page.locator('main .prose details.disclosure')).toHaveCount(2);
 }
 await page.goto('knowledge/game-math-03/');
 await expect(page.locator('main h1')).toContainText('게임 수학 03');
 await expect(page.locator('main .prose code').first()).toBeVisible();
 if(legacyVisualRollout.chapters.some(chapter=>chapter.slug==='game-math-03')){
  await expect(page.getByRole('tab')).toHaveText(Object.values(legacyTabLabels));
 }else await expect(page.locator('main .prose details.disclosure')).toHaveCount(2);
 await page.goto('knowledge/winapi-04a/');
 if(legacyVisualRollout.chapters.some(chapter=>chapter.slug==='winapi-04a'))await page.getByRole('tab',{name:'실습·복습',exact:true}).click();
 await expect(page.locator('#github-source .code-block').filter({hasText:'int main()'}).first()).toBeVisible();
 await expect(page.locator('main .code-block').filter({hasText:'add_test('})).toBeVisible();
 await page.goto('knowledge/winapi-math-lab/');
 if(legacyVisualRollout.chapters.some(chapter=>chapter.slug==='winapi-math-lab'))await page.getByRole('tab',{name:'실습·복습',exact:true}).click();
 await expect(page.locator('main h1')).toContainText('공통 실습 전체 소스');
 await expect(page.locator('main .code-block').filter({hasText:'winapi_demo.cpp'}).first()).toBeVisible();
 await page.goto('knowledge/dx9-pipeline-lab/');
 await expect(page.locator('main h1')).toContainText('공통 실습 전체 소스');
 await revealLegacy(page,'dx9-pipeline-lab','code','pipeline_trace');
 await expect(page.locator('main .code-block').filter({hasText:'pipeline_trace'}).first()).toBeVisible();
});

test('DX9 implementation chapters and corrected full C++ source are linked inside the atlas',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'DirectX 9');
 const window=page.getByTestId('learning-current-group');
 await expect(window.getByRole('link',{name:/DX9 03 · VB\/IB/})).toBeVisible();
 await window.getByRole('link',{name:/DX9 03 · VB\/IB/}).click();
 await expect(page).toHaveURL(/\/knowledge\/dx9-impl-03\/$/);
 await expect(page.locator('main h1')).toContainText('DX9 03');
 await revealLegacy(page,'dx9-impl-03','section','source-files');
 await expect(page.getByRole('link',{name:/DX9 03 · 전체 소스/})).toBeVisible().catch(async()=>{
  await expect(page.getByRole('link',{name:/전체 소스·CMake·원본 검증 페이지/}).first()).toBeVisible();
 });
 await page.goto('knowledge/dx9-impl-source-03a/');
 await expect(page.locator('main h1')).toContainText('03A');
 await revealLegacy(page,'dx9-impl-source-03a','code','#include');
 await expect(page.locator('main .code-block').filter({hasText:'#include'}).first()).toBeVisible();
 await revealLegacy(page,'dx9-impl-source-03a','code','project(dx9_resource_model');
 await expect(page.locator('main .code-block').filter({hasText:'project(dx9_resource_model'}).first()).toBeVisible();
 await page.goto('knowledge/dx9-impl-12/');
 await expect(page.locator('main h1')).toContainText('DX9 12');
 await revealLegacy(page,'dx9-impl-12','code','// DX9 12: one complete source');
 await expect(page.locator('main .code-block').first()).toBeVisible();
});

test('DX11 migrated chapter sitemap and raw diagrams are readable on the website',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'DirectX 11');
 const dx=page.getByTestId('learning-current-group');
 await expect(dx.getByRole('link',{name:/DX11 02 · Resource·View·Usage·Map/})).toBeVisible();
 await dx.getByRole('link',{name:/DX11 02 · Resource·View·Usage·Map/}).click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-02\/$/);
 await expect(page.locator('main h1')).toContainText('DX11 02');
 await expect(page.locator('main .mermaid-diagram').first()).toHaveAttribute('data-diagram-state','ready');
 await expect(page.locator('main .mermaid-canvas img').first()).toBeVisible();
 await page.goto('knowledge/dx11-06/');
 await expect(page.locator('main h1')).toContainText('DX11 06');
 await revealLegacy(page,'dx11-06','section','visual-review');
 await expect(page.locator('main .prose details.disclosure').first()).toBeVisible();
});

test('DX11 01, 07, 08 course chapters and recovered full C++ sources open',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'DirectX 11');
 const win=page.getByTestId('learning-current-group');
 await expect(win.getByRole('link',{name:/DX11 01/}).first()).toBeVisible();
 await win.getByRole('link',{name:/DX11 01/}).first().click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-01\/$/);
 await expect(page.locator('main h1')).toContainText('DX11 01');
 await expect(page.locator('main .mermaid-diagram').first()).toHaveAttribute('data-diagram-state','ready');
 await expect(page.locator('main .mermaid-canvas img').first()).toBeVisible();
 await revealLegacy(page,'dx11-01','section','full-source');
 await expect(page.getByRole('link',{name:/GitHub Pages 전체 소스 교재에서 보기/})).toBeVisible();
 await page.goto('knowledge/dx11-sources-01/');
 await expect(page.locator('main h1')).toContainText('Device 초기화');
 await revealLegacy(page,'dx11-sources-01','code','D3D11CreateDeviceAndSwapChain');
 await expect(page.locator('main .code-block').filter({hasText:'D3D11CreateDeviceAndSwapChain'}).first()).toBeVisible();
 await page.goto('knowledge/dx11-07/');
 await expect(page.locator('main h1')).toContainText('DX11 07');
 await page.goto('knowledge/dx11-08/');
 await expect(page.locator('main h1')).toContainText('DX11 08');
 await page.goto('knowledge/dx11-sources-05-08/');
 await expect(page.locator('main h1')).toContainText('05–08');
 await revealLegacy(page,'dx11-sources-05-08','code','add_subdirectory(08_terrain)');
 await expect(page.locator('main .code-block').filter({hasText:'add_subdirectory(08_terrain)'}).first()).toBeVisible();
});

test('DX11 Mesh/Animation/Collider/NavMesh navigation and full source links',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'DirectX 11');
 const group=page.getByTestId('learning-current-group');
 await revealLearningRoute(page,'/knowledge/dx11-09/',courseRoutes('DirectX 11'));
 const mesh=group.getByRole('link',{name:/DX11 09 · Mesh·Material·Assimp Node/});
 await expect(mesh).toBeVisible();
 await mesh.click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-09\/$/);
 await expect(page.locator('main h1')).toContainText('DX11 09');
 await expect(page.locator('main .mermaid-diagram').first()).toHaveAttribute('data-diagram-state','ready');
 await expect(page.locator('main .mermaid-canvas img').first()).toBeVisible();
 await revealLegacy(page,'dx11-09','section','full-cpu-source');
 await page.getByRole('link',{name:'전체 C++ 및 CMake 파일 열기'}).click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-sources-09\/$/);
 await revealLegacy(page,'dx11-sources-09','code','int main()');
 await expect(page.locator('main .code-block').filter({hasText:'int main()'}).first()).toBeVisible();
 await page.goto('knowledge/dx11-10/');
 await expect(page.locator('main h1')).toContainText('Skeletal Animation');
 await page.goto('knowledge/dx11-11/');
 await expect(page.locator('main h1')).toContainText('Collider');
 await page.goto('knowledge/dx11-12/');
 await expect(page.locator('main h1')).toContainText('Navigation Mesh');
 await revealLegacy(page,'dx11-12','section','full-cpu-source');
 await page.getByRole('link',{name:'전체 C++ 및 CMake 파일 열기'}).click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-sources-11-12\/$/);
 await revealLegacy(page,'dx11-sources-11-12','code','add_subdirectory(12_navmesh)');
 await expect(page.locator('main .code-block').filter({hasText:'add_subdirectory(12_navmesh)'}).first()).toBeVisible();
});

test('DX11 Instancing, Geometry, QuadTree, Font and Deferred lessons stay browsable',async({page})=>{
 await page.goto('knowledge/');
 for(const slug of ['dx11-13','dx11-13b','dx11-14','dx11-16','dx11-17','dx11-18']){
  await selectLearningGroup(page,'DirectX 11');
  const link=await revealLearningRoute(page,`/knowledge/${slug}/`,courseRoutes('DirectX 11'));
  await expect(link).toBeVisible();
 }
 await selectLearningGroup(page,'DirectX 11');
 await (await revealLearningRoute(page,'/knowledge/dx11-13/',courseRoutes('DirectX 11'))).click();
 await expect(page).toHaveURL(/\/knowledge\/dx11-13\/$/);
 await expect(page.locator('main h1')).toContainText('Instancing');
 await revealLegacy(page,'dx11-13','section','github-source');
 await expect(page.getByRole('link',{name:/13 · GitHub 전체 소스 교재/})).toBeVisible();
 await page.goto('knowledge/dx11-sources-13/');
 await expect(page.locator('main h1')).toContainText('Instancing');
 await revealLegacy(page,'dx11-sources-13','code','DynamicInstances');
 await expect(page.locator('main .code-block').filter({hasText:'DynamicInstances'}).first()).toBeVisible();
 await page.goto('knowledge/dx11-14/');
 await expect(page.locator('main h1')).toContainText('Geometry Shader');
 await page.goto('knowledge/dx11-sources-14/');
 await revealLegacy(page,'dx11-sources-14','code','emitPointSprite');
 await expect(page.locator('main .code-block').filter({hasText:'emitPointSprite'}).first()).toBeVisible();
 await page.goto('knowledge/dx11-16/');
 await expect(page.locator('main h1')).toContainText('QuadTree');
 await page.goto('knowledge/dx11-17/');
 await expect(page.locator('main h1')).toContainText('Font');
 await page.goto('knowledge/dx11-18/');
 await expect(page.locator('main h1')).toContainText('Deferred');
 await page.goto('knowledge/dx11-sources-18/');
 await revealLegacy(page,'dx11-sources-18','code','deferred');
 await expect(page.locator('main .code-block').filter({hasText:'deferred'}).first()).toBeVisible();
});

test('DX11 knowledge outline follows numeric chapter sequence despite migration history',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'DirectX 11');
 const expected=courseRoutes('DirectX 11');
 await expectLearningRoutes(page,expected);
 await selectLearningGroup(page,'DirectX 11');
 const names:string[]=[];
 for(let offset=0;offset<expected.length;offset+=learningPageSize){
  await expect(learningRows(page)).toHaveCount(Math.min(learningPageSize,expected.length-offset));
  await expect(learningRows(page).first()).toHaveAttribute('href','/um.github.io'+expected[offset]);
  names.push(...await learningRows(page).allTextContents());
  if(offset+learningPageSize<expected.length)await page.getByRole('button',{name:'다음 목록',exact:true}).click();
 }
 expect(names).toHaveLength(expected.length);
 const indexOf=(title:string)=>names.findIndex(x=>x.includes(title));
 const idx13=indexOf('DX11 13 · Instancing —');
 const idx13Source=indexOf('DX11 13 · Instancing 전체');
 const idx14=indexOf('DX11 14 · Geometry Shader');
 const idx15=indexOf('Frustum Culling');
 const idx16=indexOf('DX11 16 · QuadTree');
 const idx17=indexOf('DX11 17 · Font');
 const idx18=indexOf('DX11 18 · Deferred');
 for(const idx of [idx13,idx13Source,idx14,idx15,idx16,idx17,idx18])expect(idx).toBeGreaterThanOrEqual(0);
 expect(idx13).toBeLessThan(idx13Source);
 expect(idx13Source).toBeLessThan(idx14);
 expect(idx14).toBeLessThan(idx15);
 expect(idx15).toBeLessThan(idx16);
 expect(idx16).toBeLessThan(idx17);
 expect(idx17).toBeLessThan(idx18);
});


test('published C01 opens from the C language curriculum topic',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'C 언어');
 const topic=page.getByTestId('learning-current-group');
 expect(courseRoutes('C 언어')).toHaveLength(cCount);
 await expectLearningRoutes(page,courseRoutes('C 언어'));
 await selectLearningGroup(page,'C 언어');
 const link=topic.locator('.learning-lesson-link[href="/um.github.io/knowledge/c-language-01/"]');
 await expect(link).toHaveCount(1);
 await expect(link).toHaveAttribute('href','/um.github.io/knowledge/c-language-01/');
 await link.click();
 await expect(page).toHaveURL(/\/knowledge\/c-language-01\/$/);
 await expect(page.locator('main h1')).toContainText('C01');
});

test('C fundamentals 04–05 are linked and render learning visuals',async({page})=>{
 await page.goto('knowledge/');
 await selectLearningGroup(page,'C 언어');
 const topic=page.getByTestId('learning-current-group');
 for(const chapter of ['04','05']){
  const route=`/um.github.io/knowledge/c-basics-${chapter}/`;
  const chapterLink=topic.locator(`a[href="${route}"]`);
  await expect(chapterLink).toHaveCount(1);
  await chapterLink.click();
  await expect(page).toHaveURL(new RegExp(`/knowledge/c-basics-${chapter}/$`));
  await expect(page.locator('main h1')).toContainText(`C 기초 ${chapter}`);
  await expect(page.locator('main .mermaid-diagram').first()).toHaveAttribute('data-diagram-state','ready');
  await expect(page.locator('main .mermaid-canvas img').first()).toBeVisible();
  await page.goto('knowledge/');await selectLearningGroup(page,'C 언어');
 }
});
