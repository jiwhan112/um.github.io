/** Optional owner-only cloud mirror. Local storage remains the offline source until a successful sync. */
const settingsKey = 'atlas-cloud-settings-v1';
const sessionKey = 'atlas-cloud-session-v1';
const pendingKey = 'atlas-cloud-pending-v1';
const eventName = 'atlas-cloud-state';
const storageEvent = 'atlas-storage';
type Config = {url:string;key:string};
type Session = {access_token:string;refresh_token:string;expires_at:number};
type Row = {storage_key:string;value:string};
let state = '미연결';
let running:Promise<void>|null = null;
let retry = false;
let started = false;
let lastSynced = '';
const supported = new Set(['jiwhan-briefing-v1','atlas-reading-v1','atlas-language-sections-v1','atlas-review-v1']);
export function canSyncKey(key:string):boolean {
 return supported.has(key)||/^thinking-ctci-[a-z0-9.-]+-draft$/.test(key)||/^atlas-code-[a-z0-9-]+$/.test(key);
}
function load<T>(key:string,fallback:T):T {try{const s=localStorage.getItem(key);return s?JSON.parse(s) as T:fallback;}catch{return fallback;}}
function save(key:string,value:unknown){localStorage.setItem(key,JSON.stringify(value));}
function announce(value:string){state=value;window.dispatchEvent(new Event(eventName));}
export function cloudStatus(){return state+(lastSynced?' · '+lastSynced:'');}
export function cloudSubscribe(notify:()=>void){window.addEventListener(eventName,notify);return()=>window.removeEventListener(eventName,notify);}
export function cloudSettings():Config {return load<Config>(settingsKey,{url:'',key:''});}
export function saveCloudSettings(config:Config){
 const url=config.url.trim().replace(/\/+$/,'');
 const parsed=new URL(url);
 if(parsed.protocol!=='https:'||!/^https:\/\/[a-z0-9-]+\.supabase\.co$/.test(parsed.origin))throw Error('Supabase 프로젝트의 https://...supabase.co 주소가 필요합니다.');
 if(!config.key.trim())throw Error('공개용 anon/publishable key가 필요합니다.');
 if(/^(sb_secret_|service_role)/i.test(config.key.trim()))throw Error('비공개 secret/service_role 키는 사용할 수 없습니다.');
 const old=cloudSettings();
 if(old.url!==url||old.key!==config.key.trim())localStorage.removeItem(sessionKey);
 save(settingsKey,{url,key:config.key.trim()});announce('로그인 필요');
}
function session(){return load<Session|null>(sessionKey,null);}
function validConfig(c:Config){return Boolean(c.url&&c.key);}
function headers(c:Config,token:string):HeadersInit{return {'apikey':c.key,'Authorization':'Bearer '+token,'Content-Type':'application/json'};}
async function currentToken(c:Config){
 const s=session();if(!s)throw Error('GitHub 계정으로 로그인해 주세요.');
 if(Date.now()<s.expires_at-60000)return s.access_token;
 const res=await fetch(c.url+'/auth/v1/token?grant_type=refresh_token',{method:'POST',headers:{apikey:c.key,'Content-Type':'application/json'},body:JSON.stringify({refresh_token:s.refresh_token})});
 if(!res.ok){localStorage.removeItem(sessionKey);throw Error('로그인 세션이 만료되었습니다. 다시 로그인해 주세요.');}
 const data=await res.json() as {access_token:string;refresh_token:string;expires_in:number};
 save(sessionKey,{access_token:data.access_token,refresh_token:data.refresh_token,expires_at:Date.now()+data.expires_in*1000});
 return data.access_token;
}
function pending(){return load<Record<string,number>>(pendingKey,{});}
function writePending(value:Record<string,number>){save(pendingKey,value);}
export function localChanged(key:string){
 if(!canSyncKey(key))return;
 writePending({...pending(),[key]:Date.now()});
 if(session())void syncCloud().catch(()=>{});
}
export function signInCloud(){
 const c=cloudSettings();if(!validConfig(c))throw Error('먼저 Supabase 설정을 저장해 주세요.');
 const callback=new URL(import.meta.env.BASE_URL,location.origin).href;
 const u=new URL(c.url+'/auth/v1/authorize');
 u.searchParams.set('provider','github');u.searchParams.set('redirect_to',callback);
 location.assign(u.href);
}
export async function signOutCloud(){
 const c=cloudSettings(),s=session();localStorage.removeItem(sessionKey);lastSynced='';
 announce(validConfig(c)?'로그인 필요':'미연결');
 if(s&&validConfig(c))await fetch(c.url+'/auth/v1/logout',{method:'POST',headers:headers(c,s.access_token)}).catch(()=>{});
}
export function startCloud(){
 if(started)return;started=true;
 const c=cloudSettings();
 const hash=new URLSearchParams(location.hash.slice(1));
 if(hash.has('access_token')&&hash.has('refresh_token')&&validConfig(c)){
  const expires=Number(hash.get('expires_in')||'3600');
  save(sessionKey,{access_token:hash.get('access_token')!,refresh_token:hash.get('refresh_token')!,expires_at:Date.now()+expires*1000});
  history.replaceState(history.state,'',location.pathname+location.search);
 }
 if(!validConfig(c))announce('미연결');
 else if(!session())announce('로그인 필요');
 else void syncCloud().catch(()=>{});
 window.addEventListener('online',()=>{if(session())void syncCloud().catch(()=>{});});
 window.addEventListener('focus',()=>{if(session())void syncCloud().catch(()=>{});});
}
export async function syncCloud():Promise<void>{
 if(running){retry=true;return running;}
 const c=cloudSettings();if(!validConfig(c)){announce('미연결');return;}
 if(!session()){announce('로그인 필요');return;}
 running=(async()=>{
  announce('동기화 중');
  const token=await currentToken(c);
  const ownerResponse=await fetch(c.url+'/rest/v1/atlas_profile?select=user_id&user_id=eq.1',{headers:headers(c,token),cache:'no-store'});
  if(!ownerResponse.ok)throw Error('계정 확인 실패 ('+ownerResponse.status+'). Supabase 정책을 확인해 주세요.');
  const owners=await ownerResponse.json() as {user_id:number}[];
  if(owners.length!==1||owners[0].user_id!==1)throw Error('사용자 1번이 아직 이 GitHub 계정에 연결되지 않았습니다. Supabase 초기 설정을 확인해 주세요.');
  const response=await fetch(c.url+'/rest/v1/atlas_state?select=storage_key,value&user_id=eq.1',{headers:headers(c,token),cache:'no-store'});
  if(!response.ok)throw Error('클라우드 읽기 실패 ('+response.status+'). 사용자 1번의 권한과 테이블 구성을 확인해 주세요.');
  const records=await response.json() as Row[];
  const remote=new Map(records.filter(r=>canSyncKey(r.storage_key)&&typeof r.value==='string').map(r=>[r.storage_key,r.value]));
  const changed=pending();
  for(const [key,value] of remote){
   if(changed[key])continue;
   if(localStorage.getItem(key)!==value){const old=localStorage.getItem(key);if(old!==null){const previous=load<Record<string,string>>('atlas-cloud-conflicts-v1',{});save('atlas-cloud-conflicts-v1',{...previous,[key]:old});}localStorage.setItem(key,value);window.dispatchEvent(new Event(storageEvent));}
  }
  const localKeys=Object.keys(localStorage).filter(canSyncKey);
  for(const key of localKeys){
   if(!remote.has(key)||changed[key]){
    const value=localStorage.getItem(key);if(value===null)continue;
    const updated=changed[key];
    const write=await fetch(c.url+'/rest/v1/atlas_state?on_conflict=user_id,storage_key',{
     method:'POST',headers:{...headers(c,token),'Prefer':'resolution=merge-duplicates,return=minimal'},
     body:JSON.stringify({user_id:1,storage_key:key,value})});
    if(!write.ok)throw Error('클라우드 저장 실패 ('+write.status+'). 계정 권한과 RLS 정책을 확인해 주세요.');
    if(localStorage.getItem(key)===value){
     const now=pending();if(now[key]===updated){delete now[key];writePending(now);}
    }
   }
  }
  lastSynced=new Intl.DateTimeFormat('ko-KR',{hour:'2-digit',minute:'2-digit'}).format(new Date());
  announce('동기화됨');
 })().catch((error:unknown)=>{announce('동기화 실패');throw error;}).finally(()=>{running=null;if(retry){retry=false;void syncCloud().catch(()=>{});}});
 return running;
}
