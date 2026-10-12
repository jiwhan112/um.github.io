/** Fail before publishing if generated files would overwrite authoring input. */
import {readFile,stat} from 'node:fs/promises';
const c=JSON.parse(await readFile('dist/data/atlas/catalog.json','utf8'));
const r=JSON.parse(await readFile('dist/data/atlas/routes.json','utf8'));
const routes=new Set(r.map(p=>p.slice(1)+'index.html'));
for(const d of c.documents){if(routes.has(d.source))throw Error('Authoring source must not be index.html: '+d.source+'. Use _content.html or content.json.');}
for(const p of r){if(!/^\/[a-zA-Z0-9/_-]*$/.test(p)||!p.endsWith('/'))throw Error('Unsafe generated route: '+p);}
const html=await readFile('dist/index.html','utf8');
if(!html.includes('/um.github.io/assets/react/')||/atlas\/detail|atlas-theme|learning-lab\.css|<iframe/.test(html))throw Error('Legacy runtime leaked into React entry');
console.log('Publication invariants passed:',r.length,'routes; authoring inputs remain separate.');
function links(value){if(!value||typeof value!=="object")return [];return Object.entries(value).flatMap(([k,v])=>(k==='href'||k==='src')&&typeof v==='string'?[v]:typeof v==='object'?Array.isArray(v)?v.flatMap(links):links(v):[]);}
const broken=[];
for(const d of c.documents){const doc=JSON.parse(await readFile('dist/'+d.file,'utf8'));for(const url of links(doc)){if(!url.startsWith('/')||url.startsWith('//'))continue;const path=url.split(/[?#]/)[0];try{await stat('dist'+path);}catch{broken.push({document:d.route,url});}}}
if(broken.length)throw Error('Broken internal content links: '+JSON.stringify(broken));
console.log('All internal document links resolve to generated routes or preserved resources.');
