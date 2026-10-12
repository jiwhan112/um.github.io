/* Network-first PWA assets. Do not delete caches belonging to other Atlas sections. */
const CACHE='language-study-20261012d1';
const ASSETS=['./','index.html','style.css?v=4','app.js?v=20261012d1','atlas-theme.css?v=20261012d1','manifest.webmanifest?v=3','data/catalog.json','data/progress.json','../assets/atlas/detail.css?v=20261012d1','../assets/atlas/detail.js?v=20261012d1'];
self.addEventListener('install',event=>event.waitUntil(caches.open(CACHE).then(c=>c.addAll(ASSETS)).then(()=>self.skipWaiting())));
self.addEventListener('activate',event=>event.waitUntil(caches.keys().then(keys=>Promise.all(keys.filter(k=>k.startsWith('language-study-')&&k!==CACHE).map(k=>caches.delete(k)))).then(()=>self.clients.claim())));
self.addEventListener('fetch',event=>{
 if(event.request.method!=='GET'||new URL(event.request.url).origin!==self.location.origin)return;
 event.respondWith(fetch(event.request,{cache:'no-store'}).then(response=>{
  if(response.ok){const copy=response.clone();event.waitUntil(caches.open(CACHE).then(c=>c.put(event.request,copy)).catch(()=>{}));}return response;
 }).catch(async()=>{const cache=await caches.open(CACHE);return (await cache.match(event.request))||new Response('오프라인입니다. 이 자료를 먼저 온라인에서 열어 주세요.',{status:503,headers:{'Content-Type':'text/plain; charset=utf-8'}});}));
});
