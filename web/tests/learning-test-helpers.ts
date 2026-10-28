import {expect,type Page} from '@playwright/test';

export const learningPageSize=12;
export const learningHref=(route:string)=>'/um.github.io'+route;
export const learningRows=(page:Page)=>page.getByTestId('learning-current-group').getByTestId('library-card');
export async function selectLearningGroup(page:Page,id:string,kind:'knowledge'|'game-design'='knowledge'){
 const select=page.getByLabel(kind==='knowledge'?'학습 과정':'강의 주제',{exact:true});
 if((page.viewportSize()?.width||1440)<=760){await expect(select).toBeVisible();await select.selectOption(id);}
 else{
  await expect(page.getByTestId('learning-group-nav')).toBeVisible();
  const target=page.getByTestId('learning-group-nav').locator('button[data-group]');
  // Exact attribute comparison avoids selectors escaping authored punctuation in category names.
  const values=await target.evaluateAll(nodes=>nodes.map(node=>node.getAttribute('data-group')));
  const index=values.indexOf(id);expect(index,`group ${id} is available`).toBeGreaterThanOrEqual(0);
  await target.nth(index).click();
 }
 await expect.poll(()=>new URL(page.url()).searchParams.get(kind==='knowledge'?'course':'topic')).toBe(id);
 await expect(page.getByTestId('learning-current-group')).toHaveAttribute('data-group',id);
 await expect.poll(()=>new URL(page.url()).searchParams.get('page')).toBeNull();
}
export async function expectLearningRoutes(page:Page,expected:string[]){
 const seen:string[]=[];
 for(let start=0;start<expected.length;start+=learningPageSize){
  const chunk=expected.slice(start,start+learningPageSize).map(learningHref);
  await expect(learningRows(page)).toHaveCount(chunk.length);
  await expect(learningRows(page).first()).toHaveAttribute('href',chunk[0]);
  const actual=await learningRows(page).evaluateAll(nodes=>nodes.map(node=>node.getAttribute('href')));
  expect(actual).toEqual(chunk);seen.push(...actual as string[]);
  if(start+learningPageSize<expected.length)await page.getByRole('button',{name:'다음 목록',exact:true}).click();
 }
 expect(seen).toEqual(expected.map(learningHref));expect(new Set(seen).size).toBe(expected.length);
 return seen;
}
export async function revealLearningRoute(page:Page,route:string,orderedRoutes:string[]){
 const position=orderedRoutes.indexOf(route);expect(position,`authored route ${route}`).toBeGreaterThanOrEqual(0);
 for(let pageNumber=1;pageNumber<=Math.floor(position/learningPageSize);pageNumber++){
  await page.getByRole('button',{name:'다음 목록',exact:true}).click();
  await expect.poll(()=>new URL(page.url()).searchParams.get('page')).toBe(String(pageNumber+1));
 }
 const link=page.getByTestId('learning-current-group').locator(`a[href="${learningHref(route)}"]`);
 await expect(link).toHaveCount(1);return link;
}
