const CACHE='language-study-v3';
const ASSETS=['./','index.html','style.css?v=3','app.js?v=3','manifest.webmanifest?v=3','data/catalog.json','data/progress.json'];
self.addEventListener('install',e=>e.waitUntil(caches.open(CACHE).then(c=>c.addAll(ASSETS)).then(()=>self.skipWaiting())));
self.addEventListener('activate',e=>e.waitUntil(caches.keys().then(keys=>Promise.all(keys.filter(k=>k!==CACHE).map(k=>caches.delete(k)))).then(()=>self.clients.claim())));
self.addEventListener('fetch',e=>{
  e.respondWith(fetch(e.request,{cache:'no-store'}).then(r=>{
    const c=r.clone();
    caches.open(CACHE).then(x=>x.put(e.request,c));
    return r;
  }).catch(()=>caches.match(e.request)));
});