import { localChanged } from './cloud';
import { useCallback, useMemo, useSyncExternalStore } from 'react';
const eventName='atlas-storage';
export const keys={briefing:'jiwhan-briefing-v1',reading:'atlas-reading-v1',language:'atlas-language-sections-v1',review:'atlas-review-v1'} as const;
function read(key:string){try{return localStorage.getItem(key);}catch{return null;}}
function subscribe(notify:()=>void){window.addEventListener('storage',notify);window.addEventListener(eventName,notify);return()=>{window.removeEventListener('storage',notify);window.removeEventListener(eventName,notify);};}
export function parseStored<T>(serialized:string|null,fallback:T):T {if(serialized===null)return fallback;try{return JSON.parse(serialized) as T;}catch{return fallback;}}
export function useStored<T>(key:string,fallback:T,raw=false){
 const snapshot=useCallback(()=>read(key),[key]);
 const serialized=useSyncExternalStore(subscribe,snapshot,()=>null);
 const value=useMemo(()=>raw?(serialized??fallback) as T:parseStored(serialized,fallback),[serialized,fallback,raw]);
 const set=useCallback((next:T|((old:T)=>T))=>{try{const old=raw?(read(key)??fallback) as T:parseStored(read(key),fallback);const v=typeof next==='function'?(next as (old:T)=>T)(old):next;localStorage.setItem(key,raw?String(v):JSON.stringify(v));localChanged(key);window.dispatchEvent(new Event(eventName));return true;}catch{return false;}},[key,raw,fallback]);
 return [value,set] as const;
}
export function objectBooleans(value:unknown):Record<string,boolean>{return value!==null&&typeof value==='object'&&!Array.isArray(value)?Object.fromEntries(Object.entries(value).filter(([,v])=>typeof v==='boolean')):{};}
export function briefingState(value:unknown):{saved:string[];read:string[]}{const v=(value&&typeof value==='object'?value:{}) as Record<string,unknown>;const a=(x:unknown)=>Array.isArray(x)?x.filter((s):s is string=>typeof s==='string'):[];return {saved:a(v.saved),read:a(v.read)};}
