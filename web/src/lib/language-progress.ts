import type { Language, LessonMeta } from './types';
import { objectBooleans } from './storage';

export function lessonProgress(raw:unknown,lang:Language,meta:LessonMeta){
 const checked=objectBooleans(raw);
 const required=meta.sections?.length?meta.sections:(lang==='english'?['conversation','expressions','reading','vocabulary']:['grammar','examples','kana','words']);
 const count=required.filter(section=>checked[`${lang}:${meta.id}:lang-${section}`]).length;
 return {count,total:required.length,done:required.length>0&&count===required.length};
}
export function reviewId(item:{language:Language;from_lesson:number;term:string;due_date:string}){
 return [item.language,item.from_lesson,item.term,item.due_date].join(':');
}
export function seoulDate(){return new Date(Date.now()+9*60*60*1000).toISOString().slice(0,10);}
