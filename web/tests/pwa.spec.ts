import {test,expect} from '@playwright/test';
test.use({serviceWorkers:'allow'});
test('language worker preserves unrelated cache and storage, then opens a cached lesson offline',async({page,context})=>{
 test.setTimeout(60000);
 await page.goto('build-info.json');
 await page.evaluate(async()=>{await caches.open('language-study-v3');const c=await caches.open('other-app-cache');await c.put('/keep',new Response('keep'));localStorage.setItem('other-app-value','keep');});
 await page.goto('language-study/japanese/9/');await expect(page.locator('main h1')).toBeVisible();
 await page.evaluate(async()=>{await navigator.serviceWorker.ready;});
 await page.waitForFunction(()=>!!navigator.serviceWorker.controller);
 await expect.poll(()=>page.evaluate(async()=>(await caches.keys()).includes('language-study-v3'))).toBe(false);
 expect(await page.evaluate(async()=>(await caches.keys()).includes('other-app-cache'))).toBe(true);
 expect(await page.evaluate(()=>localStorage.getItem('other-app-value'))).toBe('keep');
 await context.setOffline(true);await page.reload();await expect(page.getByRole('tab',{name:'문법',exact:true})).toBeVisible();
 await page.getByRole('tab',{name:'예문',exact:true}).click();await expect(page.getByRole('button',{name:'문장 1 듣기',exact:true})).toBeVisible();
 await context.setOffline(false);
});
