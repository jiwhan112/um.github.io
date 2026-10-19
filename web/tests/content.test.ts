import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFile,readdir} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {load} from 'cheerio';
import {buildContent,compileBlocks,compileNews,safeLink} from '../scripts/content';
import {parseStored,briefingState,objectBooleans} from '../src/lib/storage';
import {lessonProgress,reviewId} from '../src/lib/language-progress';
import {canSyncKey} from '../src/lib/cloud';
import {filterDocuments,readLibraryFilter,matchesDocument} from '../src/lib/document-library';
import {catalogSchema, documentSchema, type Block} from '../src/lib/types';
const catalog=await buildContent();
test('all source lessons and documents are compiled',()=>{assert.equal(catalog.version,2);assert.ok(catalog.documents.length>=9);assert.ok(catalog.languages.english.length>=9);assert.ok(catalog.languages.japanese.length>=9);assert.ok(catalog.articleCount>=135);});
test('memo storage keys are preserved exactly',()=>{const expected:Record<string,string>={'1.1':'thinking-ctci-1-1-draft','1.2':'thinking-ctci-1-2-draft','1.3':'thinking-ctci-1.3-draft','1.4':'thinking-ctci-1.4-draft'};for(const [number,key]of Object.entries(expected))assert.equal(catalog.documents.find(d=>d.kind==='thinking'&&d.number===number)?.memoKey,key);});
function codeBlocks(bs:Block[]):string[]{return bs.flatMap(b=>b.type==='code'?[b.text]:'blocks'in b?codeBlocks(b.blocks):b.type==='list'?b.items.flatMap(codeBlocks):[]);}
test('C++ examples are byte-for-byte unchanged after entity decoding',async()=>{
 for(const meta of catalog.documents.filter(d=>d.kind==='thinking')){const source=await readFile('../'+meta.source,'utf8'),$=load(source),document=JSON.parse(await readFile('public/'+meta.file,'utf8'));const expected=meta.source.endsWith('.json')?codeBlocks(JSON.parse(source).sections.flatMap((s:{blocks:Block[]})=>s.blocks)):$('main pre').toArray().map(n=>$(n).text().replace(/^\n/,'').replace(/\n$/,''));const actual=codeBlocks(document.sections.flatMap((s:{blocks:Block[]})=>s.blocks));assert.deepEqual(actual,expected,meta.route);}
});
test('data adapter never includes scripts, styles or event handlers',()=>{const b=compileBlocks('<script>alert(1)</script><p onclick="evil()">safe <a href="javascript:alert(1)">link</a></p><style>*{display:none}</style>');const s=JSON.stringify(b);assert.ok(s.includes('safe'));assert.ok(!s.includes('evil'));assert.ok(!s.includes('alert'));assert.ok(!s.includes('display:none'));assert.equal(safeLink('javascript:alert(1)','/'),'');});
test('archived markdown body and stable article IDs remain available',()=>{const rows=compileNews({date:'2026-01-01',items:[{title:'test'}],original_markdown:'## 1. test\n\n본문입니다.\n\n```cpp\nint value = 1;\n```'});assert.equal(rows[0].id,'2026-01-01:0');assert.ok(rows[0].blocks.some(b=>b.type==='code'));});
test('corrupt and old storage is handled without deleting valid state',()=>{assert.deepEqual(parseStored('broken',{}),{});assert.deepEqual(briefingState({saved:['2026-10-12:0',3],read:['2026-10-11:1']}),{saved:['2026-10-12:0'],read:['2026-10-11:1']});assert.deepEqual(objectBooleans({'x':true,'y':'wrong'}),{x:true});});
test('production React code has no legacy DOM reparenting or raw HTML renderer',async()=>{
 async function files(dir:string):Promise<string[]>{return (await Promise.all((await readdir(dir,{withFileTypes:true})).map(x=>x.isDirectory()?files(dir+'/'+x.name):[dir+'/'+x.name]))).flat();}
 for(const f of await files('src'))if(/\.tsx?$/.test(f)){const s=await readFile(f,'utf8');assert.ok(!/dangerouslySetInnerHTML|\.innerHTML\s*=|\.appendChild\(|\.replaceChildren\(/.test(s),f);}
});

test('thinking taxonomy uses authored catalog tags rather than dates or UI labels', async()=>{
 const raw=JSON.parse(await readFile('../thinking-lab/data/catalog.json','utf8'));
 for(const e of raw.episodes){const d=catalog.documents.find(d=>d.kind==='thinking'&&d.number===e.number)!;assert.ok(e.category);assert.deepEqual(d.tags,e.tags);assert.equal(d.category,e.category);assert.equal(d.topic,e.topic_en);assert.ok(!d.tags.some(t=>/\d{4}-\d{2}-\d{2}|학습일|게시일|힌트와/.test(t)));}
});
test('library matches Korean, English, number and combined tags without mutating source',()=>{
 const docs=catalog.documents.filter(d=>d.kind==='thinking'&&['1.1','1.2','1.3','1.4'].includes(d.number||'')),original=docs.map(d=>d.id),base={query:'',category:'',tags:[] as string[],sort:'number' as const};
 assert.equal(filterDocuments(docs,{...base,query:'PALINDROME'}).length,1);
 assert.equal(filterDocuments(docs,{...base,query:'  비트   연산  '})[0].number,'1.4');
 assert.equal(filterDocuments(docs,{...base,query:'1.3'})[0].number,'1.3');
 assert.deepEqual(filterDocuments(docs,{...base,tags:['문자열','문자 빈도']}).map(d=>d.number),['1.2','1.4']);
 assert.equal(filterDocuments(docs,{...base,tags:['존재하지않음']}).length,0);
 assert.equal(filterDocuments(docs,{...base,category:'존재하지않음'}).length,0);
 assert.equal(filterDocuments(docs,{...base,sort:'newest'})[0].number,'1.4');
 assert.deepEqual(docs.map(d=>d.id),original);
 assert.equal(matchesDocument(docs[0],{...base,query:'this_word_is_only_in_a_hidden_solution'}),false);
});
test('URL filter decoding tolerates duplicate tags and invalid sort',()=>{
 const f=readLibraryFilter(new URLSearchParams('q=%ED%9A%8C%EB%AC%B8&tag=abc&tag=abc&sort=invalid'));
 assert.equal(f.query,'회문');assert.deepEqual(f.tags,['abc']);assert.equal(f.sort,'number');
});
test('new briefings accept either a summary string or an array of short sentences',()=>{
 const items=compileNews({date:'2026-10-12',items:[{title:'schema',short_summary:['첫째.','둘째.']}]});assert.deepEqual(items[0].short,['첫째.','둘째.']);
});

test('lesson completion derives from actual compiled sections and keeps historical check keys',()=>{
 const meta=catalog.languages.japanese.at(-1)!;
 assert.ok(meta.sections?.length);
 const state=Object.fromEntries(meta.sections!.map(section=>[`japanese:${meta.id}:lang-${section}`,true]));
 assert.equal(lessonProgress(state,'japanese',meta).done,true);
 state[`japanese:${meta.id}:lang-${meta.sections![0]}`]=false;
 assert.equal(lessonProgress(state,'japanese',meta).done,false);
 assert.equal(lessonProgress({},'japanese',meta).count,0);
});
test('sync coverage is explicit and does not send auth, config or unrelated browser keys',()=>{
 for(const k of ['jiwhan-briefing-v1','atlas-reading-v1','atlas-language-sections-v1','atlas-review-v1','atlas-game-design-bookmarks-v1','thinking-ctci-1.4-draft','atlas-code-thinking--example'])assert.equal(canSyncKey(k),true,k);
 for(const k of ['atlas-cloud-session-v1','atlas-cloud-settings-v1','atlas-cloud-pending-v1','other-app-password','language-study-sw-cache'])assert.equal(canSyncKey(k),false,k);
 assert.equal(reviewId({language:'english',from_lesson:9,term:'hello',due_date:'2026-10-12'}),'english:9:hello:2026-10-12');
});


test('game design episodes are compiled with stable video IDs and source links',async()=>{
 const source=JSON.parse(await readFile('../game-design/data/catalog.json','utf8'));
 const docs=catalog.documents.filter(d=>d.kind==='game-design');
 assert.equal(docs.length,source.episodes.length);
 for(const e of source.episodes){const d=docs.find(d=>d.videoId===e.video_id);assert.ok(d,e.video_id);assert.equal(d.route,'/game-design/'+e.path);assert.equal(d.videoUrl,e.video_url);assert.ok(d.sections.some(s=>s.tab==='summary'));const output=JSON.parse(await readFile('public/'+d.file,'utf8'));assert.ok(output.sections.some((s:{tab:string})=>s.tab==='implementation'));}
 const routes=JSON.parse(await readFile('public/data/atlas/routes.json','utf8'));
 assert.ok(routes.includes('/game-design/'));assert.ok(routes.includes('/game-design/'+source.episodes[0].path));
});

test('game design illustrations are copied into the published resource tree',async()=>{
 const first=catalog.documents.find(d=>d.kind==='game-design'&&d.number==='01')!;
 const document=JSON.parse(await readFile('public/'+first.file,'utf8'));
 function images(blocks:Block[]):string[]{return blocks.flatMap(b=>b.type==='image'?[b.src]:b.type==='list'?b.items.flatMap(images):'blocks'in b?images(b.blocks):[]);}
 const pics=document.sections.flatMap((section:{blocks:Block[]})=>images(section.blocks));
 assert.ok(pics.length>=2,'first lesson should include two concept diagrams');
 for(const path of pics){assert.match(path,/^\/game-design\/ep-[a-z0-9-]+\/assets\/[a-z0-9-]+\.svg$/);const bytes=await readFile('public/'+path.slice(1),'utf8');assert.ok(bytes.includes('<svg'),path);}
});

test('migrated knowledge corpus preserves routes, typed content and Notion originals',async()=>{
 const checked=catalogSchema.parse(catalog);
 const docs=checked.documents.filter(d=>d.kind==='knowledge');
 const sourceCatalog=JSON.parse(await readFile('../knowledge/data/catalog.json','utf8')) as {path:string;notion_url:string;category:string}[];
 assert.equal(docs.length,45);
 assert.equal(sourceCatalog.length,45);
 for(const old of ['/knowledge/dx11-frustum/','/knowledge/algorithm-lis/','/knowledge/python-html/'])
  assert.ok(docs.some(d=>d.route===old),'preserve old prototype '+old);
 const imports=sourceCatalog.filter(x=>/^winapi-0[1-4]a?$|^game-math-0[1-5]$|^dx9-pipeline-0[1-5]$/.test(x.path));
 assert.equal(imports.length,15);
 for(const entry of sourceCatalog){
  const meta=docs.find(d=>d.route==='/knowledge/'+entry.path+'/');
  assert.ok(meta,'catalog misses '+entry.path);
  const full=documentSchema.parse(JSON.parse(await readFile('public/'+meta.file,'utf8')));
  assert.equal(full.id,meta.id);
  assert.equal(full.route,meta.route);
  assert.equal(full.category,entry.category);
  assert.ok(full.sections.some(section=>section.blocks.some(b=>b.type==='code')),'code should be preserved for '+entry.path);
  assert.ok(full.sections.some(section=>section.blocks.some(b=>b.type==='details')),'review answers should be preserved for '+entry.path);
 }
 for(const entry of imports){
  const original=await readFile('../knowledge/'+entry.path+'/source-notion.md','utf8');
  const doc=docs.find(d=>d.route==='/knowledge/'+entry.path+'/')!;
  const full=documentSchema.parse(JSON.parse(await readFile('public/'+doc.file,'utf8')));
  assert.ok(original.length>4000,'full original was truncated '+entry.path);
  assert.ok(original.includes('```mermaid'),'native Mermaid source absent '+entry.path);
  assert.ok(original.includes('<details>'),'folded answers absent '+entry.path);
  if(entry.path.startsWith('dx9-pipeline-'))assert.ok(full.sections.some(section=>section.blocks.some(b=>b.type==='table')),'DX9 source comparison table lost '+entry.path);
  assert.ok(full.sections.some(s=>s.blocks.some(b=>b.type==='code'&&b.language==='mermaid')),'diagram source lost '+entry.path);
  assert.ok(full.sections.some(s=>s.id==='full-source-refs'&&s.blocks.some(b=>b.type==='paragraph'&&b.content.some(n=>n.kind==='link'&&n.href?.startsWith('/knowledge/')))),'missing internal full source reference '+entry.path);
 }
});

test('WinAPI04A native CPU sources preserve original Notion byte counts and SHA-256',async()=>{
 const cpp=await readFile('../knowledge/winapi-04a/main.cpp');
 const cmake=await readFile('../knowledge/winapi-04a/CMakeLists.txt');
 assert.equal(cpp.length,4701);
 assert.equal(cmake.length,543);
 assert.equal(createHash('sha256').update(cpp).digest('hex'),'5e711d0e1b1685cca876e94c3416dde0e4271c3bb4e9e959fc292f2d7dde15d2');
 const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--winapi-04a.json','utf8')));
 const embedded=doc.sections.find(s=>s.id==='github-source')!;
 assert.ok(embedded.blocks.some(b=>b.type==='code'&&b.language==='cpp'&&b.text.includes('int main()')));
 assert.ok(embedded.blocks.some(b=>b.type==='code'&&b.language==='cmake'&&b.text.includes('add_test(')));
});

test('shared WinAPI/math and DX9 source files match original archived Markdown byte-for-byte',async()=>{
 const packages=[
  {folder:'game-math-winapi-lab',original:'game_math_winapi_sources_v1.md',fence:'`',count:18},
  {folder:'dx9-pipeline-lab',original:'dx9_pipeline_sources_v1.md',fence:'~',count:12},
 ];
 for(const pack of packages){
  const original=await readFile('../knowledge/shared/'+pack.original,'utf8');
  const delimiter=pack.fence.repeat(4);
  const rx=new RegExp('^## ([^\\n]+)\\n\\n'+delimiter+'[^\\n]*\\n([\\s\\S]*?)\\n'+delimiter+'\\s*$','gm');
  const parts=[...original.matchAll(rx)];
  assert.equal(parts.length,pack.count,'all source parts should exist '+pack.folder);
  for(const match of parts){
   const expected=Buffer.from(match[2]+(pack.folder==='dx9-pipeline-lab'?'\n':''),'utf8');
   const actual=await readFile('../knowledge/shared/'+pack.folder+'/'+match[1]);
   assert.deepEqual(actual,expected,'byte-for-byte restore '+pack.folder+'/'+match[1]);
  }
 }
 const hashes=JSON.parse(await readFile('../knowledge/shared/game-math-winapi-lab/source_hashes.json','utf8')) as Record<string,string>;
 let matched=0;
 for(const [name,sha] of Object.entries(hashes)){
  if(name.startsWith('logs/'))continue;
  const code=await readFile('../knowledge/shared/game-math-winapi-lab/'+name);
  assert.equal(createHash('sha256').update(code).digest('hex'),sha,name);
  matched++;
 }
 assert.equal(matched,17,'17 original source/metadata hashes are preserved');
});

test('full shared source articles are published and reachable with CMake and complete code',async()=>{
 for(const [slug,expected] of [['winapi-math-lab',19],['dx9-pipeline-lab',13]] as const){
  const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+slug+'.json','utf8')));
  assert.equal(doc.sections.length,expected);
  assert.ok(doc.sections.some(s=>s.title==='CMakeLists.txt'&&s.blocks.some(b=>b.type==='code'&&b.text.includes('project('))));
  assert.ok(doc.sections.some(s=>s.title.startsWith('src/')&&s.blocks.some(b=>b.type==='code'&&b.text.length>400)));
  assert.ok(doc.sections[0].blocks.some(b=>b.type==='table'),'verify environment comparison present');
 }
});

test('DX9 implementation original lesson sections and independently archived full source pages are present',async()=>{
 const map=JSON.parse(await readFile('../knowledge/data/catalog.json','utf8')) as {path:string;notion_url:string}[];
 const implementations=Array.from({length:12},(_,i)=>'dx9-impl-'+String(i+1).padStart(2,'0'));
 const suffixes=['01a','02a','03a','04a','05a','06a','07b','11a'];
 assert.equal(map.filter(x=>/^dx9-impl-/.test(x.path)).length,20);
 for(const slug of [...implementations,...suffixes.map(s=>'dx9-impl-source-'+s)]){
  const entry=map.find(x=>x.path===slug)!;
  assert.ok(entry,'catalog missing '+slug);
  const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+slug+'.json','utf8')));
  assert.equal(doc.route,'/knowledge/'+slug+'/');
  const original=await readFile('../knowledge/'+slug+'/source-notion.md','utf8');
  assert.ok(original.length>1800,slug+' source content truncated');
  assert.ok(original.includes('## '),slug+' original headings absent');
  assert.ok(doc.sections.length>=1,slug+' empty rendered document');
  if(slug.endsWith('-03')||slug.endsWith('-04')){
   assert.equal(entry.notion_url,'https://app.notion.com/p/bdcccf3ce6d34930827bf0a519b4fa78');
  }
 }
 const linked=new Set(['01','02','03','04','05','06','07','11']);
 for(const chapter of linked){
  const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--dx9-impl-'+chapter+'.json','utf8')));
  assert.ok(doc.sections.some(s=>s.id==='source-files'&&s.blocks.some(b=>b.type==='paragraph'&&b.content.some(n=>n.kind==='link'&&n.href?.startsWith('/knowledge/dx9-impl-source-')))),'source chapter link missing '+chapter);
 }
 for(const [s,files] of Object.entries({
  '01a':['com_owner.h','com_owner_tests.cpp','d3d9_window.cpp','CMakeLists.txt'],
  '02a':['include/engine_api.h','src/engine_api.cpp','src/client.cpp','tests/engine_contract_tests.cpp','CMakeLists.txt'],
  '03a':['resource_model.cpp','CMakeLists.txt'],
  '04a':['renderer_components.cpp','notion_example.cpp','CMakeLists.txt'],
  '05a':['scene_loop.hpp','tests.cpp','CMakeLists.txt'],
  '06a':['mesh_texture_contract.cpp','CMakeLists.txt'],
  '07b':['main.cpp','CMakeLists.txt'],
  '11a':['main.cpp','CMakeLists.txt']
 })){
  for(const f of files)assert.ok((await readFile('../knowledge/shared/dx9-impl-'+s+'/'+f,'utf8')).length>70,f+' must contain full source');
  const d=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--dx9-impl-source-'+s+'.json','utf8')));
  assert.ok(d.sections.some(section=>section.blocks.some(b=>b.type==='code'&&b.language==='cpp'&&b.text.length>200)),'missing full C++ source in article '+s);
  assert.ok(d.sections.some(section=>section.blocks.some(b=>b.type==='code'&&b.text.includes('project('))),'missing CMake code in article '+s);
 }
});

test('DX11 chapter 02–06 original text and diagram/answers are preserved without claiming child-source migration',async()=>{
 for(const num of ['02','03','04','05','06']){
  const slug='dx11-'+num;
  const original=await readFile('../knowledge/'+slug+'/source-notion.md','utf8');
  assert.ok(original.length>3000,slug+' original is too short');
  assert.ok(original.includes('```mermaid'),slug+' original diagram missing');
  const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+slug+'.json','utf8')));
  assert.equal(doc.route,'/knowledge/'+slug+'/');
  assert.ok(doc.sections.some(s=>s.blocks.some(b=>b.type==='code'&&b.language==='mermaid')),slug+' Mermaid code missing');
  assert.ok(doc.sections.some(s=>s.id==='source-followup'),slug+' original child links missing');
 }
});
