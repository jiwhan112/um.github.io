import {test,expect} from '@playwright/test';
const date='2026-11-06',id=date+':0',other=date+':1';
test('read control stays above the article title and preserves original state across navigation',async({page,isMobile})=>{
 await page.addInitScript(({id,other})=>{
  if(!localStorage.getItem('read-control-test-seeded')){
   localStorage.setItem('jiwhan-briefing-v1',JSON.stringify({saved:[id],read:[id,other]}));
   localStorage.setItem('unrelated-value','keep-me');
   localStorage.setItem('read-control-test-seeded','1');
  }
 },{id,other});
 await page.goto('daily-briefing/?date='+date+'&article='+encodeURIComponent(id));
 const openReader=async()=>{if(isMobile)await page.locator('.article-list-item').first().click();};
 await openReader();
 const heading=page.locator('.article-heading');
 const read=heading.getByRole('button',{name:'읽음 완료',exact:true});
 await expect(read).toBeVisible();
 await expect(read).toHaveAttribute('aria-pressed','true');
 const readBox=await read.boundingBox(),titleBox=await heading.locator('h1').boundingBox();
 expect(readBox!.y+readBox!.height).toBeLessThanOrEqual(titleBox!.y);
 await expect(page.locator('.article-actions').getByRole('button',{name:/읽음/})).toHaveCount(0);
 await read.click();
 await expect(heading.getByRole('button',{name:'읽음 표시',exact:true})).toHaveAttribute('aria-pressed','false');
 await page.reload();await openReader();
 await expect(heading.getByRole('button',{name:'읽음 표시',exact:true})).toBeVisible();
 expect(await page.evaluate(()=>JSON.parse(localStorage.getItem('jiwhan-briefing-v1')!))).toEqual({saved:[id],read:[other]});
 await heading.getByRole('button',{name:'읽음 표시',exact:true}).click();
 await expect(heading.getByRole('button',{name:'읽음 완료',exact:true})).toHaveAttribute('aria-pressed','true');
 if(isMobile)await page.getByRole('button',{name:'기사 목록',exact:true}).click();
 await page.locator('.article-list-item').nth(1).click();
 await expect(heading.getByRole('button',{name:'읽음 완료',exact:true})).toBeVisible();
 if(isMobile)await page.getByRole('button',{name:'기사 목록',exact:true}).click();
 await page.locator('.article-list-item').first().click();
 await expect(heading.getByRole('button',{name:'읽음 완료',exact:true})).toBeVisible();
 await page.reload();await openReader();
 expect(await page.evaluate(()=>JSON.parse(localStorage.getItem('jiwhan-briefing-v1')!))).toEqual({saved:[id],read:[other,id]});
 expect(await page.evaluate(()=>localStorage.getItem('unrelated-value'))).toBe('keep-me');
 expect(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1)).toBeTruthy();
});
