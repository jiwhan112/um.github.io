import { createContext, useContext, useEffect, useState, type ReactNode } from 'react';
import { z } from 'zod';
import { asset } from './utils';
import { catalogSchema, articleSchema, documentSchema, lessonSchema, type Catalog, type Article } from './types';

export async function fetchData<T>(path:string,schema:z.ZodType<T>,signal?:AbortSignal):Promise<T>{
 const r=await fetch(asset(path),{cache:'no-store',signal});if(!r.ok)throw Error(`자료 요청 실패 (${r.status})`);return schema.parse(await r.json());
}
export function useResource<T>(path:string|null,schema:z.ZodType<T>){
 const [state,set]=useState<{path:string|null;data?:T;error?:string}>({path:null});
 useEffect(()=>{if(!path)return;const controller=new AbortController();fetchData(path,schema,controller.signal).then(data=>set({path,data})).catch(e=>{if(e.name!=='AbortError')set({path,error:'자료를 불러오지 못했습니다. 연결 또는 데이터 형식을 확인해 주세요.'});});return()=>controller.abort();},[path,schema]);
 return state.path===path?state:{path};
}
const DataContext=createContext<{catalog:Catalog;articles:Article[]}|null>(null);
const articlesSchema=z.array(articleSchema);
export function DataProvider({children}:{children:ReactNode}){
 const c=useResource('data/atlas/catalog.json',catalogSchema),a=useResource('data/atlas/articles.json',articlesSchema);
 if(c.error||a.error)return <div className="boot-screen"><h1>자료를 불러올 수 없습니다.</h1><p>{c.error||a.error}</p><button onClick={()=>location.reload()}>다시 시도</button></div>;
 if(!c.data||!a.data)return <div className="boot-screen" role="status"><span className="brand-symbol">✦</span><strong>ATLAS NOTE</strong><p>학습 라이브러리를 불러옵니다.</p></div>;
 return <DataContext.Provider value={{catalog:c.data as Catalog,articles:a.data}}>{children}</DataContext.Provider>;
}
export function useData(){const value=useContext(DataContext);if(!value)throw Error('DataProvider required');return value;}
export {documentSchema,lessonSchema};
