import {test,expect} from '@playwright/test';
import {readFileSync} from 'node:fs';
// Stable author-approved fixtures prevent scheduled news updates from changing UI baselines.
// Functional tests above always use the complete current production data instead.
const catalog=readFileSync('tests/fixtures/catalog.json','utf8');
const articles=readFileSync('tests/fixtures/articles.json','utf8');
test.beforeEach(async({page})=>{
 await page.route('**/data/atlas/catalog.json',route=>route.fulfill({contentType:'application/json',body:catalog}));
 await page.route('**/data/atlas/articles.json',route=>route.fulfill({contentType:'application/json',body:articles}));
});
for(const [name,path] of Object.entries({dashboard:'',briefing:'daily-briefing/',language:'language-study/japanese/9/',problem:'thinking-lab/ep-01-04-palindrome-permutation/',gallery:'ai-asset-lab/',library:'thinking-lab/',report:'ai-asset-lab/experiments/2d/001-potion-icons/report/'})){
 test('visual '+name,async({page,isMobile})=>{
  await page.goto(path);await expect(page.locator('main h1').first()).toBeVisible();
  if(name==='briefing'&&isMobile)await page.locator('.article-list-item').first().click();
  if(name==='library')await page.locator('#problem-library').scrollIntoViewIfNeeded();
  await page.evaluate(()=>document.fonts.ready);await expect(page).toHaveScreenshot(name+'.png',{fullPage:false});
 });
}
