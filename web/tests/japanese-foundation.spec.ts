import {test,expect} from '@playwright/test';

test('foundation provides complete filtered curriculum and direct chapter navigation',async({page})=>{
 await page.goto('language-study/');await page.getByRole('link',{name:/일본어 기초 전체 교재/}).click();
 await expect(page).toHaveURL(/language-study\/foundation\/$/);await expect(page.locator('.foundation-unit')).toHaveCount(30);
 await page.getByLabel('기초 교재 영역').selectOption('verbs');await expect(page.locator('.foundation-unit')).toHaveCount(8);
 await page.getByLabel('기초 교재 검색').fill('no-such-foundation');await expect(page.getByRole('heading',{name:'검색 결과가 없습니다.'})).toBeVisible();
 await page.getByLabel('기초 교재 검색').fill('');await page.getByLabel('기초 교재 영역').selectOption('');
 await page.locator('.foundation-unit').nth(13).click();await expect(page).toHaveURL(/foundation\/jf-14\//);await expect(page.locator('.foundation-reader h1')).toBeVisible();
 await page.reload();await expect(page.locator('.foundation-concept')).not.toHaveCount(0);
 await page.getByRole('link',{name:'다음 장',exact:true}).click();await expect(page).toHaveURL(/jf-15/);await page.goBack();await expect(page).toHaveURL(/jf-14/);
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth)).toBe(true);
 await page.goto('language-study/foundation/jf-01/');await page.getByText('문자표 · 92개 소리 확인',{exact:true}).click();await expect(page.locator('.foundation-kana-grid .foundation-sentence')).toHaveCount(92);expect(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth)).toBe(true);
});

test('foundation folded practice and listening preserve separate daily progress and Japanese TTS',async({page})=>{
 await page.addInitScript(()=>{if(!localStorage.getItem('atlas-language-sections-v1'))localStorage.setItem('atlas-language-sections-v1',JSON.stringify({'japanese:9:lang-grammar':true,'english:30:lang-reading':true}));(window as any).__foundationSpoken=[];Object.defineProperty(window,'speechSynthesis',{configurable:true,value:{cancel(){},speak(u:SpeechSynthesisUtterance){(window as any).__foundationSpoken.push({text:u.text,lang:u.lang,rate:u.rate});}}});});
 await page.goto('language-study/foundation/jf-15/');await expect(page.locator('.foundation-reader h1')).toBeVisible();
 const tabs=page.getByRole('tablist',{name:'기초 교재 학습 단계'});
 await tabs.getByRole('tab',{name:'연습',exact:true}).click();const answer=page.locator('.foundation-answer').first();await expect(answer).not.toHaveAttribute('open','');await answer.locator('summary').click();await expect(answer).toHaveAttribute('open','');
 await page.locator('.foundation-mark input').check();
 await tabs.getByRole('tab',{name:'듣기',exact:true}).click();const script=page.locator('.foundation-script');await expect(script).not.toHaveAttribute('open','');
 await page.getByRole('button',{name:'대화 전체 듣기',exact:true}).click();const spoken=await page.evaluate(()=>(window as any).__foundationSpoken.at(-1));expect(spoken.lang).toBe('ja-JP');const data=await(await page.request.get('data/atlas/japanese-foundation.json')).json();expect(spoken.text).toBe(data.units.find((u:any)=>u.id==='jf-15').listening.lines.map((x:any)=>x.tts).join('\n'));expect(spoken.text).not.toMatch(/[가-힣]/);
 await script.locator(':scope > summary').click();await expect(script).toHaveAttribute('open','');await expect(script.locator('.foundation-ja')).not.toHaveCount(0);
 await page.reload();const stored=await page.evaluate(()=>JSON.parse(localStorage.getItem('atlas-language-sections-v1')||'{}'));expect(stored['japanese-foundation:jf-15:practice']).toBe(true);expect(stored['japanese:9:lang-grammar']).toBe(true);expect(stored['english:30:lang-reading']).toBe(true);
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth)).toBe(true);
});

test('foundation every chapter and word collection have usable reading and source content',async({page})=>{
 test.setTimeout(120000);
 const response=await page.request.get('data/atlas/japanese-foundation.json');expect(response.ok()).toBe(true);const course=await response.json();expect(course.units).toHaveLength(30);
 for(const unit of course.units){const navigation=await page.goto('language-study/foundation/'+unit.id+'/');expect(navigation?.status()).toBe(200);await expect(page.locator('.foundation-reader h1')).toHaveText(unit.title);await expect(page.locator('.foundation-concept')).toHaveCount(unit.sections.length);}
 await page.goto('language-study/foundation/?view=words');await expect(page.locator('.foundation-word')).not.toHaveCount(0);await page.getByLabel('기초 교재 검색').fill('no-such-word');await expect(page.getByRole('heading',{name:'검색 결과가 없습니다.'})).toBeVisible();
});


test('foundation merges consecutive filter resets without restoring stale search',async({page})=>{
 await page.goto('language-study/foundation/?q=no-such-foundation&module=verbs');
 await expect(page.getByRole('heading',{name:'검색 결과가 없습니다.'})).toBeVisible();
 await page.evaluate(()=>{
  const input=document.querySelector<HTMLInputElement>('.foundation-toolbar input')!;
  const select=document.querySelector<HTMLSelectElement>('.foundation-toolbar select')!;
  Object.getOwnPropertyDescriptor(HTMLInputElement.prototype,'value')!.set!.call(input,'');
  input.dispatchEvent(new Event('input',{bubbles:true}));
  select.value='';select.dispatchEvent(new Event('change',{bubbles:true}));
 });
 await expect(page).toHaveURL(/language-study\/foundation\/$/);
 await expect(page.locator('.foundation-unit')).toHaveCount(30);
 await page.goBack();
 await expect(page).toHaveURL(/module=verbs/);
 await expect(page.getByLabel('기초 교재 검색')).toHaveValue('');
 await expect(page.locator('.foundation-unit')).toHaveCount(8);
});
