import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFile,readdir} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {validateFoundation,foundationProgress,foundationProgressKey,foundationWords} from '../src/lib/japanese-foundation';
import preserved from './fixtures/foundation-preserved-lessons.json' with {type:'json'};
const dir='../language-study/foundation/japanese/';
const meta=JSON.parse(await readFile(dir+'course.json','utf8'));
const files=(await readdir(dir+'units')).filter(f=>/^jf-\d{2}\.json$/.test(f)).sort();
const raw={...meta,units:await Promise.all(files.map(async f=>JSON.parse(await readFile(dir+'units/'+f,'utf8'))))};
const course=validateFoundation(raw);
test('foundation has all 30 ordered authored chapters and six distinct curriculum areas',()=>{
 assert.equal(course.units.length,30);assert.equal(course.units[0].kana?.length,92);assert.deepEqual([...new Set(course.units.map(u=>u.module))],['letters','sentences','verbs','expressions','reading','review']);
 assert.ok(course.scope.join(' ').includes('보장하지 않습니다'));
 for(const u of course.units){assert.ok(u.sections.flatMap(s=>s.examples).length>=6);assert.ok(u.dialogue.lines.length>=6);assert.ok(u.listening.lines.length>=4);if(u.order>1)assert.ok(u.practice.filter(q=>q.type==='review'||q.reviewOf).length>=2,u.id);}
});
test('foundation rejects missing chapters, duplicate IDs and forward prerequisites',()=>{
 assert.throws(()=>validateFoundation({...raw,units:raw.units.slice(1)}));
 const duplicate=structuredClone(raw);duplicate.units[1].id=duplicate.units[0].id;assert.throws(()=>validateFoundation(duplicate));
 const future=structuredClone(raw);future.units[0].prerequisites=['jf-30'];assert.throws(()=>validateFoundation(future));
});
test('every Japanese sample preserves Japanese TTS and has reading, pronunciation and translation',()=>{
 for(const u of course.units)for(const x of [...(u.kana||[]),...u.sections.flatMap(s=>s.examples),...u.dialogue.lines,...u.vocabulary,...u.listening.lines,...u.practice.flatMap(q=>q.prompt?[q.prompt]:[])]){
  assert.equal(x.tts,x.ja);assert.ok(x.reading&&x.pron&&x.ko&&x.romaji);assert.ok(!/[\p{Script=Han}]/u.test(x.reading));
 }
 const broken=structuredClone(raw);broken.units[0].sections[0].examples[0].tts='다른 음성';assert.throws(()=>validateFoundation(broken));
 const mixed=structuredClone(raw);mixed.units[0].sections[0].examples[0].ja='あ 한국어 English';mixed.units[0].sections[0].examples[0].tts='あ 한국어 English';assert.throws(()=>validateFoundation(mixed));
 const reading=structuredClone(raw);reading.units[0].sections[0].examples[0].reading='This is not kana';assert.throws(()=>validateFoundation(reading));
 const choice=structuredClone(raw);choice.units.flatMap((u:any)=>u.practice).find((q:any)=>q.type==='choice').answer='not an option';assert.throws(()=>validateFoundation(choice));
});
test('foundation progress uses namespaced keys and never counts daily lesson records',()=>{
 const state={'japanese:1:lang-grammar':true,'english:30:lang-reading':true};assert.equal(foundationProgress(state,'jf-01').count,0);
 const next={...state,...Object.fromEntries(['learn','dialogue','words','practice','listening'].map(s=>[foundationProgressKey('jf-01',s as 'learn'),true]))};
 assert.equal(foundationProgress(next,'jf-01').done,true);assert.equal(foundationProgress(next,'jf-02').count,0);assert.equal(next['japanese:1:lang-grammar'],true);
});
test('all existing 60 daily lesson source files remain byte-identical',async()=>{
 assert.equal(Object.keys(preserved).length,60);
 for(const [path,hash]of Object.entries(preserved))assert.equal(createHash('sha256').update(await readFile('../'+path)).digest('hex'),hash,path);
});
test('course vocabulary keeps distinct readings and meanings and valid source-unit links',()=>{
 const words=foundationWords(course.units);assert.ok(words.length>=120);assert.equal(new Set(words.map(w=>w.id)).size,words.length);
 for(const w of words){assert.ok(w.units.length);for(const u of w.units)assert.ok(u.vocabulary.some(x=>x.ja===w.value.ja&&x.reading===w.value.reading&&x.ko===w.value.ko));}
});
test('foundation source references are HTTPS and every legacy lesson exists',async()=>{
 for(const u of course.units){for(const s of u.sources)assert.ok(s.url.startsWith('https://'));for(const id of u.legacyLessons)assert.ok(await readFile('../language-study/data/japanese/'+String(id).padStart(3,'0')+'.json','utf8'));}
});
