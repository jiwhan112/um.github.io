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
 assert.equal(docs.length,13);
 assert.equal(sourceCatalog.length,13);
 for(const old of ['/knowledge/dx11-frustum/','/knowledge/algorithm-lis/','/knowledge/python-html/'])
  assert.ok(docs.some(d=>d.route===old),'preserve old prototype '+old);
 const imports=sourceCatalog.filter(x=>/^winapi-|^game-math-/.test(x.path));
 assert.equal(imports.length,10);
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
  assert.ok(full.sections.some(s=>s.blocks.some(b=>b.type==='code'&&b.language==='mermaid')),'diagram source lost '+entry.path);
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
