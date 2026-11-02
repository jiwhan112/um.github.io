import { z } from 'zod';
/** Derived notebooks: original lessons and their review/storage identifiers stay untouched. */
export type NotebookLanguage = 'english' | 'japanese';
export type NotebookArticle = { title: string; publisher: string; date: string; url: string };
export type NotebookLine = { en?: string; jp?: string; pron?: string; ko?: string; tts?: string };
export type NotebookLesson = {
 id: number; date: string; title: string; article?: NotebookArticle;
 vocabulary?: { term: string; meaning: string; pron?: string; ko?: string; tts?: string }[];
 words?: NotebookLine[]; reading?: NotebookLine[]; conversation?: NotebookLine[]; examples?: NotebookLine[];
};
export type NotebookSource = {
 lessonId: number; lessonTitle: string; date: string; route: string;
 section: 'vocabulary' | 'words'; wordIndex: number; originalTerm: string; meaning: string; pronunciation: string;
 article?: NotebookArticle;
};
export type NotebookExample = {
 text: string; pron: string; ko: string; tts: string; lessonId: number;
 section: 'reading' | 'conversation' | 'examples';
};
export type NotebookEntry = {
 id: string; language: NotebookLanguage; term: string; reading: string;
 pronunciations: string[]; romaji: string; tts: string; meanings: string[];
 sources: NotebookSource[]; examples: NotebookExample[];
 /** Number of distinct lessons containing this vocabulary entry, not general-language frequency. */
 occurrences: number; kanji: string[];
};
export type WordNotebook = {
 version: 1; english: NotebookEntry[]; japanese: NotebookEntry[]; kanji: NotebookEntry[];
 stats: { englishLessons: number; japaneseLessons: number; englishRows: number; japaneseRows: number; englishWords: number; japaneseWords: number; kanjiWords: number; englishWithoutPronunciation: number };
};
const han = /[\p{Script=Han}々]/u;
const ruby = /([\p{Script=Han}々ぁ-ゖァ-ヺー]+)[（(]([ぁ-ゖァ-ヺー/／]+)[）)]/gu;
const normalize = (value: string) => value.normalize('NFKC').trim().replace(/\s+/g, ' ');
const unique = (values: string[]) => [...new Set(values.filter(Boolean))];
/** Exact code-point encoding is deterministic and collision-free; it does not depend on list order. */
export function notebookEntryId(language: NotebookLanguage, term: string, reading = ''): string {
 return `word-notebook-v1:${language}:` + [...normalize(term).toLowerCase() + '\u0000' + normalize(reading)].map(c => c.codePointAt(0)!.toString(16)).join('-');
}
export function japaneseForms(value: string): { term: string; reading: string } {
 return { term: value.replace(ruby, '$1').trim(), reading: value.replace(ruby, '$2').trim() };
}
function englishMeaning(item: NonNullable<NotebookLesson['vocabulary']>[number]): string {
 return item.ko?.trim() || item.meaning.replace(/^발음\s*[:：][\s\S]*?[·|]\s*뜻\s*[:：]\s*/, '').trim();
}
function englishPronunciation(item: NonNullable<NotebookLesson['vocabulary']>[number]): string {
 return item.pron?.trim() || item.meaning.match(/발음\s*[:：]\s*([^·|]+)[·|]\s*뜻\s*[:：]/)?.[1]?.trim() || '';
}
function annotatedJapanese(item: NotebookLine): {term:string;reading:string} {
 const original=japaneseForms(item.jp||'');
 if(han.test(original.term))return original;
 // A word's own explicit kanji spelling in its definition can join kana/kanji variants.
 for(const match of (item.ko||'').matchAll(ruby)){
  if(normalize(match[2])===normalize(original.reading)) return {term:match[1],reading:match[2]};
 }
 return original;
}
function entryExample(lesson: NotebookLesson, language: NotebookLanguage, term: string, reading: string): NotebookExample | undefined {
 const sections = language==='english'?['reading','conversation'] as const:['examples'] as const;
 const escaped=term.replace(/[.*+?^${}()|[\]\\]/g,'\\$&');
 const englishPattern=new RegExp(`(^|[^a-z])${escaped}([^a-z]|$)`,'i');
 for(const section of sections)for(const row of lesson[section]||[]){
  const text=language==='english'?row.en||'':row.jp||'';
  if(language==='english'?englishPattern.test(text):text.includes(term)||japaneseForms(text).reading.includes(reading))return {text,pron:row.pron||'',ko:row.ko||'',tts:row.tts||(language==='japanese'?japaneseForms(text).reading:text),lessonId:lesson.id,section};
 }
 return undefined;
}
function compile(lessons: NotebookLesson[], language: NotebookLanguage): NotebookEntry[] {
 const entries=new Map<string,NotebookEntry>();
 for(const lesson of [...lessons].sort((a,b)=>a.id-b.id)){
  const rows=language==='english'?lesson.vocabulary||[]:lesson.words||[];
  rows.forEach((raw,index)=>{
   const en=raw as NonNullable<NotebookLesson['vocabulary']>[number], jp=raw as NotebookLine;
   const originalTerm=language==='english'?en.term:jp.jp||'';
   if(!originalTerm.trim())throw new Error(`Empty notebook word: ${language}/${lesson.id}/${index}`);
   const forms=language==='english'?{term:normalize(en.term),reading:''}:annotatedJapanese(jp);
   const term=normalize(forms.term),reading=normalize(forms.reading),id=notebookEntryId(language,term,reading);
   const meaning=language==='english'?englishMeaning(en):jp.ko||'';
   const pron=language==='english'?englishPronunciation(en):jp.pron||'';
   const entry=entries.get(id)||{id,language,term,reading,pronunciations:[],romaji:'',tts:language==='english'?en.tts||term:han.test(reading)||/[／/]/.test(reading)?term:reading,meanings:[],sources:[],examples:[],occurrences:0,kanji:unique([...term].filter(c=>han.test(c)))};
   entry.meanings=unique([...entry.meanings,meaning]);entry.pronunciations=unique([...entry.pronunciations,pron]);
   entry.sources.push({lessonId:lesson.id,lessonTitle:lesson.title,date:lesson.date,route:`/language-study/${language}/${lesson.id}/`,section:language==='english'?'vocabulary':'words',wordIndex:index,originalTerm,meaning,pronunciation:pron,...(language==='english'&&lesson.article?{article:{title:lesson.article.title,publisher:lesson.article.publisher,date:lesson.article.date,url:lesson.article.url}}:{})});
   const example=entryExample(lesson,language,term,reading);
   if(example&&!entry.examples.some(e=>e.lessonId===example.lessonId&&e.text===example.text))entry.examples.push(example);
   entries.set(id,entry);
  });
 }
 for(const entry of entries.values())entry.occurrences=new Set(entry.sources.map(s=>s.lessonId)).size;
 return [...entries.values()].sort((a,b)=>a.term.localeCompare(b.term,language==='english'?'en':'ja')||a.reading.localeCompare(b.reading,'ja'));
}
export function buildWordNotebook(input:{english:NotebookLesson[];japanese:NotebookLesson[]}):WordNotebook {
 const english=compile(input.english,'english'),japanese=compile(input.japanese,'japanese');
 const kanji=japanese.filter(e=>e.kanji.length).sort((a,b)=>b.occurrences-a.occurrences||a.reading.localeCompare(b.reading,'ja'));
 return {version:1,english,japanese,kanji,stats:{englishLessons:input.english.length,japaneseLessons:input.japanese.length,englishRows:input.english.reduce((n,l)=>n+(l.vocabulary?.length||0),0),japaneseRows:input.japanese.reduce((n,l)=>n+(l.words?.length||0),0),englishWords:english.length,japaneseWords:japanese.length,kanjiWords:kanji.length,englishWithoutPronunciation:english.filter(e=>!e.pronunciations.length).length}};
}

const notebookArticleSchema=z.object({title:z.string(),publisher:z.string(),date:z.string(),url:z.string().url()});
const notebookSourceSchema=z.object({lessonId:z.number().int().positive(),lessonTitle:z.string(),date:z.string(),route:z.string(),section:z.enum(['vocabulary','words']),wordIndex:z.number().int().nonnegative(),originalTerm:z.string().min(1),meaning:z.string(),pronunciation:z.string(),article:notebookArticleSchema.optional()});
const notebookExampleSchema=z.object({text:z.string(),pron:z.string(),ko:z.string(),tts:z.string(),lessonId:z.number().int().positive(),section:z.enum(['reading','conversation','examples'])});
export const notebookEntrySchema=z.object({id:z.string().min(1),language:z.enum(['english','japanese']),term:z.string().min(1),reading:z.string(),pronunciations:z.array(z.string()),romaji:z.string(),tts:z.string().min(1),meanings:z.array(z.string()),sources:z.array(notebookSourceSchema).min(1),examples:z.array(notebookExampleSchema),occurrences:z.number().int().positive(),kanji:z.array(z.string())});
export const notebookSchema=z.object({version:z.literal(1),english:z.array(notebookEntrySchema),japanese:z.array(notebookEntrySchema),kanji:z.array(notebookEntrySchema),stats:z.object({englishLessons:z.number().int(),japaneseLessons:z.number().int(),englishRows:z.number().int(),japaneseRows:z.number().int(),englishWords:z.number().int(),japaneseWords:z.number().int(),kanjiWords:z.number().int(),englishWithoutPronunciation:z.number().int()})});
