import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFile,readdir} from 'node:fs/promises';
import {load} from 'cheerio';
import {buildContent,compileBlocks,compileNews,safeLink} from '../scripts/content';
import {parseStored,briefingState,objectBooleans} from '../src/lib/storage';
import type {Block} from '../src/lib/types';
const catalog=await buildContent();
test('all source lessons and documents are compiled',()=>{assert.equal(catalog.version,2);assert.ok(catalog.documents.length>=9);assert.ok(catalog.languages.english.length>=9);assert.ok(catalog.languages.japanese.length>=9);assert.ok(catalog.articleCount>=135);});
test('memo storage keys are preserved exactly',()=>{const expected:Record<string,string>={'1.1':'thinking-ctci-1-1-draft','1.2':'thinking-ctci-1-2-draft','1.3':'thinking-ctci-1.3-draft','1.4':'thinking-ctci-1.4-draft'};for(const [number,key]of Object.entries(expected))assert.equal(catalog.documents.find(d=>d.kind==='thinking'&&d.number===number)?.memoKey,key);});
function codeBlocks(bs:Block[]):string[]{return bs.flatMap(b=>b.type==='code'?[b.text]:'blocks'in b?codeBlocks(b.blocks):b.type==='list'?b.items.flatMap(codeBlocks):[]);}
test('C++ examples are byte-for-byte unchanged after entity decoding',async()=>{
 for(const meta of catalog.documents.filter(d=>d.kind==='thinking')){const source=await readFile('../'+meta.source,'utf8'),$=load(source),document=JSON.parse(await readFile('public/'+meta.file,'utf8'));const expected=$('main pre').toArray().map(n=>$(n).text().replace(/^\n/,'').replace(/\n$/,''));const actual=codeBlocks(document.sections.flatMap((s:{blocks:Block[]})=>s.blocks));assert.deepEqual(actual,expected,meta.route);}
});
test('data adapter never includes scripts, styles or event handlers',()=>{const b=compileBlocks('<script>alert(1)</script><p onclick="evil()">safe <a href="javascript:alert(1)">link</a></p><style>*{display:none}</style>');const s=JSON.stringify(b);assert.ok(s.includes('safe'));assert.ok(!s.includes('evil'));assert.ok(!s.includes('alert'));assert.ok(!s.includes('display:none'));assert.equal(safeLink('javascript:alert(1)','/'),'');});
test('archived markdown body and stable article IDs remain available',()=>{const rows=compileNews({date:'2026-01-01',items:[{title:'test'}],original_markdown:'## 1. test\n\n본문입니다.\n\n```cpp\nint value = 1;\n```'});assert.equal(rows[0].id,'2026-01-01:0');assert.ok(rows[0].blocks.some(b=>b.type==='code'));});
test('corrupt and old storage is handled without deleting valid state',()=>{assert.deepEqual(parseStored('broken',{}),{});assert.deepEqual(briefingState({saved:['2026-10-12:0',3],read:['2026-10-11:1']}),{saved:['2026-10-12:0'],read:['2026-10-11:1']});assert.deepEqual(objectBooleans({'x':true,'y':'wrong'}),{x:true});});
test('production React code has no legacy DOM reparenting or raw HTML renderer',async()=>{
 async function files(dir:string):Promise<string[]>{return (await Promise.all((await readdir(dir,{withFileTypes:true})).map(x=>x.isDirectory()?files(dir+'/'+x.name):[dir+'/'+x.name]))).flat();}
 for(const f of await files('src'))if(/\.tsx?$/.test(f)){const s=await readFile(f,'utf8');assert.ok(!/dangerouslySetInnerHTML|\.innerHTML\s*=|\.appendChild\(|\.replaceChildren\(/.test(s),f);}
});
