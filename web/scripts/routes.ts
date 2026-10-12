/** Real files preserve deep links on GitHub Pages, which has no SPA rewrite rule. */
import { readFile, writeFile, mkdir, readdir } from 'node:fs/promises';
import { resolve, dirname, join } from 'node:path';
import { createHash } from 'node:crypto';
const dist=resolve('dist'),html=await readFile(join(dist,'index.html'),'utf8');
const routes: string[]=JSON.parse(await readFile(join(dist,'data/atlas/routes.json'),'utf8'));
for(const route of routes){const path=join(dist,route,'index.html');await mkdir(dirname(path),{recursive:true});await writeFile(path,html);}
await writeFile(join(dist,'404.html'),html);await writeFile(join(dist,'.nojekyll'),'');
const catalog=JSON.parse(await readFile(join(dist,'data/atlas/catalog.json'),'utf8'));
const base='/um.github.io/';
const assets=[base+'language-study/',...(await readdir(join(dist,'assets/react'))).filter(p=>/\.(js|css)$/.test(p)).map(p=>base+'assets/react/'+p)];
const precache=[...assets,base+'data/atlas/catalog.json',base+'data/atlas/articles.json',...Object.values(catalog.languages as Record<string,{file:string}[]>).flat().map(m=>base+'language-study/'+m.file)];
const cache='language-study-react-'+createHash('sha256').update(html+catalog.revision).digest('hex').slice(0,12);
// Same scope as the existing language PWA; only language-study-* caches are retired.
const worker=`const CACHE=${JSON.stringify(cache)};const ASSETS=${JSON.stringify(precache)};
self.addEventListener('install',e=>e.waitUntil(caches.open(CACHE).then(c=>c.addAll(ASSETS)).then(()=>self.skipWaiting())));
self.addEventListener('activate',e=>e.waitUntil((async()=>{const keys=await caches.keys();await Promise.all(keys.filter(k=>k.startsWith('language-study-')&&k!==CACHE).map(k=>caches.delete(k)));await self.clients.claim();})()));
self.addEventListener('fetch',e=>{if(e.request.method!=='GET'||new URL(e.request.url).origin!==self.location.origin)return;e.respondWith((async()=>{const cache=await caches.open(CACHE);try{const r=await fetch(e.request,{cache:'no-store'});if(r.ok){const copy=r.clone();e.waitUntil(cache.put(e.request,copy).catch(()=>{}));}return r;}catch{const cached=await cache.match(e.request);if(cached)return cached;if(e.request.mode==='navigate')return await cache.match(${JSON.stringify(base+'language-study/')});return new Response('오프라인입니다. 자료를 먼저 온라인에서 열어 주세요.',{status:503,headers:{'Content-Type':'text/plain; charset=utf-8'}});}})());});`;
await writeFile(join(dist,'language-study/sw.js'),worker);
await writeFile(join(dist,'sitemap.xml'),'<?xml version="1.0" encoding="UTF-8"?><urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">'+routes.map(r=>'<url><loc>https://jiwhan112.github.io/um.github.io'+r+'</loc></url>').join('')+'</urlset>');
await writeFile(join(dist,'robots.txt'),'User-agent: *\nAllow: /\nSitemap: https://jiwhan112.github.io/um.github.io/sitemap.xml\n');
console.log(`Generated ${routes.length} direct-entry routes and scoped PWA worker`);
