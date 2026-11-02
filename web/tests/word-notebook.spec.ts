import {test,expect} from '@playwright/test';
test('notebooks expose source provenance, folded meaning, filters and stable navigation',async({page})=>{
 await page.goto('language-study/');await page.getByRole('link',{name:/단어 노트로 다시 익히기/}).click();
 await expect(page).toHaveURL(/language-study\/notebooks\//);await expect(page.getByRole('heading',{name:'읽었던 말, 내 단어로.'})).toBeVisible();
 const card=page.locator('.notebook-card').first();await expect(card).toBeVisible();await expect(card.locator('.notebook-meanings')).toHaveCount(0);
 await card.getByRole('button',{name:'뜻 보기',exact:true}).click();await expect(card.locator('.notebook-meanings')).toBeVisible();
 await card.locator('.notebook-sources summary').click();await expect(card.locator('.notebook-source a').first()).toHaveAttribute('href',/language-study\/english\//);
 await page.getByLabel('원래 레슨 필터').selectOption('30');await expect(page.locator('.notebook-card')).not.toHaveCount(0);
 await page.getByLabel('단어 노트 검색').fill('zzzz-no-match');await expect(page.getByRole('heading',{name:'찾는 단어가 없습니다.'})).toBeVisible();
 await page.getByLabel('단어 노트 종류').getByRole('button',{name:/일본어 단어 노트/}).click();await expect(page.locator('.notebook-card')).not.toHaveCount(0);
 await page.getByLabel('단어 노트 종류').getByRole('button',{name:/자주 만난 한자어/}).click();await expect(page.locator('.notebook-intro')).toContainText('일반 일본어 사용 빈도 순위가 아니며');
 await page.reload();await expect(page.locator('.notebook-intro h2')).toHaveText('자주 만난 한자어');
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth)).toBe(true);
});
test('notebook speech uses original words without changing old learning records',async({page})=>{
 await page.addInitScript(()=>{localStorage.setItem('atlas-language-sections-v1',JSON.stringify({'japanese:9:lang-grammar':true}));(window as any).__spoken=[];Object.defineProperty(window,'speechSynthesis',{configurable:true,value:{cancel(){},speak(u:SpeechSynthesisUtterance){(window as any).__spoken.push({text:u.text,lang:u.lang});}}});});
 await page.goto('language-study/notebooks/?tab=japanese');const card=page.locator('.notebook-card').first();await expect(card).toBeVisible();await card.getByRole('button',{name:/ 듣기$/}).first().click();
 expect(await page.evaluate(()=>(window as any).__spoken[0].lang)).toBe('ja-JP');
 expect(await page.evaluate(()=>JSON.parse(localStorage.getItem('atlas-language-sections-v1')||'{}')['japanese:9:lang-grammar'])).toBe(true);
});
