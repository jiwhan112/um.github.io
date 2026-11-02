import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { buildWordNotebook, japaneseForms, notebookEntryId, notebookSchema, type NotebookLesson } from '../src/lib/word-notebook';
async function lessons(language:string):Promise<NotebookLesson[]>{
 const dir='../language-study/data/'+language;
 return Promise.all((await readdir(dir)).filter(f=>/^\d+\.json$/.test(f)).sort().map(async f=>JSON.parse(await readFile(dir+'/'+f,'utf8'))));
}
const input={english:await lessons('english'),japanese:await lessons('japanese')};
const before=JSON.stringify(input),book=buildWordNotebook(input);
test('notebook compilation preserves every authored vocabulary row and never changes lesson JSON',()=>{
 assert.equal(JSON.stringify(input),before);notebookSchema.parse(book);
 for(const language of ['english','japanese'] as const){
  const expected=input[language].flatMap(l=>(language==='english'?l.vocabulary||[]:l.words||[]).map((_,i)=>`${l.id}:${i}`));
  const actual=book[language].flatMap(e=>e.sources.map(s=>`${s.lessonId}:${s.wordIndex}`));
  assert.deepEqual(actual.sort(),expected.sort());
  assert.equal(new Set(book[language].map(e=>e.id)).size,book[language].length);
 }
});
test('entry IDs do not depend on source order, inserted lessons or display sorting',()=>{
 const reversed=buildWordNotebook({english:[...input.english].reverse(),japanese:[...input.japanese].reverse()});
 assert.deepEqual(reversed,book);
 assert.equal(notebookEntryId('english',' BOOK '),notebookEntryId('english','book'));
 assert.notEqual(notebookEntryId('japanese','日','ひ'),notebookEntryId('japanese','日','にち'));
 assert.notEqual(notebookEntryId('english','a-b'),notebookEntryId('english','ab'));
});
test('English vocabulary sources retain exact original article and lesson provenance',()=>{
 for(const entry of book.english)for(const source of entry.sources){
  const lesson=input.english.find(l=>l.id===source.lessonId)!;
  const article=lesson.article!;
  assert.deepEqual(source.article,{title:article.title,publisher:article.publisher,date:article.date,url:article.url});
  assert.equal(source.originalTerm,lesson.vocabulary![source.wordIndex].term);
  assert.equal(source.section,'vocabulary');assert.equal(source.route,`/language-study/english/${lesson.id}/`);
 }
});
test('missing article or pronunciation is not fabricated; separate meanings survive deduplication',()=>{
 const english:NotebookLesson[]=[{id:1,date:'2026-01-01',title:'대화',vocabulary:[{term:'record',meaning:'기록'}],conversation:[{en:'Keep a record.',pron:'킵 어 레커드',ko:'기록을 남겨요.'}]},{id:2,date:'2026-01-02',title:'읽기',vocabulary:[{term:'record',meaning:'발음: 리코드 · 뜻: 녹음하다'}]}];
 const entry=buildWordNotebook({english,japanese:[]}).english[0];
 assert.deepEqual(entry.meanings,['기록','녹음하다']);assert.deepEqual(entry.pronunciations,['리코드']);
 assert.ok(entry.sources.every(s=>s.article===undefined));assert.equal(entry.examples[0].section,'conversation');
 assert.equal(buildWordNotebook({english:[english[0]],japanese:[]}).stats.englishWithoutPronunciation,1);
});
test('kanji readings are copied from explicit annotations, including inflections and alternatives',()=>{
 assert.deepEqual(japaneseForms('読（よ）みます'),{term:'読みます',reading:'よみます'});
 assert.deepEqual(japaneseForms('何（なに／なん）'),{term:'何',reading:'なに／なん'});
 const sample=buildWordNotebook({english:[],japanese:[{id:1,date:'2026-01-01',title:'단어',words:[{jp:'みず',pron:'미즈',ko:'물. 水（みず）.'},{jp:'日（ひ）',pron:'히',ko:'날'},{jp:'日（にち）',pron:'니치',ko:'날을 세는 말'}]}]});
 assert.equal(sample.kanji.length,3);assert.ok(sample.kanji.some(e=>e.term==='水'&&e.reading==='みず'));
 assert.equal(new Set(sample.kanji.map(e=>e.id)).size,3);
});
test('kanji notebook is a subset with distinct-lesson frequency, not general-language ranking',()=>{
 for(const entry of book.kanji){
  assert.ok(book.japanese.includes(entry));assert.ok(entry.kanji.length>0);
  assert.equal(entry.occurrences,new Set(entry.sources.map(s=>s.lessonId)).size);
 }
 for(let i=1;i<book.kanji.length;i++)assert.ok(book.kanji[i-1].occurrences>=book.kanji[i].occurrences);
});
test('examples and translations remain exact lesson passages with truthful section labels',()=>{
 for(const language of ['english','japanese'] as const)for(const entry of book[language])for(const example of entry.examples){
  const lesson=input[language].find(l=>l.id===example.lessonId)!;
  const original=lesson[example.section]!.find(x=>(language==='english'?x.en:x.jp)===example.text)!;
  assert.ok(original);assert.equal(example.ko,original.ko||'');assert.equal(example.pron,original.pron||'');
 }
});
