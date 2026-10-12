/** Publish only generated runtime files to the repository's existing master/root Pages source. */
import {readFile,writeFile,readdir,mkdir,copyFile} from 'node:fs/promises';
import {resolve,join,dirname,relative} from 'node:path';
import {execFileSync} from 'node:child_process';
const repo=resolve('..'),dist=resolve(process.env.ATLAS_DIST||'dist');
const git=(...args)=>execFileSync('git',args,{cwd:repo,encoding:'utf8'}).trim();
const head=git('rev-parse','HEAD');
git('fetch','origin','master');
if(git('rev-parse','origin/master')!==head)throw Error('New source changes arrived during validation. Refusing stale publication; the queued build will publish them.');
const info=JSON.parse(await readFile(join(dist,'build-info.json'),'utf8'));
if(info.framework!=='React')throw Error('Invalid distribution');
info.sourceCommit=head;await writeFile(join(dist,'build-info.json'),JSON.stringify(info)+'\n');
const routes=JSON.parse(await readFile(join(dist,'data/atlas/routes.json'),'utf8'));
const routeFiles=new Set(routes.map(r=>r.replace(/^\//,'')+'index.html'));
const catalog=JSON.parse(await readFile(join(dist,'data/atlas/catalog.json'),'utf8'));
for(const d of catalog.documents)if(routeFiles.has(d.source))throw Error('Refusing to overwrite authoring source: '+d.source);
const fixed=new Set(['index.html','404.html','.nojekyll','favicon.svg','build-info.json','robots.txt','sitemap.xml','language-study/sw.js']);
const allowed=p=>fixed.has(p)||routeFiles.has(p)||p.startsWith('assets/react/')||p.startsWith('data/atlas/');
async function walk(dir){return (await Promise.all((await readdir(dir,{withFileTypes:true})).map(e=>e.isDirectory()?walk(join(dir,e.name)):Promise.resolve([join(dir,e.name)])))).flat();}
const files=[];
for(const file of await walk(dist)){const p=relative(dist,file).replaceAll('\\','/');if(!allowed(p))continue;if(p.includes('..'))throw Error('Invalid path');await mkdir(dirname(join(repo,p)),{recursive:true});await copyFile(file,join(repo,p));files.push(p);}
// Keep older hashed bundles available for already-open browser sessions. Never touch authoring JSON/PNG.
const manifest='_maintenance/react-generated-files.json';
await mkdir(join(repo,'_maintenance'),{recursive:true});await writeFile(join(repo,manifest),JSON.stringify({sourceCommit:head,revision:info.revision,files:files.sort()},null,2)+'\n');
files.push(manifest);
git('config','user.name','github-actions[bot]');git('config','user.email','41898282+github-actions[bot]@users.noreply.github.com');
git('add','--',...files);
let changed=true;try{git('diff','--cached','--quiet');changed=false;}catch{}
if(changed){git('commit','-m',`build(react): publish validated ${head.slice(0,7)} [skip ci]`);git('push','origin','HEAD:master');}
const published=git('rev-parse','HEAD');
console.log('Published runtime commit',published,'from tested source',head);
if(process.env.GITHUB_OUTPUT)await writeFile(process.env.GITHUB_OUTPUT,`commit=${published}\n`,{flag:'a'});
