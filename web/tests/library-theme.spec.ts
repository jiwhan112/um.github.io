import { test, expect } from '@playwright/test';
import {readFileSync} from 'node:fs';
import type {Catalog} from '../src/lib/types';
const catalog=JSON.parse(readFileSync('public/data/atlas/catalog.json','utf8')) as Catalog;
const docs=catalog.documents.filter(d=>d.kind==='thinking');
// The full route suite covers every new document; filter examples use a stable four-document fixture.
const fixture=JSON.parse(readFileSync('tests/fixtures/catalog.json','utf8'));
test.beforeEach(async({page},info)=>{if(info.title.startsWith('library'))await page.route('**/data/atlas/catalog.json',r=>r.fulfill({contentType:'application/json',body:JSON.stringify(fixture)}));});

test('all route families use the dashboard dark theme even when OS prefers light',async({page})=>{
 await page.emulateMedia({colorScheme:'light'});
 for(const path of ['', 'daily-briefing/', 'language-study/', 'language-study/japanese/9/', 'language-study/english/9/', 'thinking-lab/', 'thinking-lab/ep-01-04-palindrome-permutation/', 'ai-asset-lab/', 'ai-asset-lab/experiments/2d/001-potion-icons/report/', 'opensource-lab/', 'bookmarks/', 'archive/']){
  await page.goto(path);await expect(page.locator('[data-framework=react]')).toBeVisible();await expect(page.locator('main h1').first()).toBeVisible();
  await expect(page.locator('html')).toHaveAttribute('data-theme','dark');await expect(page.locator('.app-shell')).toHaveCSS('background-color','rgb(16, 19, 25)');
  expect(await page.evaluate(()=>getComputedStyle(document.documentElement).colorScheme)).toBe('dark');
 }
 await page.locator('.top-search').click();await expect(page.getByRole('dialog')).toBeVisible();
 expect(await page.getByRole('dialog').evaluate(e=>getComputedStyle(e).getPropertyValue('--text').trim())).toBe('#e9edf5');
});
test('library combines category, tag and query with URL restoration',async({page})=>{
 await page.goto('thinking-lab/');await expect(page.getByTestId('library-card')).toHaveCount(4);
 await page.getByRole('button',{name:'태그 문자 빈도',exact:true}).click();await expect(page.getByTestId('library-card')).toHaveCount(2);
 await page.getByRole('button',{name:'태그 회문',exact:true}).click();await expect(page.getByTestId('library-card')).toHaveCount(1);
 await page.getByRole('searchbox',{name:'문제 검색',exact:true}).fill('PALINDROME');
 await expect(page.getByTestId('library-card')).toHaveCount(1);await page.getByLabel('문제 분류',{exact:true}).selectOption('배열과 문자열');
 const url=page.url();await page.reload();await expect(page.getByRole('searchbox',{name:'문제 검색'})).toHaveValue('PALINDROME');
 await expect(page.getByRole('button',{name:'태그 회문',exact:true})).toHaveAttribute('aria-pressed','true');await expect(page.getByTestId('library-card')).toHaveCount(1);
 await page.getByRole('button',{name:'필터 초기화',exact:true}).click();await expect(page.getByTestId('library-card')).toHaveCount(4);
 await page.goBack();await expect(page).toHaveURL(url);await expect(page.getByTestId('library-card')).toHaveCount(1);
});
test('library has sorting, no-results recovery and no horizontal overflow',async({page})=>{
 await page.goto('thinking-lab/?tag=unknown');await expect(page.getByText('조건에 맞는 문제가 없습니다.',{exact:true})).toBeVisible();
 await page.getByRole('button',{name:'전체 문제 보기',exact:true}).click();await expect(page.getByTestId('library-card')).toHaveCount(4);
 await page.getByLabel('문제 정렬',{exact:true}).selectOption('newest');await expect(page.getByTestId('library-card').first()).toContainText('1.4');
 await page.getByRole('searchbox',{name:'문제 검색'}).fill('1.3');await expect(page.getByTestId('library-card')).toHaveCount(1);
 await expect(page.getByTestId('library-card').first()).toContainText('URLify');
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBe(true);
 await page.getByRole('searchbox',{name:'문제 검색'}).fill('없는검색어');await expect(page.getByTestId('library-card')).toHaveCount(0);
});

test('newly published problems appear without changing a page component',async({page})=>{await page.goto('thinking-lab/');await expect(page.getByTestId('library-card')).toHaveCount(docs.length);});
