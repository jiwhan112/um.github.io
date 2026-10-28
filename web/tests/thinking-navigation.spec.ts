import {test,expect,type Page} from '@playwright/test';
import {readFileSync} from 'node:fs';
import type {Catalog} from '../src/lib/types';

const catalog=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
const docs=catalog.documents.filter(d=>d.kind==='thinking').sort((a,b)=>String(a.number).localeCompare(String(b.number),'ko',{numeric:true}));
const chapters=[...new Set(docs.map(d=>String(d.number).split('.')[0]))];
const routeKey=(route:string)=>route.replace(/^\//,'');
const href=(route:string)=>'/um.github.io'+route;
const inChapter=(chapter:string)=>docs.filter(d=>String(d.number).split('.')[0]===chapter);
async function selectChapter(page:Page,chapter:string){
 const select=page.getByLabel('학습 장',{exact:true});
 if((page.viewportSize()?.width || 1440)<=760){await expect(select).toBeVisible();await select.selectOption(chapter);}
 else await page.getByRole('navigation',{name:'학습 장 목록',exact:true}).getByRole('button',{name:new RegExp('^'+chapter.padStart(2,'0')+' ')}).click();
}
async function noOverflow(page:Page){expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);}
async function seedReading(page:Page,reading:unknown){
 await page.addInitScript(({reading})=>{
  if(localStorage.getItem('atlas-reading-v1')===null){
   localStorage.setItem('atlas-reading-v1',JSON.stringify(reading));
   localStorage.setItem('unrelated-value','keep-me');
   localStorage.setItem('thinking-ctci-1-1-draft','preserved basic note');
   localStorage.setItem('thinking-ctci-1-1-2-draft','preserved advanced note');
  }
 },{reading});
}

test('thinking starts at the first basic page with one chapter of paired topics',async({page},testInfo)=>{
 const errors:string[]=[];page.on('pageerror',error=>errors.push(error.message));
 await page.goto('thinking-lab/');
 await expect(page.getByTestId('thinking-study-entry')).toHaveAttribute('href',href(docs[0].route));
 await expect(page.getByLabel('학습 장',{exact:true})).toHaveValue(chapters[0]);
 await expect(page.getByTestId('library-card')).toHaveCount(inChapter(chapters[0]).length);
 await expect(page.locator('.thinking-topic-row')).toHaveCount(new Set(inChapter(chapters[0]).map(d=>String(d.number).split('.').slice(0,2).join('.'))).size);
 await expect(page.locator('main')).not.toContainText('매일 한 문제');
 await expect(page.locator('main .challenge-grid')).toHaveCount(0);
 for(const row of await page.locator('.thinking-topic-row').all()){
  await expect(row.getByTestId('library-card')).toHaveCount(2);
  await expect(row.getByTestId('library-card').nth(0)).toHaveAttribute('aria-label',/\.1 기본/);
  await expect(row.getByTestId('library-card').nth(1)).toHaveAttribute('aria-label',/\.2 심화/);
 }
 expect(await page.getByTestId('thinking-study-entry').evaluate(a=>a.getBoundingClientRect().top)).toBeLessThan(700);
 await noOverflow(page);
 expect(await page.evaluate(()=>localStorage.getItem('atlas-reading-v1'))).toBeNull();
 expect(errors).toEqual([]);
 await page.evaluate(()=>document.fonts.ready);await page.screenshot({path:testInfo.outputPath('thinking-ui-index.png'),fullPage:false});
});

test('thinking chapter selection restores through reload, Back and Forward',async({page},testInfo)=>{
 await page.goto('thinking-lab/');
 const select=page.getByLabel('학습 장',{exact:true});
 await selectChapter(page,chapters[1]);
 await expect(page).toHaveURL(new RegExp('chapter='+chapters[1]+'(?:&|$)'));
 const secondUrl=page.url();
 await expect(page.getByTestId('library-card')).toHaveCount(inChapter(chapters[1]).length);
 await selectChapter(page,chapters.at(-1)!);
 await expect(select).toHaveValue(chapters.at(-1)!);
 const lastUrl=page.url();
 await page.reload();
 await expect(select).toHaveValue(chapters.at(-1)!);
 await expect(page.getByTestId('library-card')).toHaveCount(inChapter(chapters.at(-1)!).length);
 await page.goBack();await expect(page).toHaveURL(secondUrl);await expect(select).toHaveValue(chapters[1]);
 await page.goForward();await expect(page).toHaveURL(lastUrl);await expect(select).toHaveValue(chapters.at(-1)!);
 await noOverflow(page);
 await page.locator('.thinking-chapter-heading').scrollIntoViewIfNeeded();
 await page.evaluate(()=>document.fonts.ready);await page.screenshot({path:testInfo.outputPath('thinking-ui-chapter17.png'),fullPage:false});
});

test('thinking continue derives only from preserved completed-page flags, not newest publication',async({page})=>{
 const firstChapter=inChapter(chapters[0]);
 const reading:Record<string,unknown>=Object.fromEntries(firstChapter.map(d=>[routeKey(d.route),true]));
 reading['unknown/old-document/']=true;
 const target=docs[firstChapter.length];reading[routeKey(target.route)]=false;
 await seedReading(page,reading);await page.goto('thinking-lab/');
 await expect(page.getByTestId('thinking-study-entry')).toHaveAttribute('href',href(target.route));
 await expect(page.getByLabel('학습 장',{exact:true})).toHaveValue(chapters[1]);
 await expect(page.getByTestId('thinking-progress')).toContainText(`${firstChapter.length} / ${docs.length} 완료`);
 const original=await page.evaluate(()=>Object.fromEntries(Object.entries(localStorage)));
 await selectChapter(page,chapters.at(-1)!);
 await page.getByRole('searchbox',{name:'문제 검색',exact:true}).fill('17.26');
 await expect(page.locator('.thinking-topic-row')).toHaveCount(1);await expect(page.getByTestId('library-card')).toHaveCount(2);
 await page.reload();
 await expect(page.getByTestId('thinking-study-entry')).toHaveAttribute('href',href(target.route));
 expect(await page.evaluate(()=>Object.fromEntries(Object.entries(localStorage)))).toEqual(original);
});

test('thinking completion can be reversed without resetting any other study or memo data',async({page})=>{
 await seedReading(page,{});await page.goto(docs[0].route.slice(1));
 const complete=page.getByRole('checkbox',{name:'이 문서 학습 완료',exact:true});
 await complete.check();await page.goto('thinking-lab/');
 await expect(page.getByTestId('thinking-study-entry')).toHaveAttribute('href',href(docs[1].route));
 await page.goto(docs[0].route.slice(1));await expect(complete).toBeChecked();await complete.uncheck();
 await page.goto('thinking-lab/');await expect(page.getByTestId('thinking-study-entry')).toHaveAttribute('href',href(docs[0].route));
 expect(await page.evaluate(()=>localStorage.getItem('unrelated-value'))).toBe('keep-me');
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1-1-draft'))).toBe('preserved basic note');
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1-1-2-draft'))).toBe('preserved advanced note');
});

test('thinking all-completed and malformed progress retain a safe first-page entry',async({page})=>{
 const reading=Object.fromEntries(docs.map(d=>[routeKey(d.route),true]));
 await seedReading(page,reading);await page.goto('thinking-lab/');
 await expect(page.getByTestId('thinking-study-entry')).toHaveAttribute('href',href(docs[0].route));
 await expect(page.getByTestId('thinking-progress')).toContainText(`${docs.length} / ${docs.length} 완료`);
 expect(await page.evaluate(()=>JSON.parse(localStorage.getItem('atlas-reading-v1')!))).toEqual(reading);
 await page.evaluate(()=>localStorage.setItem('atlas-reading-v1','{"thinking-lab/ep-01-01-is-unique/":"true"}'));
 await page.reload();await expect(page.getByTestId('thinking-study-entry')).toHaveAttribute('href',href(docs[0].route));
 await noOverflow(page);
});

test('thinking lesson navigation stays in the current chapter and crosses chapter boundaries correctly',async({page,isMobile},testInfo)=>{
 const last=inChapter(chapters[0]).at(-1)!,index=docs.findIndex(d=>d.id===last.id);
 await page.goto(last.route.slice(1));await expect(page.getByTestId('thinking-current-chapter')).toContainText('1장 ·');
 await page.evaluate(()=>document.fonts.ready);await page.screenshot({path:testInfo.outputPath('thinking-ui-lesson.png'),fullPage:false});
 const nextPrevious=page.getByRole('navigation',{name:'이전 다음 문서',exact:true});
 await expect(nextPrevious.locator('a').nth(0)).toHaveAttribute('href',href(docs[index-1].route));
 await expect(nextPrevious.locator('a').nth(1)).toHaveAttribute('href',href(docs[index+1].route));
 if(isMobile)await page.getByRole('button',{name:'사이트 메뉴 열기',exact:true}).click();
 const scope=isMobile?page.getByRole('dialog'):page.locator('.sidebar');
 const chapterNav=scope.getByTestId('thinking-chapter-nav');await expect(chapterNav).toBeVisible();
 const navRoutes=await chapterNav.locator('a').evaluateAll(links=>links.map(link=>new URL((link as HTMLAnchorElement).href).pathname.replace(/^\/um\.github\.io/,'')));
 const lessonRoutes=navRoutes.filter(route=>/^\/thinking-lab\/ep-/.test(route));
 expect(lessonRoutes.slice().sort()).toEqual(inChapter(chapters[0]).map(d=>d.route).sort());
 await expect(chapterNav.locator('[aria-current="page"]')).toHaveAttribute('href',href(last.route));
 if(isMobile)await chapterNav.scrollIntoViewIfNeeded();
 await expect(chapterNav.locator('[aria-current="page"]')).toBeInViewport();
 if(isMobile)await page.screenshot({path:testInfo.outputPath('thinking-ui-sidebar.png'),fullPage:false});
 if(isMobile){await page.keyboard.press('Escape');await expect(page.getByRole('dialog')).toHaveCount(0);await expect(page).toHaveURL(new RegExp(last.route+'$'));}
 await nextPrevious.locator('a').nth(1).click();await expect(page).toHaveURL(new RegExp(docs[index+1].route+'$'));
 await expect(page.getByTestId('thinking-current-chapter')).toContainText('2장 ·');
 await page.goBack();await expect(page).toHaveURL(new RegExp(last.route+'$'));await expect(page.getByTestId('thinking-current-chapter')).toContainText('1장 ·');
 await noOverflow(page);
});


test('thinking numeric search is exact at topic boundaries and restores a stale chapter deep link',async({page})=>{
 await page.goto('thinking-lab/?chapter=1&q=17.1');
 await expect(page.getByRole('searchbox',{name:'문제 검색',exact:true})).toHaveValue('17.1');
 await expect(page.getByLabel('학습 장',{exact:true})).toHaveValue('17');
 await expect(page.locator('.thinking-topic-row')).toHaveCount(1);
 const links=page.getByTestId('library-card');await expect(links).toHaveCount(2);
 await expect(links.nth(0)).toHaveAttribute('aria-label',/^17\.1\.1 기본/);
 await expect(links.nth(1)).toHaveAttribute('aria-label',/^17\.1\.2 심화/);
 await page.getByRole('searchbox',{name:'문제 검색',exact:true}).fill('17.10');
 await expect(links).toHaveCount(2);
 await expect(links.nth(0)).toHaveAttribute('aria-label',/^17\.10\.1 기본/);
 await page.reload();await expect(links.nth(0)).toHaveAttribute('aria-label',/^17\.10\.1 기본/);
 await page.getByRole('button',{name:'필터 초기화',exact:true}).click();
 await expect(page.getByLabel('학습 장',{exact:true})).toHaveValue(chapters[0]);
 await expect(page.getByTestId('library-card')).toHaveCount(inChapter(chapters[0]).length);
 await noOverflow(page);
});

test('thinking sorting changes topic order without leaving the selected chapter',async({page})=>{
 const chapter=chapters.at(-1)!;
 await page.goto('thinking-lab/?chapter='+chapter);
 const details=page.locator('details.thinking-filter-more');
 await details.locator('summary').click();
 await page.getByLabel('문제 정렬',{exact:true}).selectOption('newest');
 await expect(page.getByLabel('학습 장',{exact:true})).toHaveValue(chapter);
 await expect(page).toHaveURL(new RegExp('chapter='+chapter+'(?:&|$)'));
 const rows=page.locator('.thinking-topic-row'),chapterDocs=inChapter(chapter);
 const topicNumbers=[...new Set(chapterDocs.map(d=>String(d.number).split('.').slice(0,2).join('.')))];
 await expect(rows).toHaveCount(topicNumbers.length);
 await expect(rows.first().locator('.thinking-topic-number')).toHaveText(topicNumbers.at(-1)!);
 await page.getByLabel('문제 정렬',{exact:true}).selectOption('number');
 await expect(page.getByLabel('학습 장',{exact:true})).toHaveValue(chapter);
 await expect(rows.first().locator('.thinking-topic-number')).toHaveText(topicNumbers[0]);
 await page.reload();await expect(page.getByLabel('학습 장',{exact:true})).toHaveValue(chapter);
});
