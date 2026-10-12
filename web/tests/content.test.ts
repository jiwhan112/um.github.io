import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFile,readdir} from 'node:fs/promises';
import {load} from 'cheerio';
import {buildContent,compileBlocks,compileNews,safeLink} from '../scripts/content';
import {parseStored,briefingState,objectBooleans} from '../src/lib/storage';
import {lessonProgress,reviewId} from '../src/lib/language-progress';
import {canSyncKey} from '../src/lib/cloud';
import {filterDocuments,readLibraryFilter,matchesDocument} from '../src/lib/document-library';
import type {Block} from '../src/lib/types';
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
 for(const k of ['jiwhan-briefing-v1','atlas-reading-v1','atlas-language-sections-v1','atlas-review-v1','thinking-ctci-1.4-draft','atlas-code-thinking--example'])assert.equal(canSyncKey(k),true,k);
 for(const k of ['atlas-cloud-session-v1','atlas-cloud-settings-v1','atlas-cloud-pending-v1','other-app-password','language-study-sw-cache'])assert.equal(canSyncKey(k),false,k);
 assert.equal(reviewId({language:'english',from_lesson:9,term:'hello',due_date:'2026-10-12'}),'english:9:hello:2026-10-12');
});
