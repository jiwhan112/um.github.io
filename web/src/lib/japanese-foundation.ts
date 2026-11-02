import {z} from 'zod';
const text=z.string().trim().min(1);
export const utteranceSchema=z.object({ja:text,reading:text,pron:text,ko:text,tts:text,romaji:text});
const line=utteranceSchema.extend({speaker:text});
export const foundationUnitSchema=z.object({
 id:z.string().regex(/^jf-\d{2}$/),order:z.number().int().min(1).max(30),module:z.enum(['letters','sentences','verbs','expressions','reading','review']),title:text,summary:text,
 kana:z.array(utteranceSchema).optional(),
 objectives:z.array(text).min(3),prerequisites:z.array(z.string().regex(/^jf-\d{2}$/)),legacyLessons:z.array(z.number().int().min(1).max(30)),
 sections:z.array(z.object({title:text,explanation:z.array(text).min(2),examples:z.array(utteranceSchema).min(2),pitfalls:z.array(text).min(1)})).min(3),
 dialogue:z.object({title:text,context:text,lines:z.array(line).min(6)}),vocabulary:z.array(utteranceSchema.extend({note:text})).min(6),
 practice:z.array(z.object({id:z.string().regex(/^jf-\d{2}-q\d{2}$/),type:z.enum(['choice','produce','review']),question:text,prompt:utteranceSchema.optional(),options:z.array(text).optional(),answer:text,explanation:text,reviewOf:z.string().regex(/^jf-\d{2}$/).optional()})).min(6),
 listening:z.object({title:text,instruction:text,lines:z.array(line).min(4),questions:z.array(z.object({question:text,answer:text,explanation:text})).min(2)}),
 checklist:z.array(text).min(3),sources:z.array(z.object({title:text,url:z.string().url().startsWith('https://'),note:text})).min(1)
});
export const foundationSchema=z.object({version:z.literal(1),title:text,publishedDate:z.string().regex(/^\d{4}-\d{2}-\d{2}$/),scope:z.array(text).min(2),units:z.array(foundationUnitSchema).length(30)});
export type Foundation=z.infer<typeof foundationSchema>;
export type FoundationUnit=z.infer<typeof foundationUnitSchema>;
export type Utterance=z.infer<typeof utteranceSchema>;
export const foundationModules={letters:'문자와 소리',sentences:'문장의 뼈대',verbs:'동사의 변화',expressions:'생활 표현',reading:'읽고 듣기',review:'종합 복습'} as const;
export const foundationSections={learn:'개념과 예문',dialogue:'회화',words:'단어',practice:'연습',listening:'듣기'} as const;
export type FoundationSection=keyof typeof foundationSections;
export const foundationProgressKey=(id:string,section:FoundationSection)=>`japanese-foundation:${id}:${section}`;
export function foundationProgress(state:Record<string,boolean>,id:string){const count=Object.keys(foundationSections).filter(s=>state[foundationProgressKey(id,s as FoundationSection)]).length;return {count,total:5,done:count===5};}
export function validateFoundation(value:unknown):Foundation{
 const course=foundationSchema.parse(value),ids=new Set(course.units.map(u=>u.id));
 if(ids.size!==30)throw Error('Duplicate foundation unit ID');
 course.units.forEach((u,i)=>{
  if(u.order!==i+1||u.id!==`jf-${String(i+1).padStart(2,'0')}`)throw Error('Foundation unit order mismatch: '+u.id);
  for(const p of u.prerequisites)if(!ids.has(p)||Number(p.slice(3))>=u.order)throw Error('Invalid prerequisite: '+u.id+' '+p);
  if(new Set(u.practice.map(q=>q.id)).size!==u.practice.length)throw Error('Duplicate practice ID: '+u.id);
  for(const q of u.practice){if(!q.id.startsWith(u.id+'-'))throw Error('Practice identity mismatch');if(q.reviewOf&&(!ids.has(q.reviewOf)||Number(q.reviewOf.slice(3))>=u.order))throw Error('Invalid review reference: '+q.id);if(q.type==='choice'&&(!q.options||q.options.length<2))throw Error('Missing answer options: '+q.id);if(q.type==='choice'&&!q.options!.includes(q.answer))throw Error('Choice answer missing from options: '+q.id);}
  const samples=[...(u.kana||[]),...u.sections.flatMap(s=>s.examples),...u.dialogue.lines,...u.vocabulary,...u.listening.lines,...u.practice.flatMap(q=>q.prompt?[q.prompt]:[])];
  for(const x of samples){if(/[\p{Script=Hangul}\p{Script=Latin}]/u.test(x.ja))throw Error('Non-Japanese speech text: '+u.id);if(!/^[\p{Script=Hiragana}\p{Script=Katakana}\p{P}\p{Z}\p{N}ー〜～\s]+$/u.test(x.reading))throw Error('Reading must use kana: '+u.id);if(x.tts!==x.ja)throw Error('TTS must preserve exact Japanese: '+u.id+' '+x.ja);if(!/[\p{Script=Hiragana}\p{Script=Katakana}\p{Script=Han}]/u.test(x.ja))throw Error('Missing Japanese: '+u.id);if(/[\p{Script=Han}]/u.test(x.reading))throw Error('Reading must resolve kanji: '+u.id+' '+x.reading);}
 });
 return course;
}
export function foundationWords(units:FoundationUnit[]){
 const rows=new Map<string,{id:string;value:FoundationUnit['vocabulary'][number];units:FoundationUnit[]}>();
 for(const u of units)for(const word of u.vocabulary){const id=[word.ja,word.reading,word.ko].join('|');const row=rows.get(id);if(row){if(!row.units.some(x=>x.id===u.id))row.units.push(u);}else rows.set(id,{id,value:word,units:[u]});}
 return [...rows.values()];
}
