import {test,expect,type Page} from '@playwright/test';
import {readFileSync} from 'node:fs';
import type {Catalog,DocumentMeta} from '../src/lib/types';
import {compareKnowledgeDocuments} from '../src/lib/knowledge-order';
import {learningHref,learningPageSize,learningRows,selectLearningGroup,expectLearningRoutes} from './learning-test-helpers';

const catalog=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
const knowledge=catalog.documents.filter(doc=>doc.kind==='knowledge');
const game=catalog.documents.filter(doc=>doc.kind==='game-design').sort((a,b)=>Number(a.number)-Number(b.number));
const course=(category:string)=>knowledge.filter(doc=>doc.category===category).sort(compareKnowledgeDocuments);
const topic=(doc:DocumentMeta)=>doc.topic.match(/\[([^\]]+)\]\s*$/)?.[1]||doc.category||'기타 주제';
const routeKey=(route:string)=>route.replace(/^\//,'');
const groupName=(kind:'knowledge'|'game-design')=>kind==='knowledge'?'학습 과정':'강의 주제';
async function noOverflow(page:Page){expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);}
async function filters(page:Page){const panel=page.locator('details.learning-filter-more');if(await panel.getAttribute('open')===null)await panel.locator('summary').click();await expect(panel).toHaveAttribute('open','');}
async function seed(page:Page,reading:unknown,bookmarks:unknown=[]){
 await page.addInitScript(({reading,bookmarks})=>{
  if(localStorage.getItem('learning-test-seeded')!==null)return;
  localStorage.setItem('learning-test-seeded','yes');
  localStorage.setItem('atlas-reading-v1',JSON.stringify(reading));
  localStorage.setItem('atlas-game-design-bookmarks-v1',JSON.stringify(bookmarks));
  localStorage.setItem('unrelated-value','keep-me');
  localStorage.setItem('thinking-ctci-1-1-draft','preserved thinking note');
  localStorage.setItem('atlas-code-game-design-example','preserved design note');
 },{reading,bookmarks});
}

for(const kind of ['knowledge','game-design'] as const){
 const docs=kind==='knowledge'?knowledge:game;
 const initial=kind==='knowledge'?course('C 언어'):game;
 const groupKey=kind==='knowledge'?'course':'topic';
 test(`learning ${kind} starts compactly with authentic totals and no storage writes`,async({page},info)=>{
  const errors:string[]=[];page.on('pageerror',error=>errors.push(error.message));
  await page.goto(kind+'/');
  await expect(page.getByTestId('learning-study-entry')).toHaveAttribute('href',learningHref(initial[0].route));
  await expect(page.getByTestId('learning-progress')).toContainText(`0 / ${docs.length} 완료`);
  await expect(page.getByTestId('library-count')).toHaveText(`${docs.length} / ${docs.length}개`);
  await expect(learningRows(page)).toHaveCount(Math.min(learningPageSize,initial.length));
  expect(await learningRows(page).evaluateAll(nodes=>nodes.map(node=>node.getAttribute('href')))).toEqual(initial.slice(0,learningPageSize).map(doc=>learningHref(doc.route)));
  await expect(page.locator('main .challenge-grid,main .knowledge-recent-list')).toHaveCount(0);
  expect(await page.getByTestId('learning-study-entry').evaluate(node=>node.getBoundingClientRect().top)).toBeLessThan(650);
  expect(await page.evaluate(()=>localStorage.getItem('atlas-reading-v1'))).toBeNull();
  expect(await page.evaluate(()=>localStorage.getItem('atlas-game-design-bookmarks-v1'))).toBeNull();
  await noOverflow(page);expect(errors).toEqual([]);
  await page.evaluate(()=>document.fonts.ready);await page.screenshot({path:info.outputPath(kind+'-index.png'),fullPage:false});
 });

 test(`learning ${kind} group and page state survive reload Back and Forward`,async({page},info)=>{
  const firstGroup=kind==='knowledge'?'DirectX 11':'all';
  const secondGroup=kind==='knowledge'?'C++ 기초·심화':topic(game[0]);
  const firstDocs=kind==='knowledge'?course(firstGroup):game;
  await page.goto(kind+'/');await selectLearningGroup(page,firstGroup,kind);
  await page.getByRole('button',{name:'다음 목록',exact:true}).click();
  await expect(page).toHaveURL(/page=2(?:&|$)/);const secondPage=page.url();
  await expect(learningRows(page).first()).toHaveAttribute('href',learningHref(firstDocs[learningPageSize].route));
  await selectLearningGroup(page,secondGroup,kind);const secondUrl=page.url();
  expect(new URL(secondUrl).searchParams.has('page')).toBe(false);
  await expect(page.getByLabel(groupName(kind),{exact:true})).toHaveValue(secondGroup);
  await page.reload();await expect(page.getByLabel(groupName(kind),{exact:true})).toHaveValue(secondGroup);
  await page.goBack();await expect(page).toHaveURL(secondPage);await expect(learningRows(page).first()).toHaveAttribute('href',learningHref(firstDocs[learningPageSize].route));
  await page.goForward();await expect(page).toHaveURL(secondUrl);await expect(page.getByTestId('learning-current-group')).toHaveAttribute('data-group',secondGroup);
  await selectLearningGroup(page,firstGroup,kind);
  await expect(page.getByRole('button',{name:'이전 목록',exact:true})).toBeDisabled();
  await noOverflow(page);await page.evaluate(()=>document.fonts.ready);await page.screenshot({path:info.outputPath(kind+'-selected-group.png'),fullPage:false});
 });

 test(`learning ${kind} query category tags and sorting restore without stale-group traps`,async({page})=>{
  const target=kind==='knowledge'?knowledge.find(doc=>doc.title.includes('BeautifulSoup'))!:game[0];
  const search=kind==='knowledge'?'BeautifulSoup':target.title;
  const url=new URLSearchParams({[groupKey]:'missing-group',page:'999',q:search});
  await page.goto(kind+'/?'+url);
  await expect(page.getByRole('searchbox',{name:'문서 검색',exact:true})).toHaveValue(search);
  await expect(learningRows(page)).toHaveCount(1);await expect(learningRows(page)).toHaveAttribute('href',learningHref(target.route));
  await filters(page);
  await page.getByLabel('문제 분류',{exact:true}).selectOption(target.category);
  await page.getByRole('button',{name:'태그 '+target.tags[0],exact:true}).click();
  await page.getByLabel('문제 정렬',{exact:true}).selectOption('newest');
  // Rapid changes must compose before reload; a dropped tag is a real lost update.
  await expect.poll(()=>new URL(page.url()).searchParams.getAll('tag')).toEqual([target.tags[0]]);
  await expect.poll(()=>new URL(page.url()).searchParams.get('sort')).toBe('newest');
  const filtered=page.url();await page.reload();await filters(page);
  await expect(page.getByLabel('문제 분류',{exact:true})).toHaveValue(target.category);
  await expect(page.getByRole('button',{name:'태그 '+target.tags[0],exact:true})).toHaveAttribute('aria-pressed','true');
  await expect(page.getByLabel('문제 정렬',{exact:true})).toHaveValue('newest');
  await expect(learningRows(page)).toHaveCount(1);await expect(learningRows(page)).toHaveAttribute('href',learningHref(target.route));
  await page.getByRole('button',{name:'필터 초기화',exact:true}).click();
  await expect(page).toHaveURL(new RegExp('/'+kind+'/$'));
  await page.goBack();await expect(page).toHaveURL(filtered);await expect(learningRows(page)).toHaveCount(1);
  await page.getByRole('searchbox',{name:'문서 검색',exact:true}).fill('존재하지않는학습자료검색');
  await expect(page.getByText('조건에 맞는 자료가 없습니다.',{exact:true})).toBeVisible();await expect(learningRows(page)).toHaveCount(0);
  await page.getByRole('button',{name:'전체 자료 보기',exact:true}).click();
  await expect(page.getByTestId('library-count')).toHaveText(`${docs.length} / ${docs.length}개`);await noOverflow(page);
 });

 test(`learning ${kind} rapid repeated filter events compose without losing URL state`,async({page})=>{
  const target=kind==='knowledge'?knowledge.find(doc=>doc.title.includes('BeautifulSoup'))!:game[0];
  expect(target.tags.length).toBeGreaterThanOrEqual(2);
  await page.goto(kind+'/?'+new URLSearchParams({q:target.title}));
  await expect(learningRows(page)).toHaveCount(1);await filters(page);
  // One event turn makes the formerly intermittent render-time URL closure deterministic.
  // Use the real React controls and browser change/click events, without artificial waits.
  await page.evaluate(({category,firstTag,secondTag})=>{
   const panel=document.querySelector('details.learning-filter-more')!;
   const categorySelect=panel.querySelector<HTMLSelectElement>('select[aria-label="문제 분류"]')!;
   const sortSelect=panel.querySelector<HTMLSelectElement>('select[aria-label="문제 정렬"]')!;
   const tags=Array.from(panel.querySelectorAll<HTMLButtonElement>('button[aria-label]'));
   const first=tags.find(button=>button.getAttribute('aria-label')==='태그 '+firstTag)!;
   const second=tags.find(button=>button.getAttribute('aria-label')==='태그 '+secondTag)!;
   const select=(node:HTMLSelectElement,value:string)=>{node.value=value;node.dispatchEvent(new Event('change',{bubbles:true}));};
   select(categorySelect,category);first.click();select(sortSelect,'newest');second.click();select(sortSelect,'oldest');first.click();select(sortSelect,'newest');
  },{category:target.category,firstTag:target.tags[0],secondTag:target.tags[1]});
  await expect.poll(()=>new URL(page.url()).searchParams.getAll('tag')).toEqual([target.tags[1]]);
  const final=new URL(page.url());expect(final.searchParams.get('category')).toBe(target.category);expect(final.searchParams.get('sort')).toBe('newest');expect(final.searchParams.get('q')).toBe(target.title);
  await expect(page.getByRole('button',{name:'태그 '+target.tags[0],exact:true})).toHaveAttribute('aria-pressed','false');
  await expect(page.getByRole('button',{name:'태그 '+target.tags[1],exact:true})).toHaveAttribute('aria-pressed','true');
  await expect(learningRows(page)).toHaveAttribute('href',learningHref(target.route));
  await page.reload();await filters(page);await expect(page.getByRole('button',{name:'태그 '+target.tags[1],exact:true})).toHaveAttribute('aria-pressed','true');
  await page.goBack();await expect(page.getByLabel('문제 정렬',{exact:true})).toHaveValue('oldest');
  await expect(page.getByRole('button',{name:'태그 '+target.tags[1],exact:true})).toHaveAttribute('aria-pressed','true');
  await page.goForward();await expect(page).toHaveURL(final.href);await expect(learningRows(page)).toHaveAttribute('href',learningHref(target.route));
 });

 test(`learning ${kind} completion persists and reverses without touching other records`,async({page})=>{
  const first=initial[0],second=initial[1];
  await seed(page,{'unknown/archived/':true});await page.goto(first.route.slice(1));
  const complete=page.getByRole('checkbox',{name:'이 문서 학습 완료',exact:true});
  await complete.check();await page.goto(kind+'/');
  await expect(page.getByTestId('learning-study-entry')).toHaveAttribute('href',learningHref(second.route));
  await expect(page.getByTestId('learning-progress')).toContainText(`1 / ${docs.length} 완료`);
  await page.getByRole('group',{name:'학습 상태 필터'}).getByRole('button',{name:/^미완료 /}).click();
  await expect(page).toHaveURL(/status=unread/);await expect(learningRows(page).locator(`xpath=self::a[@href="${learningHref(first.route)}"]`)).toHaveCount(0);
  await page.goto(first.route.slice(1));await expect(complete).toBeChecked();await complete.uncheck();
  await page.goto(kind+'/');await expect(page.getByTestId('learning-study-entry')).toHaveAttribute('href',learningHref(first.route));
  const values=await page.evaluate(()=>Object.fromEntries(Object.entries(localStorage)));
  expect(JSON.parse(values['atlas-reading-v1'])).toEqual({'unknown/archived/':true,[routeKey(first.route)]:false});
  expect(values['unrelated-value']).toBe('keep-me');expect(values['thinking-ctci-1-1-draft']).toBe('preserved thinking note');expect(values['atlas-code-game-design-example']).toBe('preserved design note');
 });

 test(`learning ${kind} complete and malformed reading states keep bounded progress`,async({page})=>{
  const reading=Object.fromEntries(docs.map(doc=>[routeKey(doc.route),true]));
  await seed(page,reading);await page.goto(kind+'/');
  await expect(page.getByTestId('learning-progress')).toContainText(`${docs.length} / ${docs.length} 완료`);
  const progress=page.getByTestId('learning-progress').locator('progress');await expect(progress).toHaveAttribute('max',String(docs.length));await expect(progress).toHaveAttribute('value',String(docs.length));
  await expect(page.getByTestId('learning-study-entry')).toHaveAttribute('href',learningHref(initial[0].route));
  const original=await page.evaluate(()=>Object.fromEntries(Object.entries(localStorage)));
  await page.getByRole('group',{name:'학습 상태 필터'}).getByRole('button',{name:/^미완료 /}).click();
  await expect(page.getByText('조건에 맞는 자료가 없습니다.',{exact:true})).toBeVisible();
  await page.reload();expect(await page.evaluate(()=>Object.fromEntries(Object.entries(localStorage)))).toEqual(original);
  await page.evaluate(({route})=>localStorage.setItem('atlas-reading-v1',JSON.stringify({[route]:'true',unknown:1})),{route:routeKey(initial[0].route)});
  await page.goto(kind+'/');await expect(progress).toHaveAttribute('value','0');await expect(page.getByTestId('learning-study-entry')).toHaveAttribute('href',learningHref(initial[0].route));
 });

 test(`learning ${kind} lesson sidebar stays in authored group and its index crumb restores page`,async({page,isMobile},info)=>{
  const selected=kind==='knowledge'?course('DirectX 11'):game.filter(doc=>topic(doc)===topic(game.at(-1)!));
  const last=selected.at(-1)!;
  await page.goto(last.route.slice(1));await expect(page.locator('main h1')).toHaveText(last.title);
  const crumb=page.getByRole('navigation',{name:'학습 위치',exact:true});await expect(crumb).toBeVisible();
  const chronological=kind==='game-design'?game:selected,position=chronological.findIndex(doc=>doc.route===last.route);
  const adjacent=[chronological[position-1],chronological[position+1]].filter((doc):doc is DocumentMeta=>!!doc);
  const nextPrevious=page.getByRole('navigation',{name:'이전 다음 문서',exact:true});
  expect(await nextPrevious.getByRole('link').evaluateAll(nodes=>nodes.map(node=>node.getAttribute('href')))).toEqual(adjacent.map(doc=>learningHref(doc.route)));
  await expect(crumb.getByRole('link')).toHaveAttribute('href',new RegExp(groupKey+'='));
  if(isMobile)await page.getByRole('button',{name:'사이트 메뉴 열기'}).click();
  const scope=isMobile?page.getByRole('dialog',{name:'사이트 메뉴',exact:true}):page.locator('.sidebar');
  const nav=scope.getByTestId('learning-sidebar-list');await expect(nav).toBeVisible();
  expect(await nav.locator('a').evaluateAll(nodes=>nodes.map(node=>node.getAttribute('href')))).toEqual(selected.map(doc=>learningHref(doc.route)));
  await expect(nav.locator('a[aria-current=page]')).toHaveAttribute('href',learningHref(last.route));
  if(isMobile)await nav.scrollIntoViewIfNeeded();await expect(nav.locator('a[aria-current=page]')).toBeInViewport();
  await page.evaluate(()=>document.fonts.ready);await page.screenshot({path:info.outputPath(kind+'-lesson-navigation.png'),fullPage:false});
  if(isMobile){await page.keyboard.press('Escape');await expect(page.getByRole('dialog')).toHaveCount(0);await expect(page).toHaveURL(new RegExp(last.route+'$'));}
  await crumb.getByRole('link').click();
  await expect(learningRows(page).locator(`xpath=self::a[@href="${learningHref(last.route)}"]`)).toHaveCount(1);
  expect(new URL(page.url()).searchParams.get('page')).toBe(String(Math.floor((selected.length-1)/learningPageSize)+1));
  await page.goBack();await expect(page).toHaveURL(new RegExp(last.route+'$'));await expect(page.locator('main h1')).toHaveText(last.title);
  if(isMobile){
   await page.getByRole('button',{name:'사이트 메뉴 열기'}).click();
   await scope.getByLabel('사이드바 '+groupName(kind),{exact:true}).selectOption(kind==='knowledge'?'C 언어':topic(game[0]));
   await expect(page.getByRole('dialog')).toHaveCount(0);await expect(page.getByTestId('learning-current-group')).toBeVisible();
  }
  await noOverflow(page);
 });

 test(`learning ${kind} pagination returns reading position and keyboard focus to the new list`,async({page})=>{
  const id=kind==='knowledge'?'DirectX 11':'all',ordered=kind==='knowledge'?course(id):game;
  await page.goto(kind+'/');await selectLearningGroup(page,id,kind);
  const next=page.getByRole('button',{name:'다음 목록',exact:true}),previous=page.getByRole('button',{name:'이전 목록',exact:true});
  const heading=page.getByTestId('learning-list-heading');
  await next.scrollIntoViewIfNeeded();await expect(learningRows(page).first()).not.toBeInViewport();
  await next.click();await expect(learningRows(page).first()).toHaveAttribute('href',learningHref(ordered[learningPageSize].route));
  await expect(heading).toBeFocused();await expect(learningRows(page).first()).toBeInViewport({ratio:1});
  await page.keyboard.press('Tab');await expect(learningRows(page).first()).toBeFocused();
  await previous.scrollIntoViewIfNeeded();await expect(learningRows(page).first()).not.toBeInViewport();
  await previous.click();await expect(learningRows(page).first()).toHaveAttribute('href',learningHref(ordered[0].route));
  await expect(heading).toBeFocused();await expect(learningRows(page).first()).toBeInViewport({ratio:1});
 });

 test(`learning ${kind} date sorting changes real list order without losing selected group`,async({page})=>{
  const id=kind==='knowledge'?'DirectX 11':'all',ordered=kind==='knowledge'?course(id):game;
  await page.goto(kind+'/');await selectLearningGroup(page,id,kind);
  await page.getByRole('button',{name:'다음 목록',exact:true}).click();await expect(page).toHaveURL(/page=2(?:&|$)/);
  await filters(page);await page.getByLabel('문제 정렬',{exact:true}).selectOption('newest');
  await expect(page.getByTestId('learning-current-group')).toHaveAttribute('data-group',id);
  await expect.poll(()=>new URL(page.url()).searchParams.get('page')).toBeNull();
  const newest=ordered.slice().sort((a,b)=>b.date.localeCompare(a.date));
  const oldest=ordered.slice().sort((a,b)=>a.date.localeCompare(b.date));
  expect(newest.map(doc=>doc.route)).not.toEqual(oldest.map(doc=>doc.route));
  await expectLearningRoutes(page,newest.map(doc=>doc.route));
  await page.getByLabel('문제 정렬',{exact:true}).selectOption('oldest');
  await expect(page.getByLabel(groupName(kind),{exact:true})).toHaveValue(id);
  await expect.poll(()=>new URL(page.url()).searchParams.get('page')).toBeNull();
  await expectLearningRoutes(page,oldest.map(doc=>doc.route));
  await page.getByLabel('문제 정렬',{exact:true}).selectOption('number');
  await expect(learningRows(page).first()).toHaveAttribute('href',learningHref(ordered[0].route));
  await expect(page.getByLabel(groupName(kind),{exact:true})).toHaveValue(id);await noOverflow(page);
 });

 for(const colorScheme of ['light','dark'] as const)test(`learning ${kind} honors shared dark theme when OS is ${colorScheme}`,async({page})=>{
  await page.emulateMedia({colorScheme});
  for(const path of [kind+'/',initial[0].route.slice(1)]){
   await page.goto(path);await expect(page.locator('main h1')).toBeVisible();
   await expect(page.locator('html')).toHaveAttribute('data-theme','dark');await expect(page.locator('.app-shell')).toHaveCSS('background-color','rgb(16, 19, 25)');
   await noOverflow(page);
  }
 });
}

test('learning game-design exposes all authored episodes once chronologically and once per source topic',async({page})=>{
 await page.goto('game-design/');await selectLearningGroup(page,'all','game-design');
 await expectLearningRoutes(page,game.map(doc=>doc.route));
 const topics=[...new Set(game.map(topic))],seen:string[]=[];
 for(const id of topics){await selectLearningGroup(page,id,'game-design');seen.push(...await expectLearningRoutes(page,game.filter(doc=>topic(doc)===id).map(doc=>doc.route)));}
 expect(seen.slice().sort()).toEqual(game.map(doc=>learningHref(doc.route)).sort());expect(new Set(seen).size).toBe(game.length);
});

test('learning game-design bookmark filter persists in URL and never rewrites legacy saved routes',async({page})=>{
 const selected=[game[0],game[20],game.at(-1)!],saved=[...selected.map(doc=>doc.route),'/unknown/legacy-bookmark/'];
 await seed(page,{},saved);await page.goto('game-design/');
 await page.getByRole('group',{name:'학습 상태 필터'}).getByRole('button',{name:/^북마크 /}).click();
 await expect(page).toHaveURL(/status=saved/);await expectLearningRoutes(page,selected.map(doc=>doc.route));
 await page.reload();await expectLearningRoutes(page,selected.map(doc=>doc.route));
 await page.goto(game[0].route.slice(1));await page.getByRole('button',{name:'북마크됨',exact:true}).click();
 await expect(page.getByRole('button',{name:'북마크 저장',exact:true})).toBeVisible();
 await page.goto('game-design/?status=saved');await expectLearningRoutes(page,selected.slice(1).map(doc=>doc.route));
 expect(await page.evaluate(()=>JSON.parse(localStorage.getItem('atlas-game-design-bookmarks-v1')!))).toEqual(saved.filter(route=>route!==game[0].route));
});

for(const kind of ['knowledge','game-design'] as const)for(const viewport of [{width:1165,height:747},{width:900,height:650}])test(`learning ${kind} short desktop ${viewport.width}x${viewport.height} keeps active navigation unobscured after resize`,async({page},info)=>{
 const selected=kind==='knowledge'?course('DirectX 11'):game.filter(doc=>topic(doc)===topic(game[0]));
 const last=selected.at(-1)!;await page.setViewportSize(viewport);
 await page.goto(kind+'/');await expect(page.locator('main h1')).toBeVisible();
 const sidebar=page.locator('aside.sidebar');await expect(sidebar.locator('.side-foot')).toBeHidden();
 await expect(sidebar.locator('.learning-sidebar-caption')).toBeInViewport({ratio:1});
 expect(await sidebar.evaluate(node=>node.scrollHeight-node.clientHeight)).toBeLessThanOrEqual(1);
 await page.goto(last.route.slice(1));await expect(page.locator('main h1')).toHaveText(last.title);await page.evaluate(()=>document.fonts.ready);
 const nav=sidebar.getByTestId('learning-sidebar-list'),active=nav.locator('a[aria-current=page]');
 await expect(sidebar.getByLabel('사이드바 '+groupName(kind),{exact:true})).toBeInViewport({ratio:1});await expect(active).toBeInViewport({ratio:1});
 const bounds=await active.evaluate(node=>{const row=node.getBoundingClientRect(),list=node.parentElement!.getBoundingClientRect();return {top:row.top,bottom:row.bottom,listTop:list.top,listBottom:list.bottom,height:innerHeight};});
 expect(bounds.top).toBeGreaterThanOrEqual(bounds.listTop-1);expect(bounds.bottom).toBeLessThanOrEqual(bounds.listBottom+1);expect(bounds.listBottom).toBeLessThanOrEqual(bounds.height);
 expect(await sidebar.evaluate(node=>node.scrollHeight-node.clientHeight)).toBeLessThanOrEqual(1);
 await page.setViewportSize({width:viewport.width,height:1000});await expect(active).toBeInViewport({ratio:1});
 await page.setViewportSize(viewport);await expect(active).toBeInViewport({ratio:1});await noOverflow(page);
 await page.screenshot({path:info.outputPath(`${kind}-short-${viewport.height}.png`),fullPage:false});
});
