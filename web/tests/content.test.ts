import thinkingMigration from './fixtures/thinking-pair-migration.json' with {type:'json'};
import {validateThinkingPairs} from '../scripts/validate-thinking-pairs.mjs';
import legacyVisualRollout from './fixtures/legacy-visual-rollout.json' with {type:'json'};
import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFile,readdir} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {spawnSync} from 'node:child_process';
import {load} from 'cheerio';
import {buildContent,compileBlocks,compileNews,safeLink} from '../scripts/content';
import {parseStored,briefingState,objectBooleans} from '../src/lib/storage';
import {lessonProgress,reviewId} from '../src/lib/language-progress';
import {canSyncKey} from '../src/lib/cloud';
import {filterDocuments,readLibraryFilter,matchesDocument} from '../src/lib/document-library';
import {compareKnowledgeDocuments} from '../src/lib/knowledge-order';
import {catalogSchema, documentSchema, type Block} from '../src/lib/types';

type LegacyChapter=(typeof legacyVisualRollout.chapters)[number]&{sourceArchives?:{path:string;sha256:string}[];sourceViews?:{sourceLibrary:string;sourceTab:string;sourceSection:string;displayedCodeFiles:string[]}[]};
const legacyChapters:LegacyChapter[]=legacyVisualRollout.chapters;
function legacyNested(blocks:Block[]):Block[]{return blocks.flatMap(b=>[b,...('blocks'in b?legacyNested(b.blocks):b.type==='list'?b.items.flatMap(legacyNested):[])]);}
function legacyCanonical(value:unknown):unknown{if(Array.isArray(value))return value.map(legacyCanonical);if(value&&typeof value==='object')return Object.fromEntries(Object.entries(value).sort(([a],[b])=>a.localeCompare(b,'en')).map(([key,item])=>[key,legacyCanonical(item)]));return value;}
const legacyHash=(value:unknown)=>createHash('sha256').update(JSON.stringify(legacyCanonical(value))).digest('hex');
function assertLegacyPayload(blocks:Block[],expected:LegacyChapter){
 const available=legacyNested(blocks).map(legacyHash);
 for(const fingerprint of [...expected.originalCodeSha256,...expected.originalCountBlocksSha256]){
  const index=available.indexOf(fingerprint);assert.notEqual(index,-1,expected.slug+' original block '+fingerprint);available.splice(index,1);
 }
}

const catalog=await buildContent();
test('all source lessons and documents are compiled',()=>{assert.equal(catalog.version,2);assert.ok(catalog.documents.length>=9);assert.ok(catalog.languages.english.length>=9);assert.ok(catalog.languages.japanese.length>=9);assert.ok(catalog.articleCount>=135);});
test('memo storage keys are preserved exactly',()=>{const expected:Record<string,string>={'1.1.1':'thinking-ctci-1-1-draft','1.2.1':'thinking-ctci-1-2-draft','1.3.1':'thinking-ctci-1.3-draft','1.4.1':'thinking-ctci-1.4-draft'};for(const [number,key]of Object.entries(expected))assert.equal(catalog.documents.find(d=>d.kind==='thinking'&&d.number===number)?.memoKey,key);});
test('split 1.1 preserves old route and gives advanced lesson independent state',()=>{
 const basic=catalog.documents.find(d=>d.kind==='thinking'&&d.number==='1.1.1')!;
 const advanced=catalog.documents.find(d=>d.kind==='thinking'&&d.number==='1.1.2')!;
 assert.equal(basic.route,'/thinking-lab/ep-01-01-is-unique/');
 assert.equal(advanced.route,'/thinking-lab/ep-01-01-02-is-unique-advanced/');
 assert.equal(advanced.memoKey,'thinking-ctci-1-1-2-draft');
 assert.notEqual(basic.id,advanced.id);assert.notEqual(basic.memoKey,advanced.memoKey);
 assert.equal(basic.date,'2026-10-09');assert.equal(advanced.date,basic.date);
 const sorted=filterDocuments([advanced,basic],{query:'',category:'',tags:[],sort:'number'});
 assert.deepEqual(sorted.map(d=>d.number),['1.1.1','1.1.2']);
});
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
 const docs=catalog.documents.filter(d=>d.kind==='thinking'&&['1.1.1','1.1.2','1.2.1','1.3.1','1.4.1'].includes(d.number||'')),original=docs.map(d=>d.id),base={query:'',category:'',tags:[] as string[],sort:'number' as const};
 assert.equal(filterDocuments(docs,{...base,query:'PALINDROME'}).length,1);
 assert.equal(filterDocuments(docs,{...base,query:'  비트   연산  '})[0].number,'1.4.1');
 assert.equal(filterDocuments(docs,{...base,query:'1.3.1'})[0].number,'1.3.1');
 assert.deepEqual(filterDocuments(docs,{...base,tags:['문자열','문자 빈도']}).map(d=>d.number),['1.2.1','1.4.1']);
 assert.equal(filterDocuments(docs,{...base,tags:['존재하지않음']}).length,0);
 assert.equal(filterDocuments(docs,{...base,category:'존재하지않음'}).length,0);
 assert.equal(filterDocuments(docs,{...base,sort:'newest'})[0].number,'1.4.1');
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
 // The source catalog grows with scheduled lessons; compare identities, not a stale total.
 assert.ok(sourceCatalog.length>=89,'preserve the established knowledge corpus');
 const sourceRoutes=sourceCatalog.map(entry=>'/knowledge/'+entry.path+'/');
 assert.equal(new Set(sourceRoutes).size,sourceRoutes.length,'source routes must be unique');
 assert.deepEqual(docs.map(doc=>doc.route).sort(),[...sourceRoutes].sort(),'compile every authored route exactly once');
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

test('C01 visual textbook preserves original source, 17 historical logs and Korean CPU source links',async()=>{
 const full=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--c-language-01.json','utf8')));
 const tabs=new Set(full.sections.map(s=>s.tab));
 assert.deepEqual([...tabs].sort(),['document','history','practice','sources']);
 assert.equal(full.sections.filter(s=>s.tab==='document').length,6);
 const blocks=full.sections.flatMap(s=>s.blocks);
 assert.ok(blocks.filter(b=>b.type==='image').length>=2);
 assert.equal(blocks.filter(b=>b.type==='code'&&b.language==='mermaid').length,2);
 assert.ok(blocks.filter(b=>b.type==='details').length>=12);
 for(const code of ['original/c11/main.c','original/c11/CMakeLists.txt','original/c01b/main.cpp','original/c01b/CMakeLists.txt']){
  const expected=JSON.parse(await readFile('../knowledge/shared/c01-lab/source-integrity.json','utf8')).source_files.find((f:{path:string})=>f.path.endsWith(code));
  assert.ok(expected,'missing C01 source inventory '+code);
  const bytes=await readFile('../'+expected.path);
  assert.equal(bytes.length,expected.bytes,code);
  assert.equal(createHash('sha256').update(bytes).digest('hex'),expected.sha256,code);
 }
 for(const folder of ['knowledge/c-language-01','knowledge/c-language-01/c01b']){
  const provenance=JSON.parse(await readFile('../'+folder+'/source-notion-capture.json','utf8'));
  const orig=await readFile('../'+folder+'/source-notion.md');
  assert.equal(orig.length,provenance.source_bytes);
  assert.equal(createHash('sha256').update(orig).digest('hex'),provenance.sha256);
 }
 const base='../knowledge/shared/c01-lab/historical-notion/';
 let historical=0;
 for(const id of ['initial','retest']){
  const index=JSON.parse(await readFile(base+id+'-index.json','utf8'));
  for(const entry of index.entries){
   const content=await readFile('../'+entry.path);
   assert.equal(content.length,entry.bytes,entry.path);
   historical++;
  }
 }
 assert.equal(historical,17);
 const originalC=await readFile('../knowledge/shared/c01-lab/original/c11/main.c','utf8');
 const originalCpp=await readFile('../knowledge/shared/c01-lab/original/c01b/main.cpp','utf8');
 assert.ok(originalC.includes('PRId64')&&originalCpp.includes('bool checkedAdd'));
 assert.ok((await readFile('../knowledge/shared/c01-lab/ko/c11/main.c','utf8')).includes('대입'));
 assert.ok((await readFile('../knowledge/shared/c01-lab/ko/c01b/main.cpp','utf8')).includes('정수'));
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
  assert.equal(doc.sections.length,expected+1,'original sections plus exactly one download section');
  assert.equal(doc.sections.filter(section=>section.id==='example-downloads'&&section.tab==='practice').length,1);
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

test('DX11 01–08 original chapter pages and five full native source articles are preserved',async()=>{
 const chapters=['01','02','03','04','05','06','07','08'];
 for(const id of chapters){
  const page=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--dx11-'+id+'.json','utf8')));
  const original=await readFile('../knowledge/dx11-'+id+'/source-notion.md','utf8');
  assert.ok(original.length>3200,'original text missing '+id);
  assert.ok(original.includes('```mermaid'),'diagram missing '+id);
  assert.ok(page.sections.some(section=>section.blocks.some(b=>b.type==='code'&&b.language==='mermaid')),'typed diagram missing '+id);
  assert.ok(page.sections.some(section=>section.blocks.some(b=>b.type==='table')),'table missing '+id);
  assert.ok(page.sections.some(s=>s.id==='github-full-source'||s.id==='full-source'),'GitHub native source link missing '+id);
 }
 for(const slug of ['dx11-sources-01','dx11-sources-02','dx11-sources-03','dx11-sources-04','dx11-sources-05-08']){
  const d=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+slug+'.json','utf8')));
  assert.ok(d.sections.some(s=>s.blocks.some(b=>b.type==='code'&&b.language==='cpp'&&b.text.length>500)),'full C++ source not rendered '+slug);
  assert.ok(d.sections.some(s=>s.blocks.some(b=>b.type==='code'&&b.language==='cmake'&&b.text.includes('project('))),'CMake root missing '+slug);
  assert.ok(d.sections[0].blocks.some(b=>b.type==='table'),'verification boundary table absent '+slug);
 }
 const mappings=[
  ['dx11-01/native-notion-source-01.md','contract_model.cpp','dx11-01/contract_model.cpp'],
  ['dx11-01/native-notion-source-01.md','d3d11_init_window.cpp','dx11-01/d3d11_init_window.cpp'],
  ['dx11-02/native-notion-source-02.md','02_resources/main.cpp','dx11-02/main.cpp'],
  ['dx11-03/native-notion-source-03.md','03_pipeline/main.cpp','dx11-03/main.cpp'],
  ['dx11-04/native-notion-source-04.md','04_texture/main.cpp','dx11-04/main.cpp'],
  ['dx11-05-08/native-notion-source-05.md','main.cpp','dx11-05-08/05_textured_rect/main.cpp'],
  ['dx11-05-08/native-notion-source-06.md','main.cpp','dx11-05-08/06_transform_camera/main.cpp'],
  ['dx11-05-08/native-notion-source-07.md','main.cpp','dx11-05-08/07_skybox_lighting/main.cpp'],
  ['dx11-05-08/native-notion-source-08.md','main.cpp','dx11-05-08/08_terrain/main.cpp']
 ];
 for(const [notion,title,path] of mappings){
  const original=await readFile('../knowledge/shared/'+notion,'utf8');
  const escaped=title.replace(/[.*+?^${}()|[\]\\]/g,'\\$&');
  const match=original.match(new RegExp('^## '+escaped+'\\n```[^\\n]*\\n([\\s\\S]*?)\\n```','m'));
  assert.ok(match,'source code fence absent: '+notion+' '+title);
  const file=await readFile('../knowledge/shared/'+path,'utf8');
  assert.equal(file,match[1],'native Notion code bytes differ: '+path);
 }
 assert.ok((await readFile('../knowledge/shared/dx11-05-08/CMakeLists.txt','utf8')).includes('add_subdirectory(08_terrain)'));
 const native01=await readFile('../knowledge/dx11-01/source-notion.md','utf8');
 assert.ok(native01.includes('notion-file-block://'),'source attachment reference was lost');
});

test('DX11 09-12 full theory and source references survive site generation',async()=>{
 const topics=['09','10','11','12'];
 for(const n of topics){
  const slug='dx11-'+n;
  const original=await readFile('../knowledge/'+slug+'/source-notion.md','utf8');
  const output=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+slug+'.json','utf8')));
  assert.ok(original.length>3500,'truncated source '+slug);
  assert.ok(original.includes('```mermaid'),'Mermaid source absent '+slug);
  assert.ok(original.includes('<table'),'original comparison table missing '+slug);
  assert.ok(original.includes('<details>'),'original review Q&A missing '+slug);
  assert.ok(output.sections.some(s=>s.blocks.some(b=>b.type==='code'&&b.language==='mermaid')),'renderable code diagram missing '+slug);
  assert.ok(output.sections.some(s=>s.blocks.some(b=>b.type==='table')),'typed table missing '+slug);
  assert.ok(output.sections.some(s=>s.blocks.some(b=>b.type==='details')),'typed folded answer missing '+slug);
  assert.ok(output.sections.some(s=>s.id==='full-cpu-source'&&s.blocks.some(b=>b.type==='paragraph'&&b.content.some(c=>c.kind==='link'&&c.href?.startsWith('/knowledge/dx11-sources-')))),'full source link missing '+slug);
 }
 for(const [source,files] of Object.entries({
  'dx11-sources-09':['main.cpp','CMakeLists.txt'],
  'dx11-sources-10':['main.cpp','CMakeLists.txt'],
  'dx11-sources-11-12':['11_collision/main.cpp','12_navmesh/main.cpp','CMakeLists.txt']
 })){
  const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+source+'.json','utf8')));
  for(const file of files)assert.ok(doc.sections.some(s=>s.title===file&&s.blocks.some(b=>b.type==='code'&&b.text.length>200)),file+' not accessible '+source);
 }
});
test('DX11 09-12 GitHub C++ sources match native Notion original code fences',async()=>{
 const mappings=[
  {origin:'dx11-09/native-notion-source-09.md',head:'main.cpp',file:'dx11-09/main.cpp'},
  {origin:'dx11-10/native-notion-source-10.md',head:'main.cpp',file:'dx11-10/main.cpp'},
  {origin:'dx11-11-12/native-notion-source-11.md',head:'main.cpp',file:'dx11-11-12/11_collision/main.cpp'}
 ];
 const sourceFrom=(text:string,head:string)=>{
  const heading='## '+head+'\n',i=text.indexOf(heading);
  assert.ok(i>=0,'missing Markdown heading '+head);
  const startFence=text.indexOf('```',i),start=text.indexOf('\n',startFence)+1,end=text.indexOf('\n```',start);
  assert.ok(startFence>=0&&start>0&&end>=start,'unclosed source fence '+head);
  return text.slice(start,end);
 };
 for(const m of mappings){
  const raw=await readFile('../knowledge/shared/'+m.origin,'utf8');
  const code=await readFile('../knowledge/shared/'+m.file,'utf8');
  assert.equal(code,sourceFrom(raw,m.head),m.file+' byte-for-byte source mismatch');
 }
 const native12=await readFile('../knowledge/shared/dx11-11-12/native-notion-source-12.md','utf8');
 const rebuilt=sourceFrom(native12,'main.cpp Part 1/2')+'\n'+sourceFrom(native12,'main.cpp Part 2/2');
 const code12=await readFile('../knowledge/shared/dx11-11-12/12_navmesh/main.cpp','utf8');
 assert.equal(code12,rebuilt,'navmesh two-part exact join mismatch');
});

test('DX11 13 through 18 migration preserves original diagram/table/review and independent sources',async()=>{
 const originals=['dx11-13','dx11-13b','dx11-14','dx11-16','dx11-17','dx11-18'];
 const explorers=['dx11-sources-13','dx11-sources-13b','dx11-sources-14','dx11-sources-16','dx11-sources-17','dx11-sources-18'];
 for(const slug of originals){
  const original=await readFile('../knowledge/'+slug+'/source-notion.md','utf8');
  const page=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+slug+'.json','utf8')));
  assert.ok(original.length>3800,'original incomplete '+slug);
  assert.ok(original.includes('```mermaid'),'original Mermaid missing '+slug);
  const blocks=page.sections.flatMap(s=>s.blocks);
  const approved=legacyChapters.find(chapter=>chapter.slug===slug);
  if(approved){
   // Frozen original payloads retain multiplicity; added diagrams/tables/answers are independently exercised.
   assertLegacyPayload(blocks,approved);
  }else{
  assert.equal(blocks.filter(b=>b.type==='code'&&b.language==='mermaid').length,(original.match(/```mermaid/g)||[]).length,'diagram count '+slug);
  assert.equal(blocks.filter(b=>b.type==='table').length,(original.match(/<table\b/g)||[]).length,'table count '+slug);
  assert.equal(blocks.filter(b=>b.type==='details').length,(original.match(/<details>/g)||[]).length+1,'answer count + verification '+slug);
  }
  assert.ok(page.sections.some(s=>s.id==='github-source'&&s.blocks.some(b=>b.type==='paragraph'&&b.content.some(n=>n.kind==='link'&&n.href?.startsWith('/knowledge/dx11-sources-')))),'source link '+slug);
 }
 for(const slug of explorers){
  const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+slug+'.json','utf8')));
  const fileBlocks=doc.sections.flatMap(s=>s.blocks).filter(b=>b.type==='code');
  assert.ok(fileBlocks.some(b=>b.language==='cpp'&&b.text.length>3000),'full CPU C++ missing '+slug);
  assert.ok(fileBlocks.some(b=>b.language==='cmake'&&b.text.includes('project(')),'full CMake missing '+slug);
 }
 const sample=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--dx11-13.json','utf8')));
 assert.ok(sample.sections.some(s=>s.id==='github-source'&&s.blocks.some(b=>b.type==='callout')),'historic SHA mismatch warning missing');
 const frustum=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--dx11-frustum.json','utf8')));
 assert.ok(frustum.title.includes('Frustum'),'preserve original chapter 15');
 assert.ok((await readFile('../knowledge/dx11-frustum/main.cpp','utf8')).length>5000,'existing 15 CPU source missing');
});
test('DX11 source reconstruction preserves actual Notion code block bytes and current 13A SHA',async()=>{
 const plans=[
  {original:'shared/dx11-13a-notion-source.md',parts:[0,1,2,3],file:'shared/dx11-13/main.cpp'},
  {original:'shared/dx11-13b1-notion-source.md',parts:[0],file:'shared/dx11-13b/main.cpp'},
  {original:'shared/dx11-13b-notion-source.md',parts:[0],file:'shared/dx11-13b-compact/main.cpp'},
  {original:'shared/dx11-14-notion-source.md',parts:[0,1,2,3,4],file:'shared/dx11-14/main.cpp'},
  {original:'dx11-frustum/source-notion.md',parts:[1,2,3,4],file:'dx11-frustum/main.cpp'},
  {original:'shared/dx11-16-notion-source.md',parts:[1,2,3,4,5,6,7,8],file:'shared/dx11-16/main.cpp'},
  {original:'shared/dx11-17b-notion-source.md',parts:[0],file:'shared/dx11-17/fontlab.hpp'},
  {original:'shared/dx11-17c-notion-source.md',parts:[0],file:'shared/dx11-17/main.cpp'},
  {original:'shared/dx11-18b-notion-source.md',parts:[0],file:'shared/dx11-18/main.cpp'}
 ];
 for(const plan of plans){
  const original=await readFile('../knowledge/'+plan.original,'utf8');
  // The capture ends at the closing fence, so the original terminal LF stays inside it.
  const parts=[...original.matchAll(/^```c\+\+\r?\n([\s\S]*?)^```[ \t]*\r?$/gm)].map(m=>m[1]);
  for(const index of plan.parts)assert.ok(index<parts.length,'missing C++ source block '+plan.original+':'+index);
  const reconstructed=Buffer.from(plan.parts.map(n=>parts[n]).join(''),'utf8');
  const saved=await readFile('../knowledge/'+plan.file);
  assert.deepEqual(saved,reconstructed,'Notion-native CPU source mismatch including terminal LF '+plan.file);
 }
 const current=await readFile('../knowledge/shared/dx11-13/main.cpp');
 assert.equal(current.length,6043);
 assert.equal(createHash('sha256').update(current).digest('hex'),'ef9b0cfe2e764adc896a45451f5607fd751ab0c8b523e6477339994ba758a4b8');
 const ledger=await readFile('../knowledge/MIGRATION_DX11_13_18_20261020.md','utf8');
 assert.ok(ledger.includes('fd3839f7c350d1e573cab36a8c3f942f55a8e6c9d4e94b45033e7d3b3b4472ae'));
 assert.ok(ledger.includes('파일 바이트를 확보하지 못했습니다'));
});

test('DX11 integrity manifest verifies page bodies, native CMake and raw historical logs',async()=>{
 const manifest=JSON.parse(await readFile('../knowledge/verification/dx11-13-18/source-integrity.json','utf8')) as {
  schemaVersion:number;
  files:{path:string;kind:string;sourcePath?:string;codeBlockIndexesZeroBased?:number[]}[];
  pages:{path:string}[];
 };
 assert.equal(manifest.schemaVersion,1);
 const entries=new Map(manifest.files.map(entry=>[entry.path,entry]));
 for(const folder of ['dx11-13','dx11-13b','dx11-13b-compact','dx11-14','dx11-16','dx11-17','dx11-18']){
  assert.ok(entries.has('knowledge/shared/'+folder+'/main.cpp'),'CPU source missing from manifest '+folder);
  assert.ok(entries.has('knowledge/shared/'+folder+'/CMakeLists.txt'),'CMake missing from manifest '+folder);
 }
 for(const path of ['knowledge/dx11-frustum/main.cpp','knowledge/dx11-frustum/CMakeLists.txt','knowledge/shared/dx11-17/fontlab.hpp','knowledge/shared/dx11-14/CMakeLists.notionshared.txt'])
  assert.ok(entries.has(path),'required preserved file missing '+path);
 const logs=manifest.files.filter(entry=>entry.path.endsWith('.log'));
 assert.ok(logs.length>=48,'all 48 recovered historical output files must remain recorded');
 for(const entry of logs){
  assert.ok(entry.sourcePath&&entries.has(entry.sourcePath),'historical stdout must have a pinned Notion snapshot '+entry.path);
  assert.ok(entry.codeBlockIndexesZeroBased?.length,'historical stdout must identify the exact original code block '+entry.path);
 }
 for(const page of manifest.pages)assert.ok(entries.has(page.path),'page body must be hash-pinned '+page.path);
 const result=spawnSync('python3',['../knowledge/scripts/verify-dx11-13-18.py'],{encoding:'utf8'});
 assert.ifError(result.error);
 assert.equal(result.status,0,result.stdout+'\n'+result.stderr);
 assert.match(result.stdout,/PASS source-integrity:/);
});

test('C++ foundation lessons publish visual assets, full Korean sources and preserved Notion captures',async()=>{
 function allBlocks(blocks:Block[]):Block[]{return blocks.flatMap(block=>[block,...('blocks'in block?allBlocks(block.blocks):block.type==='list'?block.items.flatMap(allBlocks):[])]);}
 function hasHref(value:unknown,href:string):boolean{
  if(!value||typeof value!=='object')return false;
  if(Array.isArray(value))return value.some(item=>hasHref(item,href));
  return ('href'in value&&value.href===href)||Object.values(value).some(item=>hasHref(item,href));
 }
 const docs=catalog.documents.filter(d=>d.kind==='knowledge'&&d.route.startsWith('/knowledge/cpp-basics-'));
 assert.deepEqual(docs.map(d=>d.route),['/knowledge/cpp-basics-01/','/knowledge/cpp-basics-02/','/knowledge/cpp-basics-03/','/knowledge/cpp-basics-04/','/knowledge/cpp-basics-05/']);
 const sources=[['types.cpp'],['flow.cpp'],['memory.cpp'],['containers.cpp','buffer.cpp'],['inventory.hpp','inventory.cpp','inventory_main.cpp']];
 for(const [index,files] of sources.entries()){
  const slug='cpp-basics-'+String(index+1).padStart(2,'0');
  const doc=documentSchema.parse(JSON.parse(await readFile('public/'+docs[index].file,'utf8')));
  assert.deepEqual([...new Set(doc.sections.map(s=>s.tab))],['document','practice','sources','history']);
  const blocks=allBlocks(doc.sections.flatMap(s=>s.blocks));
  const images=blocks.filter(b=>b.type==='image');
  assert.ok(images.length>=2,'visual explanations missing '+slug);
  for(const image of images){
   assert.ok(image.type==='image');
   const source=await readFile('../'+image.src.slice(1));
   assert.deepEqual(await readFile('public/'+image.src.slice(1)),source,'asset bytes '+image.src);
  }
  for(const file of files){
   const koPath='/knowledge/shared/cpp-basics-lab/ko/'+file;
   assert.ok(hasHref(blocks,koPath),'whole Korean source link '+slug+': '+file);
   const koText=await readFile('..'+koPath,'utf8');
   assert.ok(blocks.some(b=>b.type==='code'&&b.language==='cpp'&&b.text===koText),'embedded complete Korean code matches actual file '+slug+': '+file);
   assert.deepEqual(await readFile('public'+koPath),await readFile('..'+koPath));
  }
  const original=await readFile('../knowledge/'+slug+'/source-notion.md');
  const capture=JSON.parse(await readFile('../knowledge/'+slug+'/source-notion-capture.json','utf8'));
  assert.equal(original.length,capture.source_bytes);
  assert.equal(createHash('sha256').update(original).digest('hex'),capture.sha256);
  assert.deepEqual(await readFile('public/knowledge/'+slug+'/source-notion.md'),original);
 }
 const first=documentSchema.parse(JSON.parse(await readFile('public/'+docs[0].file,'utf8')));
 assert.ok(allBlocks(first.sections.flatMap(s=>s.blocks)).some(b=>b.type==='cpp-bitmask-lab'));
 const integrity=spawnSync('python3',['../knowledge/shared/cpp-basics-lab/verify_sources.py'],{encoding:'utf8'});
 assert.ifError(integrity.error);
 assert.equal(integrity.status,0,integrity.stdout+'\n'+integrity.stderr);
 assert.match(integrity.stdout,/16 exact original code blocks/);
});

test('C++ course order keeps foundations before advanced lessons in both navigation consumers',()=>{
 const docs=catalog.documents.filter(d=>d.kind==='knowledge'&&d.category==='C++ 기초·심화');
 const ordered=[...docs].reverse().sort(compareKnowledgeDocuments);
 assert.deepEqual(ordered.map(d=>d.route),[
  '/knowledge/cpp-basics-01/','/knowledge/cpp-basics-02/','/knowledge/cpp-basics-03/',
  '/knowledge/cpp-basics-04/','/knowledge/cpp-basics-05/','/knowledge/cpp-advanced-01/','/knowledge/cpp-advanced-02/',
  '/knowledge/cpp-advanced-03/','/knowledge/cpp-advanced-04/','/knowledge/cpp-advanced-05/','/knowledge/cpp-advanced-06/',
 ]);
});

test('new numbered courses sort numerically and keep the original LIS reference last',()=>{
 const template=catalog.documents.find(d=>d.kind==='knowledge')!;
 for(const [category,prefix] of [['Modern C++','modern-cpp'],['Effective Modern C++','effective-modern-cpp'],['DirectX 12','dx12'],['그래픽스 기술','graphics-tech'],['알고리즘','algorithm-core']]){
  const docs=['12','02','01','10'].map(n=>({...template,category,route:'/knowledge/'+prefix+'-'+n+'/'}));
  assert.deepEqual(docs.sort(compareKnowledgeDocuments).map(d=>d.route),['01','02','10','12'].map(n=>'/knowledge/'+prefix+'-'+n+'/'));
 }
 const items=[{...template,category:'알고리즘',route:'/knowledge/algorithm-lis/'},{...template,category:'알고리즘',route:'/knowledge/algorithm-core-12/'}];
 assert.deepEqual(items.sort(compareKnowledgeDocuments).map(d=>d.route),['/knowledge/algorithm-core-12/','/knowledge/algorithm-lis/']);
});

test('advanced lessons preserve ZIP bytes, historical evidence, complete annotated code and visual assets',async()=>{
 function allBlocks(blocks:Block[]):Block[]{return blocks.flatMap(block=>[block,...('blocks'in block?allBlocks(block.blocks):block.type==='list'?block.items.flatMap(allBlocks):[])]);}
 function hasHref(value:unknown,href:string):boolean{
  if(!value||typeof value!=='object')return false;
  if(Array.isArray(value))return value.some(item=>hasHref(item,href));
  return ('href'in value&&value.href===href)||Object.values(value).some(item=>hasHref(item,href));
 }
 const base='knowledge/shared/cpp-advanced-lab/';
 const manifest=JSON.parse(await readFile('../'+base+'source-integrity.json','utf8'));
 const preserved=[...Object.keys(manifest.artifacts),...Object.values(manifest.zip_files).map((file:any)=>file.path),...Object.keys(manifest.annotated_files)];
 for(const path of preserved){
  assert.deepEqual(await readFile('public/'+base+path),await readFile('../'+base+path),'published original bytes '+path);
 }
 for(const directory of (await readdir('../'+base+'verification')).filter(name=>name.startsWith('github-'))){
  const prefix=base+'verification/'+directory+'/';
  const evidence=JSON.parse(await readFile('../'+prefix+'artifact-manifest.json','utf8'));
  assert.equal(evidence.files.length,evidence.raw_file_count);
  assert.match(evidence.source_commit,/^[a-f0-9]{40}$/);
  for(const file of evidence.files){
   const raw=await readFile('../'+prefix+file.path);
   assert.equal(raw.length,file.bytes,'raw CI output length '+file.path);
   assert.equal(createHash('sha256').update(raw).digest('hex'),file.sha256,'raw CI output hash '+file.path);
   assert.deepEqual(await readFile('public/'+prefix+file.path),raw,'published raw CI output '+file.path);
  }
 }
 assert.equal(createHash('sha256').update(await readFile('public/'+base+'archive/cpp_advanced_effective_lab_v1.zip')).digest('hex'),'a97a4927e5d3472ad055dbffbad0e14ab72c4c81f6ea13a1d72b771d64d339ba');
 for(const [number,files]of [['01',['lifetime.cpp','check.hpp']],['02',['ownership.cpp','transaction.cpp']],['03',['polymorphism.cpp']],['04',['templates.cpp','interfaces.cpp']],['05',['containers.cpp']],['06',['allocation.cpp','pimpl.hpp','pimpl.cpp','pimpl_main.cpp']]]as const){
  const slug='cpp-advanced-'+number;
  const meta=catalog.documents.find(d=>d.route==='/knowledge/'+slug+'/');
  assert.ok(meta);
  const doc=documentSchema.parse(JSON.parse(await readFile('public/'+meta.file,'utf8')));
  assert.deepEqual([...new Set(doc.sections.map(s=>s.tab))],['document','practice','sources','history']);
  const blocks=allBlocks(doc.sections.flatMap(s=>s.blocks));
  const images=blocks.filter(b=>b.type==='image');
  assert.equal(images.length,3,'three concept illustrations '+slug);
  assert.ok(blocks.some(b=>b.type==='code'&&b.language==='mermaid'),'rendered diagram input '+slug);
  for(const image of images){
   assert.ok(image.type==='image');
   assert.ok(image.alt&&image.caption,'accessible visual explanation '+image.src);
   assert.deepEqual(await readFile('public'+image.src),await readFile('..'+image.src));
  }
  for(const file of files){
   const href='/'+base+'ko/'+file;
   assert.ok(hasHref(blocks,href),'complete source link '+slug+': '+file);
   const text=await readFile('..'+href,'utf8');
   assert.ok(blocks.some(b=>b.type==='code'&&b.language==='cpp'&&b.text===text),'complete annotated code equals source '+file);
  }
  const source=await readFile('../knowledge/'+slug+'/source-notion.md');
  const capture=JSON.parse(await readFile('../knowledge/'+slug+'/source-notion-capture.json','utf8'));
  assert.equal(source.length,capture.source_bytes);
  assert.equal(createHash('sha256').update(source).digest('hex'),capture.sha256);
  assert.deepEqual(await readFile('public/knowledge/'+slug+'/source-notion.md'),source);
 }
 const integrity=spawnSync('python3',['../'+base+'verify_sources.py'],{encoding:'utf8'});
 assert.ifError(integrity.error);
 assert.equal(integrity.status,0,integrity.stdout+'\n'+integrity.stderr);
});

test('C sample course keeps regular order, executable sources and honest new authorship',async()=>{
 const docs=catalog.documents.filter(d=>d.kind==='knowledge'&&d.category==='C 언어').sort(compareKnowledgeDocuments);
 // Derive membership from authoring data, but check the pedagogical ordering independently.
 const sourceCatalog=JSON.parse(await readFile('../knowledge/data/catalog.json','utf8')) as {path:string;category:string}[];
 const chapters=sourceCatalog.filter(entry=>entry.category==='C 언어'&&/^c-basics-\d+$/.test(entry.path))
  .map(entry=>({route:'/knowledge/'+entry.path+'/',number:Number(entry.path.slice('c-basics-'.length))}))
  .sort((a,b)=>a.number-b.number);
 assert.ok(chapters.length>=5,'preserve the established C basics');
 const expected=[...chapters.filter(chapter=>chapter.number<=11).map(chapter=>chapter.route),
  '/knowledge/c-language-01/',...chapters.filter(chapter=>chapter.number>11).map(chapter=>chapter.route)];
 assert.deepEqual(docs.map(doc=>doc.route),expected);
 const report=JSON.parse(await readFile('../knowledge/shared/c-basics-lab/verification.json','utf8'));
 assert.equal(report.status,'pass');assert.equal(report.distinct_addition_boundary_cases,9);
 assert.equal(report.commands.filter((c:{expected_success:boolean})=>!c.expected_success).length,2);
 for(const file of report.source_files){
  const bytes=await readFile('../'+file.path);
  assert.equal(createHash('sha256').update(bytes).digest('hex'),file.sha256,file.path);
  assert.deepEqual(await readFile('public/'+file.path),bytes);
 }
 for(const doc of docs.slice(0,3)){
  const full=documentSchema.parse(JSON.parse(await readFile('public/'+doc.file,'utf8')));
  assert.deepEqual([...new Set(full.sections.map(s=>s.tab))],['document','practice','sources','history']);
  const blocks=full.sections.flatMap(s=>s.blocks);
  assert.equal(blocks.filter(b=>b.type==='image').length,2);
  assert.ok(blocks.some(b=>b.type==='code'&&b.language==='mermaid'));
  assert.equal(full.sections.find(s=>s.id==='review')!.blocks.filter(b=>b.type==='details').length,6);
  assert.ok(JSON.stringify(full.sections.filter(s=>s.tab==='history')).includes('Notion 원문 이관본이 아니며'));
  const source=await readFile('../'+doc.route.slice(1)+'main.c','utf8');
  assert.ok(full.sections.find(s=>s.id==='full-code')!.blocks.some(b=>b.type==='details'&&b.blocks.some(c=>c.type==='code'&&c.text===source)));
  for(const b of blocks)if(b.type==='image')assert.ok((await readFile('public/'+b.src.slice(1),'utf8')).includes('<svg'));
 }
 // New C11 lessons must retain full executable sources and real visual blocks.
 for(const slug of chapters.filter(chapter=>chapter.number>=4).map(chapter=>chapter.route.split('/')[2])){
  const doc=docs.find(d=>d.route==='/knowledge/'+slug+'/')!;
  assert.ok(doc,'missing new lesson '+slug);
  const full=documentSchema.parse(JSON.parse(await readFile('public/'+doc.file,'utf8')));
  assert.deepEqual([...new Set(full.sections.map(s=>s.tab))],['document','practice','sources','history']);
  const blocks=full.sections.flatMap(s=>s.blocks);
  const images=blocks.filter(b=>b.type==='image');
  assert.equal(images.length,2,'concept SVG count '+slug);
  for(const image of images)if(image.type==='image')assert.ok((await readFile('public/'+image.src.slice(1),'utf8')).includes('<svg'),'missing SVG '+image.src);
  assert.ok(blocks.some(b=>b.type==='code'&&b.language==='mermaid'),'missing Mermaid '+slug);
  assert.ok(full.sections.find(s=>s.id==='review')!.blocks.filter(b=>b.type==='details').length>=6,'missing review '+slug);
  const source=await readFile('../knowledge/'+slug+'/main.c','utf8');
  assert.ok(full.sections.find(s=>s.id==='full-code')!.blocks.some(b=>b.type==='details'&&b.blocks.some(c=>c.type==='code'&&c.text===source)),'full source mismatch '+slug);
 }
 const future=[12,11,14].map(n=>({...docs[0],route:`/knowledge/c-basics-${n}/` }));
 assert.deepEqual([...future,docs.find(d=>d.route==='/knowledge/c-language-01/')!].sort(compareKnowledgeDocuments).map(d=>d.route),['/knowledge/c-basics-11/','/knowledge/c-language-01/','/knowledge/c-basics-12/','/knowledge/c-basics-14/']);
});

// Explicit rollout membership must not depend on the document already having four tabs.
test('approved legacy visual rollout preserves original evidence and adds every four-tab lesson',async()=>{
 for(const expected of legacyChapters){
  const doc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+expected.slug+'.json','utf8')));
  assert.deepEqual([...new Set(doc.sections.map(s=>s.tab))],['document','practice','sources','history'],expected.slug);
  for(const id of expected.originalSectionIds)assert.ok(doc.sections.some(s=>s.id===id),expected.slug+' original section '+id);
  const blocks=legacyNested(doc.sections.flatMap(s=>s.blocks));
  assertLegacyPayload(doc.sections.flatMap(s=>s.blocks),expected);
  for(const archive of expected.sourceArchives??[{path:expected.sourceFile,sha256:expected.sourceSha256}]){
   const source=await readFile('../'+archive.path);
   assert.equal(createHash('sha256').update(source).digest('hex'),archive.sha256,archive.path);
   assert.deepEqual(await readFile('public/'+archive.path),source,archive.path+' published bytes');
  }
  assert.equal(blocks.filter(b=>b.type==='image').length,expected.images,expected.slug);
  assert.ok(blocks.some(b=>b.type==='code'&&b.language==='mermaid'),expected.slug+' diagrams');
  const review=doc.sections.find(s=>s.id===expected.reviewSection)!;assert.ok(review,expected.slug+' review section');
  assert.ok(review.blocks.filter(b=>b.type==='details').length>=6,expected.slug+' new review questions');
  assert.ok(expected.koFiles.length>0,expected.slug+' complete Korean source links');
  const sourceDocs=new Map([[expected.slug,doc]]);
  for(const view of expected.sourceViews??[]){
   const sourceDoc=documentSchema.parse(JSON.parse(await readFile('public/data/atlas/docs/knowledge--'+view.sourceLibrary+'.json','utf8')));sourceDocs.set(view.sourceLibrary,sourceDoc);
   const section=sourceDoc.sections.find(s=>s.id===view.sourceSection);assert.ok(section,view.sourceLibrary+' source section');assert.equal(section.tab,view.sourceTab);
   const displayed=codeBlocks(section.blocks);
   for(const href of view.displayedCodeFiles){assert.ok(expected.koFiles.includes(href));assert.ok(displayed.includes(await readFile('..'+href,'utf8')),view.sourceLibrary+' full displayed bytes '+href);}
  }
  const encoded=JSON.stringify([...sourceDocs.values()]);for(const href of expected.koFiles)assert.ok(encoded.includes(JSON.stringify(href)),expected.slug+' linked source '+href);
 }
});


test('native teaching build resources are published as exact inert source bytes',async()=>{
 const paths=[
  'knowledge/shared/game-math-winapi-lab/windows/app.rc',
  'knowledge/shared/game-math-winapi-lab/windows/app.manifest',
  'knowledge/shared/dx9-pipeline-lab/examples/texture_sm3.hlsl',
  'knowledge/shared/dx11-04/texture.hlsl',
 ];
 if(legacyChapters.some(chapter=>chapter.slug==='dx9-impl-10'))paths.push('knowledge/shared/dx9-impl-10-ko/shader_contract.fx');
 for(const path of paths){
  const original=await readFile('../'+path);assert.ok(original.length>0,path);
  assert.deepEqual(await readFile('public/'+path),original,'native source byte preservation '+path);
 }
});

// Fixed migration evidence records historic state, never generated from rewritten content.
test('all original thinking routes, memo keys, dates and anchors survive paired revision',async()=>{
 const raw=JSON.parse(await readFile('../thinking-lab/data/catalog.json','utf8'));
 const policy=JSON.parse(await readFile('../automation/publishing.json','utf8')).jobs.find((j:{id:string})=>j.id==='thinking');
 const result=validateThinkingPairs(raw,policy);assert.ok(result.topics>=thinkingMigration.topics.length);
 const keys=new Set<string>();
 for(const e of raw.episodes){const meta=catalog.documents.find(d=>d.kind==='thinking'&&d.number===e.number)!;assert.ok(meta.memoKey);assert.ok(!keys.has(meta.memoKey!));keys.add(meta.memoKey!);}
 for(const old of thinkingMigration.topics){
  const basic=catalog.documents.find(d=>d.kind==='thinking'&&d.number===old.topic+'.1')!;
  const advanced=catalog.documents.find(d=>d.kind==='thinking'&&d.number===old.topic+'.2')!;
  assert.ok(basic&&advanced,old.topic);assert.equal(basic.route,'/thinking-lab/'+old.path);assert.equal(basic.memoKey,old.memoKey);
  assert.equal(basic.date,old.study_date);assert.equal(advanced.date,old.study_date);assert.notEqual(basic.id,advanced.id);assert.notEqual(basic.memoKey,advanced.memoKey);
  for(const meta of [basic,advanced]){const entry=raw.episodes.find((e:{number:string})=>e.number===meta.number);assert.equal(entry.published_date,old.published_date);}
  const doc=JSON.parse(await readFile('../'+basic.source,'utf8'));
  const ids=new Set(doc.sections.map((s:{id:string})=>s.id));
  for(const id of old.sectionIds)assert.ok(ids.has(id),old.topic+' preserved anchor '+id);
 }
 for(const mode of ['missing','same-date-other-topic','duplicate-number','page-progress','stale-current','missing-date','bad-calendar-date','bad-next-date','leading-zero']){
  const bad=structuredClone(raw);
  if(mode==='missing')bad.episodes.pop();
  if(mode==='same-date-other-topic'){const date=bad.episodes[0].study_date;bad.episodes.filter((e:{number:string})=>e.number.startsWith('1.1.')).forEach((e:{study_date:string})=>e.study_date=date);}
  if(mode==='duplicate-number')bad.episodes[1].number=bad.episodes[0].number;
  if(mode==='page-progress')bad.current.number+='.'+1;
  if(mode==='stale-current'){bad.current.number='1.1';bad.current.study_date='2026-10-09';}
  if(mode==='missing-date')bad.episodes[0].study_date=undefined;
  if(mode==='bad-calendar-date')bad.episodes[0].published_date='2026-02-31';
  if(mode==='bad-next-date')bad.next.study_date=bad.current.study_date;
  if(mode==='leading-zero')bad.episodes[0].number='0'+bad.episodes[0].number;
  assert.throws(()=>validateThinkingPairs(bad,policy),mode);
 }
});
