import { test, expect, type Page } from '@playwright/test';
import { siteSupabase, parseCloudConfig } from '../src/config/supabase';

const settingsKey='atlas-cloud-settings-v1', sessionKey='atlas-cloud-session-v1';
const oldToken='mock-existing-owner-token';
const localRecords={
 'jiwhan-briefing-v1':JSON.stringify({saved:['2026-10-12:0'],read:['2026-10-12:1']}),
 'atlas-reading-v1':JSON.stringify({'thinking-lab--ep-01-04-palindrome-permutation':true}),
 'atlas-language-sections-v1':JSON.stringify({'japanese:9:lang-grammar':true}),
 'atlas-review-v1':JSON.stringify({'review-example':true}),
 'thinking-ctci-1.4-draft':'// keep my C++ draft',
 'unrelated-value':'keep unrelated settings',
};
async function seed(page:Page,records:Record<string,string>){
 await page.addInitScript(records=>{
  if(localStorage.getItem('__atlas_defaults_seeded'))return;
  for(const [key,value] of Object.entries(records))localStorage.setItem(key,value);
  localStorage.setItem('__atlas_defaults_seeded','1');
 },records);
}
async function openDialog(page:Page){
 await page.getByRole('button',{name:'기록 동기화',exact:true}).click();
 return page.getByRole('dialog');
}
async function mockServer(page:Page,options:{owner?:boolean;url?:string;remote?:Record<string,string>}={}){
 const base=options.url||siteSupabase.url;
 const remote=new Map(Object.entries(options.remote||{}));
 const calls:{path:string;method:string;headers:Record<string,string>}[]=[];
 const cors={'access-control-allow-origin':'*','access-control-allow-headers':'authorization,apikey,content-type,prefer','access-control-allow-methods':'GET,POST,OPTIONS'};
 await page.route(base+'/**',async route=>{
  const request=route.request(),url=new URL(request.url()),method=request.method();
  if(method==='OPTIONS'){await route.fulfill({status:204,headers:cors});return;}
  calls.push({path:url.pathname,method,headers:request.headers()});
  const reply=(body:unknown,status=200)=>route.fulfill({status,headers:cors,contentType:'application/json',body:JSON.stringify(body)});
  if(url.pathname==='/auth/v1/token')return reply({access_token:'mock-refreshed-owner-token',refresh_token:'mock-new-refresh',expires_in:3600});
  if(url.pathname==='/auth/v1/logout')return reply({});
  if(url.pathname==='/rest/v1/atlas_profile')return reply(options.owner===false?[]:[{user_id:1}]);
  if(url.pathname==='/rest/v1/atlas_state'){
   if(method==='POST'){const body=request.postDataJSON() as {storage_key:string;value:string;user_id:number};expect(body.user_id).toBe(1);remote.set(body.storage_key,body.value);return route.fulfill({status:204,headers:cors});}
   return reply([...remote].map(([storage_key,value])=>({storage_key,value})));
  }
  return reply({error:'Unexpected mock endpoint'},400);
 });
 return {calls,remote};
}

test('fresh browser uses site defaults and GitHub redirect without exposing key in URL',async({page},info)=>{
 const external:string[]=[];
 await page.route(siteSupabase.url+'/**',async route=>{
  external.push(route.request().url());
  await route.fulfill({contentType:'text/html',body:'<h1>Mock GitHub authorization</h1>'});
 });
 await page.goto('');const dialog=await openDialog(page);
 await expect(dialog.locator('.sync-status')).toContainText('로그인 필요');
 await expect(dialog.locator('input')).toHaveCount(0);
 expect(external).toEqual([]);
 expect(await page.evaluate(key=>localStorage.getItem(key),settingsKey)).toBeNull();
 await page.screenshot({path:info.outputPath('default-sync-dialog.png')});
 await dialog.getByRole('button',{name:'GitHub 로그인',exact:true}).click();
 await expect(page).toHaveURL(new RegExp(siteSupabase.url.replace(/[.*+?^${}()|[\]\\]/g,'\\$&')+'/auth/v1/authorize'));
 const url=new URL(page.url());
 expect(url.searchParams.get('provider')).toBe('github');
 expect(url.searchParams.get('redirect_to')).toBe('http://127.0.0.1:4173/um.github.io/');
 expect(page.url()).not.toContain(siteSupabase.key);
});

test('existing same-project login and every local record survive adopting public defaults',async({page})=>{
 const session={access_token:oldToken,refresh_token:'mock-refresh',expires_at:Date.now()+3600000};
 await seed(page,{...localRecords,[settingsKey]:JSON.stringify({url:siteSupabase.url+'/',key:'sb_publishable_previous_public_key'}),[sessionKey]:JSON.stringify(session)});
 const server=await mockServer(page);
 await page.goto('');const dialog=await openDialog(page);
 await expect(dialog.locator('.sync-status')).toContainText('동기화됨');
 const stored=await page.evaluate(()=>({...localStorage}));
 for(const [key,value] of Object.entries(localRecords))expect(stored[key]).toBe(value);
 expect(JSON.parse(stored[sessionKey])).toEqual(session);
 expect(server.calls.some(x=>x.path==='/auth/v1/token')).toBe(false);
 for(const call of server.calls.filter(x=>x.path.startsWith('/rest/'))){expect(call.headers.apikey).toBe(siteSupabase.key);expect(call.headers.authorization).toBe('Bearer '+oldToken);}
 expect(server.remote.has('unrelated-value')).toBe(false);
 expect(server.remote.get('thinking-ctci-1.4-draft')).toBe(localRecords['thinking-ctci-1.4-draft']);
 await page.reload();await openDialog(page);await expect(page.locator('.sync-status')).toContainText('동기화됨');
 expect(await page.evaluate(key=>JSON.parse(localStorage.getItem(key)!).access_token,sessionKey)).toBe(oldToken);
});

test('OAuth return on a new device works with no saved settings and removes token hash',async({page})=>{
 const server=await mockServer(page,{remote:{'atlas-reading-v1':'{"mock-remote-document":true}'}});
 await page.goto('#access_token=mock-callback-token&refresh_token=mock-callback-refresh&expires_in=3600');
 await openDialog(page);await expect(page.locator('.sync-status')).toContainText('동기화됨');
 expect(new URL(page.url()).hash).toBe('');
 expect(await page.evaluate(key=>localStorage.getItem(key),settingsKey)).toBeNull();
 expect(await page.evaluate(()=>localStorage.getItem('atlas-reading-v1'))).toBe('{"mock-remote-document":true}');
 expect(server.calls.find(x=>x.path==='/rest/v1/atlas_profile')?.headers.authorization).toBe('Bearer mock-callback-token');
 await page.getByRole('dialog').getByRole('button',{name:'로그아웃',exact:true}).click();
 await expect(page.locator('.sync-status')).toContainText('로그인 필요');
 expect(await page.evaluate(key=>localStorage.getItem(key),sessionKey)).toBeNull();
 expect(await page.evaluate(()=>localStorage.getItem('atlas-reading-v1'))).toBe('{"mock-remote-document":true}');
});

test('expired same-project session refreshes using the default public key',async({page})=>{
 await seed(page,{...localRecords,[settingsKey]:JSON.stringify(siteSupabase),[sessionKey]:JSON.stringify({access_token:oldToken,refresh_token:'mock-old-refresh',expires_at:1})});
 const server=await mockServer(page);
 await page.goto('');await openDialog(page);await expect(page.locator('.sync-status')).toContainText('동기화됨');
 const refresh=server.calls.find(x=>x.path==='/auth/v1/token');expect(refresh).toBeDefined();expect(refresh?.headers.apikey).toBe(siteSupabase.key);
 expect(server.calls.find(x=>x.path==='/rest/v1/atlas_profile')?.headers.authorization).toBe('Bearer mock-refreshed-owner-token');
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1.4-draft'))).toBe(localRecords['thinking-ctci-1.4-draft']);
});

test('unmapped owner does not upload or read personal state',async({page})=>{
 await seed(page,{...localRecords,[sessionKey]:JSON.stringify({access_token:oldToken,refresh_token:'mock-refresh',expires_at:Date.now()+3600000})});
 const server=await mockServer(page,{owner:false});
 await page.goto('');const dialog=await openDialog(page);
 await expect(dialog.locator('.sync-status')).toContainText('동기화 실패');
 await dialog.getByRole('button',{name:'지금 동기화'}).click();
 await expect(dialog).toContainText('사용자 1번이 아직 이 GitHub 계정에 연결되지 않았습니다');
 expect(server.calls.some(x=>x.path==='/rest/v1/atlas_state')).toBe(false);
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1.4-draft'))).toBe(localRecords['thinking-ctci-1.4-draft']);
});

test('a different legacy project is not silently replaced and switch requires new login',async({page})=>{
 const previous={url:'https://previous-atlas-project.supabase.co',key:'sb_publishable_mock_previous_project'};
 await seed(page,{...localRecords,[settingsKey]:JSON.stringify(previous),[sessionKey]:JSON.stringify({access_token:oldToken,refresh_token:'mock-refresh',expires_at:Date.now()+3600000})});
 const oldServer=await mockServer(page,{url:previous.url}),newServer=await mockServer(page);
 await page.goto('');const dialog=await openDialog(page);
 await expect(dialog.locator('.sync-status')).toContainText('동기화됨');
 await expect(dialog).toContainText('다른 서버의 연결 설정');
 expect(newServer.calls).toEqual([]);expect(oldServer.calls.length).toBeGreaterThan(0);
 await dialog.getByRole('button',{name:'ATLAS NOTE 기본 서버로 전환'}).click();
 await expect(dialog.locator('.sync-status')).toContainText('로그인 필요');
 await expect(dialog).toContainText('서버가 기본 설정되어 있습니다');
 expect(await page.evaluate(key=>localStorage.getItem(key),sessionKey)).toBeNull();
 expect(await page.evaluate(()=>localStorage.getItem('thinking-ctci-1.4-draft'))).toBe(localRecords['thinking-ctci-1.4-draft']);
 expect(newServer.calls).toEqual([]);
});

test('corrupt settings fall back safely; privileged API keys are not accepted as public config',async({page})=>{
 const serviceJWT='e30.'+Buffer.from(JSON.stringify({role:'service_role'})).toString('base64url')+'.test';
 expect(parseCloudConfig({url:siteSupabase.url,key:'sb_secret_do_not_accept'})).toBeNull();
 expect(parseCloudConfig({url:siteSupabase.url,key:serviceJWT})).toBeNull();
 expect(parseCloudConfig(siteSupabase)).toEqual(siteSupabase);
 await seed(page,{[settingsKey]:'{"url":123,"key":null}','unrelated-value':'still here'});
 await page.goto('');const dialog=await openDialog(page);
 await expect(dialog.locator('.sync-status')).toContainText('로그인 필요');
 await expect(dialog).toContainText('서버가 기본 설정되어 있습니다');
 await expect(dialog.locator('input')).toHaveCount(0);
 expect(await page.evaluate(()=>localStorage.getItem('unrelated-value'))).toBe('still here');
});
