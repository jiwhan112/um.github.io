/** Build-time compatibility adapter. Never ships the old document DOM, styles or scripts. */
import { readFile, writeFile, mkdir, readdir, rm, cp, stat } from 'node:fs/promises';
import { resolve, dirname, relative, extname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createHash } from 'node:crypto';
import { load, type CheerioAPI } from 'cheerio';
import type { AnyNode, Element } from 'domhandler';
import { marked } from 'marked';
import {validatedExampleBundles} from './example-bundles';
import { z } from 'zod';
import { articleSchema, documentSchema, lessonSchema, type Block, type Inline, type Document, type Catalog, type Article, type AssetMeta, type Language } from '../src/lib/types';

const root=resolve(dirname(fileURLToPath(import.meta.url)),'../..');
const out=join(root,'web/public');
const digest=(s:string)=>createHash('sha256').update(s).digest('hex');
const text=(s:string)=>s.replace(/\s+/g,' ').trim();
const title=(s:string)=>text(s).replace(/^[\p{Extended_Pictographic}\uFE0F\s]+/u,'').replace(/^\d+[.·]\s*/,'');
const json=async(p:string)=>JSON.parse(await readFile(join(root,p),'utf8'));
async function exists(p:string){try{await stat(p);return true;}catch{return false;}}
async function write(p:string,d:unknown){await mkdir(dirname(p),{recursive:true});await writeFile(p,typeof d==='string'?d:JSON.stringify(d));}
export function safeLink(href:string, route:string):string {
 if(!href||/^(javascript|data|vbscript):/i.test(href.trim()))return '';
 if(href.startsWith('#'))return href;
 try {const u=new URL(href,'https://atlas.invalid/'+route.replace(/^\//,''));
 if(u.origin==='https://atlas.invalid')return u.pathname.replace(/\/report\.md$/,'/report/').replace(/\/prompts-v1\.md$/,'/prompts-v1/')+u.search+u.hash;
 return /^https?:$/.test(u.protocol)?u.href:'';}catch{return '';}
}
export function compileBlocks(html:string,route='/'):Block[]{const $=load(html);return blocks($,$('body').contents().toArray(),route);}
function inline($:CheerioAPI,nodes:AnyNode[],route:string,kind:Inline['kind']='text'):Inline[]{
 return nodes.flatMap(n=>{
  if(n.type==='text')return [{text:n.data,kind}];
  if(n.type!=='tag')return [];
  if(n.name==='br')return [{text:'\n',kind:'br' as const}];
  if(['script','style','button','input'].includes(n.name))return [];
  const mark=({strong:'strong',b:'strong',em:'em',i:'em',code:'code'} as const)[n.name as 'strong'];
  if(n.name==='a'){const href=safeLink($(n).attr('href')||'',route);return [{text:$(n).text(),kind:href?'link' as const:'text' as const,href}];}
  return inline($,n.children,route,mark||kind);
 });
}
function blocks($:CheerioAPI,nodes:AnyNode[],route:string):Block[]{
 const result:Block[]=[];let pending:AnyNode[]=[];
 const flush=()=>{if(pending.length){const a=inline($,pending,route);if(a.some(x=>x.text.trim()))result.push({type:'paragraph',content:a});pending=[];}};
 for(const n of nodes){
  if(n.type==='text'){if(n.data.trim())pending.push(n);continue;}
  if(n.type!=='tag')continue;
  const $n=$(n),tag=n.name,cls=$n.attr('class')||'';
  if(['script','style','nav','footer','button','textarea','input','noscript'].includes(tag)||/^(tools|jump|links|anchor-nav)$/.test(cls)||['save-msg','save-status'].includes($n.attr('id')||''))continue;
  if(['strong','b','em','i','code','a','br','span','small'].includes(tag)){pending.push(n);continue;}
  flush();
  if(tag==='pre'){result.push({type:'code',text:$n.text().replace(/^\n/,'').replace(/\n$/,''),language:($n.find('code').attr('class')||'').replace('language-','')||'cpp'});continue;}
  if(tag==='p'||/^h[1-6]$/.test(tag)||tag==='figcaption'){
   const content=inline($,n.children,route);if(content.some(x=>x.text.trim()))result.push({type:tag[0]==='h'?'heading':'paragraph',content,...(/^h/.test(tag)?{level:Number(tag[1]),id:$n.attr('id')}: {})});continue;
  }
  if(tag==='ul'||tag==='ol'){result.push({type:'list',ordered:tag==='ol',items:$n.children('li').toArray().map(li=>blocks($,(li as Element).children,route))});continue;}
  if(tag==='table'){result.push({type:'table',rows:$n.find('tr').toArray().map(tr=>({header:$(tr).find('th').length>0,cells:$(tr).children('td,th').toArray().map(c=>inline($,(c as Element).children,route))}))});continue;}
  if(tag==='details'){result.push({type:'details',title:$n.children('summary').text(),blocks:blocks($,n.children.filter(c=>!(c.type==='tag'&&c.name==='summary')),route)});continue;}
  if(tag==='img'){const src=safeLink($n.attr('src')||'',route);if(src)result.push({type:'image',src,alt:$n.attr('alt')||'실험 결과'});continue;}
  if(/(^|\s)diagram(\s|$)/.test(cls)&&$n.find('img,pre,table').length===0){result.push({type:'diagram',items:$n.children().toArray().map(x=>text($(x).text())).filter(Boolean)});continue;}
  const nested=blocks($,n.children,route);
  if(nested.length)result.push({type:/(?:^|\s)(grid|step-row|asset-gallery|preview-grid)/.test(cls)?'grid':/(callout|flag|sheet)/.test(cls)?'callout':'group',blocks:nested});
 }
 flush();return result;
}
const episodeSchema=z.object({path:z.string().regex(/^ep-[a-z0-9-]+\/$/),title:z.string(),number:z.union([z.string(),z.number()]),summary:z.string().optional(),topic_en:z.string().optional(),category:z.string().optional(),tags:z.array(z.string().trim().min(1).max(40)).min(1).max(8).optional(),date:z.string().optional(),study_date:z.string().optional(),published_date:z.string().optional(),video_id:z.string().regex(/^[A-Za-z0-9_-]{11}$/).optional(),video_url:z.string().url().optional()}).passthrough();
const metaSchema=z.object({id:z.number().int().positive(),date:z.string().regex(/^\d{4}-\d{2}-\d{2}$/),title:z.string(),file:z.string().regex(/^data\/(english|japanese)\/\d+\.json$/),sections:z.array(z.string()).optional()});
const newsDaySchema=z.object({date:z.string().regex(/^\d{4}-\d{2}-\d{2}$/),items:z.array(z.object({title:z.string(),category:z.string().optional(),summary:z.string().optional(),easy_explainer:z.string().optional(),details:z.string().optional(),short_summary:z.union([z.string(),z.array(z.string())]).optional(),technical_context:z.string().optional(),developer_view:z.string().optional(),why_it_matters:z.string().optional(),source:z.string().optional(),url:z.string().optional(),published:z.string().optional(),depth:z.enum(['deep','brief']).optional(),tags:z.array(z.string()).optional()}).passthrough()),original_markdown:z.string().optional()});

function markdownSections(md:string){const re=/^##\s+(\d+)\.\s*(.+)$/gm,m=[...md.matchAll(re)];return m.map((x,i)=>md.slice(x.index!+x[0].length,i+1<m.length?m[i+1].index:md.length));}
function flatten(bs:Block[]):string{ return bs.map(b=>b.type==='paragraph'||b.type==='heading'?b.content.map(x=>x.text).join(''):b.type==='code'?b.text:b.type==='details'?b.title+' '+flatten(b.blocks):'blocks'in b?flatten(b.blocks):b.type==='list'?b.items.map(flatten).join(' '):b.type==='diagram'?b.items.join(' '):'').join(' '); }
function paragraphs(bs:Block[]):string[]{return bs.flatMap(b=>b.type==='paragraph'?[b.content.map(x=>x.text).join('')]:('blocks'in b&&b.type!=='details')?paragraphs(b.blocks):[]).map(text).filter(Boolean);}
export function compileNews(raw:unknown):Article[]{
 const day=newsDaySchema.parse(raw),sections=markdownSections(day.original_markdown||'');
 return day.items.map((x,i)=>{
  const body=compileBlocks(marked.parse(sections[i]||'',{async:false}) as string),p=paragraphs(body);
  const links:{label:string;url:string}[]=[];
  if(x.url&&/^https?:\/\//.test(x.url))links.push({label:x.source||'원문',url:x.url});
  const $=load(marked.parse(sections[i]||'',{async:false}) as string);
  $('a[href]').each((_,n)=>{const url=$(n).attr('href')||'';if(/^https?:\/\//.test(url)&&!links.some(l=>l.url===url))links.push({url,label:$(n).text()||'참고 자료'});});
  const easy=x.easy_explainer||x.details||p[0]||(x.summary&&!x.summary.includes('원문 보기')?x.summary:'상세 설명은 보관 원문에서 확인해 주세요.');
  const summary=(Array.isArray(x.short_summary)?x.short_summary.join(' '):x.short_summary)||(x.summary&&!x.summary.includes('원문 보기')?x.summary:easy.split(/(?<=[.!?。])\s+/).slice(0,2).join(' '));
  return articleSchema.parse({id:day.date+':'+i,date:day.date,title:x.title,category:x.category||'game-dev',source:x.source||links[0]?.label||'출처 미기록',published:x.published||'',depth:x.depth||'deep',tags:x.tags||[],summary,easy,short:summary.split(/(?<=[.!?。])\s+/).filter(Boolean),technical:x.technical_context||'',developer:x.developer_view||'',importance:x.why_it_matters||'',links,blocks:body});
 });
}
async function compileDocument(source:string,route:string,kind:Document['kind'],meta:{title:string;subtitle?:string;date?:string;number?:string;category?:string;tags?:string[];topic?:string;videoUrl?:string;videoId?:string}):Promise<Document>{
 // Authors may opt into a structured content.json document. No HTML lookup at runtime.
 const structured=join(root,dirname(source),'content.json');
 if(await exists(structured)){
  const bytes=await readFile(structured,'utf8'),d=JSON.parse(bytes);
  if(d.route && d.route!==route)throw Error('Document route mismatch: '+source);
  return documentSchema.parse({...d,id:route.replace(/^\//,'').replace(/\/$/,'').replaceAll('/','--'),route,kind,
   title:d.title||meta.title,subtitle:d.subtitle||meta.subtitle||'',date:meta.date||d.date||'',number:meta.number||d.number,
   category:meta.category||d.category||'',topic:meta.topic||d.topic||'',tags:meta.tags||d.tags||[],videoUrl:meta.videoUrl||d.videoUrl,videoId:meta.videoId||d.videoId,
   source:relative(root,structured).replaceAll('\\','/'),sourceHash:digest(bytes)});
 }
 const raw=await readFile(join(root,source),'utf8'),isMarkdown=source.endsWith('.md');
 const linkRoute=isMarkdown?'/'+dirname(source)+'/':route;
 const html=isMarkdown?marked.parse(raw.replace(/^---\n[\s\S]*?\n---\n/,''),{async:false}) as string:raw;
 const $=load(html),main=isMarkdown?$('body'):$('main');if(!main.length)throw Error('Missing main: '+source);
 let head=main.children('header').first();let headerTitle=head.find('h1').first().text()||main.children('h1').first().text()||meta.title;
 const subtitle=meta.subtitle||head.find('.lead').text()||'';
 if(isMarkdown){main.children('h1').first().remove();}
 const memo=main.find('textarea').first(),memoKey=(raw.match(/['"](thinking-[^'"]*-draft)['"]/ )||[])[1];
 const sections:Document['sections']=[];
 const add=(element:Element,index:number)=>{
  const n=$(element);if(n.is('header,nav,footer,script,style')||n.hasClass('links')||n.hasClass('jump')||n.hasClass('anchor-nav'))return;
  const id=n.attr('id')||'section-'+index;
  const heading=title(n.children('h2,summary').first().text())||'기록';
  let tab=kind==='thinking'?'more':kind==='asset'?'results':'document';
  if(kind==='thinking'){
   if(['problem','challenge'].includes(id)||sections.length===0||/핵심 개념/.test(heading))tab='problem';
   else if(id==='hints'||/힌트|직접 생각|스스로/.test(heading))tab='hints';
   else if(['solution','tests','visual'].includes(id)||/풀이|정답|검증/.test(heading))tab='solution';
   if(n.find('textarea').length){if(!n.find('details').length)return;tab='solution';n.children('h2,p,.tools').remove();}
  }
  if(kind==='asset'){tab=['results','compare','process','evaluation','files'][sections.length]||'files';}
  const bs=blocks($,element.children,linkRoute);if(bs.length)sections.push({id,title:heading,tab,blocks:bs});
 };
 if(isMarkdown){let group=$('<section>');main.children().toArray().forEach(n=>{if(n.name==='h2'&&group.contents().length){add(group[0] as Element,sections.length);group=$('<section>');}group.append(n);});if(group.contents().length)add(group[0] as Element,sections.length);}
 else main.children().toArray().forEach((n,i)=>add(n,i));
 // Retain rights/scope notices rather than losing them when removing the old header.
 const note=head.find('.note').text();if(note&&sections[0])sections[0].blocks.unshift({type:'callout',blocks:[{type:'paragraph',content:[{text:note}]}]});
 return documentSchema.parse({id:route.replace(/^\//,'').replace(/\/$/,'').replaceAll('/','--'),route,kind,title:headerTitle,subtitle,date:meta.date||'',number:meta.number,category:meta.category||'',topic:meta.topic||'',videoUrl:meta.videoUrl,videoId:meta.videoId,tags:[...new Set(meta.tags||[meta.category].filter(Boolean) as string[])],memoKey,starter:memo.attr('placeholder')||'',sections,source,sourceHash:digest(raw)});
}
async function walk(path:string):Promise<string[]>{const entries=await readdir(path,{withFileTypes:true});const chunks=await Promise.all(entries.map(x=>x.isDirectory()?walk(join(path,x.name)):Promise.resolve([join(path,x.name)])));return chunks.flat();}
async function copyResources(){
 // Validate every declared archive before copying; no wildcard ZIP publication.
 const bundles=await validatedExampleBundles(root);
 for(const [path,bytes] of bundles){await mkdir(dirname(join(out,path)),{recursive:true});await writeFile(join(out,path),bytes);}
 for(const folder of ['daily-briefing/data','language-study/data','thinking-lab/data','opensource-lab/data','game-design/data','ai-asset-lab/experiments','knowledge']){
  for(const file of await walk(join(root,folder))){const rel=relative(root,file);const preservedSourceArchive=rel==='knowledge/shared/cpp-advanced-lab/archive/cpp_advanced_effective_lab_v1.zip';const nativeTeachingSource=rel.startsWith('knowledge/')&&['.rc','.manifest','.hlsl','.fx','.sh'].includes(extname(file));if(!preservedSourceArchive&&!nativeTeachingSource&&!['.json','.png','.webp','.jpg','.jpeg','.svg','.wav','.ogg','.glb','.md','.c','.h','.cpp','.hpp','.py','.txt'].includes(extname(file)))continue;await mkdir(dirname(join(out,rel)),{recursive:true});await cp(file,join(out,rel));}
 }
 // Illustration assets are authored per lesson; JSON files remain catalog/content inputs.
 for(const file of await walk(join(root,'game-design'))){
  const rel=relative(root,file).replaceAll('\\','/');
  if(!/^game-design\/ep-[a-z0-9-]+\/assets\/[a-z0-9-]+\.(svg|png|webp|jpg|jpeg)$/.test(rel))continue;
  await mkdir(dirname(join(out,rel)),{recursive:true});await cp(file,join(out,rel));
 }
}
export async function buildContent(){
 await rm(out,{recursive:true,force:true});await mkdir(out,{recursive:true});await copyResources();
 const languages={english:[] as z.infer<typeof metaSchema>[],japanese:[] as z.infer<typeof metaSchema>[]};
 const rawLanguage=await json('language-study/data/catalog.json');
 for(const lang of ['english','japanese'] as const){languages[lang]=z.array(metaSchema).parse(rawLanguage[lang]);const ids=new Set<number>();
  for(const m of languages[lang]){if(ids.has(m.id))throw Error('Duplicate lesson: '+lang+'/'+m.id);ids.add(m.id);const d=lessonSchema.parse(await json('language-study/'+m.file));if(d.id!==m.id||d.date!==m.date)throw Error('Lesson/catalog mismatch: '+m.file);m.sections=lang==='english'?['conversation','expressions','reading','vocabulary',...(d.reading_check.length?['check']:[])]:['grammar','examples','kana','words'];if(d.review_reuse.length)m.sections.push('review');}
  languages[lang].sort((a,b)=>a.id-b.id);
 }
 const docs:Document[]=[];
 for(const kind of ['thinking','opensource','game-design'] as const){const prefix=kind==='game-design'?'game-design':kind+'-lab';const eps=z.array(episodeSchema).parse((await json(prefix+'/data/catalog.json')).episodes);
  for(const e of eps){if(kind==='thinking'&&(!e.tags?.length||!e.category))throw Error('Thinking episode requires category and 1–8 tags: '+e.path);if(kind==='game-design'&&(!e.video_id||e.video_url!=='https://www.youtube.com/watch?v='+e.video_id||!e.tags?.length||!e.category))throw Error('Game design episode requires verified video and classification: '+e.path);const dir=prefix+'/'+e.path;const source=await exists(join(root,dir+'content.json'))?dir+'content.json':await exists(join(root,dir+'_content.html'))?dir+'_content.html':dir+'index.html';docs.push(await compileDocument(source,'/'+dir,kind,{title:e.title,subtitle:e.summary,date:e.study_date||e.date||e.published_date,number:String(e.number),category:e.category,tags:e.tags,topic:e.topic_en,videoUrl:e.video_url,videoId:e.video_id}));}
 }

 // Hand-curated Notion migration prototype: every document retains source Markdown and executable source files.
 const knowledge=z.array(z.object({path:z.string().regex(/^[a-z0-9-]+$/),title:z.string(),subtitle:z.string(),date:z.string(),category:z.string(),tags:z.array(z.string()).min(1)})).parse(await json('knowledge/data/catalog.json'));
 for(const entry of knowledge){
  const dir='knowledge/'+entry.path+'/';
  docs.push(await compileDocument(dir+'content.json','/'+dir,'knowledge',{title:entry.title,subtitle:entry.subtitle,date:entry.date,category:entry.category,tags:entry.tags,topic:entry.category}));
 }
 const assets:AssetMeta[]=[];
 for(const path of (await walk(join(root,'ai-asset-lab/experiments'))).filter(p=>p.endsWith('/evaluation.json'))){
  const source=relative(root,path),dir=dirname(source)+'/',evaluation=await json(source),route='/'+dir;
  const docSource=await exists(join(root,dir+'_content.html'))?dir+'_content.html':dir+'index.html';docs.push(await compileDocument(docSource,route,'asset',{title:evaluation.experiment_id,date:evaluation.date}));
  const files=[['report.md','report/'],['prompts/prompts-v1.md','prompts/prompts-v1/']];for(const [file,suffix] of files)if(await exists(join(root,dir+file)))docs.push(await compileDocument(dir+file,route+suffix,'report',{title:file,date:evaluation.date}));
  const names:Record<string,string>={P01:'체력 포션',P02:'마나 포션',P03:'해독 포션',P04:'비전 포션'};
  for(const a of evaluation.published_pngs||[])assets.push({id:evaluation.experiment_id+'/'+a.id,name:names[a.id]||a.id,src:dir+a.file,route,experiment:evaluation.experiment_id,date:evaluation.date,status:evaluation.status,size:a.dimensions,model:evaluation.generation_tool});
 }
 docs.sort((a,b)=>a.route.localeCompare(b.route,undefined,{numeric:true}));
 const daily=await json('daily-briefing/data/catalog.json');const rows=z.array(z.object({date:z.string(),file:z.string().regex(/^data\/[\d-]+\.json$/)})).parse(Array.isArray(daily)?daily:daily.entries);
 const articles=(await Promise.all(rows.map(async r=>compileNews(await json('daily-briefing/'+r.file))))).flat().sort((a,b)=>b.date.localeCompare(a.date));
 if(new Set(articles.map(a=>a.id)).size!==articles.length)throw Error('Duplicate article ID');
 const revision=digest(JSON.stringify([docs.map(d=>[d.sourceHash,d.title,d.subtitle,d.date,d.number,d.category,d.topic,d.tags]),languages,articles])).slice(0,12);
 const progress=await json('language-study/data/progress.json');
 const catalog:Catalog={version:2,revision,documents:docs.map(({sections,starter,...d})=>({...d,file:'data/atlas/docs/'+d.id+'.json',sections:sections.map(({blocks,...s})=>s)})),languages,assets,dates:[...new Set(articles.map(a=>a.date))],articleCount:articles.length,review:progress.review?.due||[]};
 for(const doc of docs)await write(join(out,'data/atlas/docs',doc.id+'.json'),doc);
 await write(join(out,'data/atlas/catalog.json'),catalog);await write(join(out,'data/atlas/articles.json'),articles);
 await write(join(out,'data/atlas/routes.json'),['/','/daily-briefing/','/language-study/','/thinking-lab/','/game-design/','/opensource-lab/','/ai-asset-lab/','/knowledge/','/bookmarks/','/archive/',...docs.map(d=>d.route),...(['english','japanese'] as Language[]).flatMap(lang=>languages[lang].map(m=>'/language-study/'+lang+'/'+m.id+'/'))]);
 await write(join(out,'favicon.svg'),'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32"><rect width="32" height="32" rx="8" fill="#101319"/><path d="M16 4l3 9 9 3-9 3-3 9-3-9-9-3 9-3z" fill="#b8a4ff"/></svg>');
 // Old workers at this exact scope update to a network-only retirement worker. Storage is not cleared.
 await write(join(out,'language-study/sw.js'),`self.addEventListener('install',()=>self.skipWaiting());self.addEventListener('activate',e=>e.waitUntil((async()=>{const keys=await caches.keys();await Promise.all(keys.filter(k=>k.startsWith('language-study-')).map(k=>caches.delete(k)));await self.clients.claim();await self.registration.unregister();})()));`);
 await write(join(out,'build-info.json'),{framework:'React',buildTool:'Vite',revision,documents:docs.length,lessons:languages.english.length+languages.japanese.length,articles:articles.length});
 console.log(`Validated ${docs.length} documents, ${languages.english.length+languages.japanese.length} lessons, ${articles.length} articles; revision ${revision}`);
 // Ensure input body remains substantive after conversion (not a UI shell).
 for(const d of docs)if(flatten(d.sections.flatMap(s=>s.blocks)).length<80)throw Error('Empty converted content: '+d.source);
 return catalog;
}
if(process.argv[1]&&resolve(process.argv[1])===fileURLToPath(import.meta.url))await buildContent();
