import { useEffect, useMemo, useRef, useState } from 'react';
import { Link, useParams, useSearchParams } from 'react-router-dom';
import { ArrowRight, BookOpen, Volume2, Square, RotateCcw, Check } from 'lucide-react';
import { useData, useResource, lessonSchema } from '@/lib/data';
import { keys, objectBooleans, useStored } from '@/lib/storage';
import { lessonProgress, reviewId, seoulDate } from '@/lib/language-progress';
import { dateLabel } from '@/lib/utils';
import { Button } from '@/components/ui/button';
import { Tabs, TabsList, TabsTrigger, TabsContent } from '@/components/ui/tabs';
import { PageHeader, Panel, Empty, Loading, ErrorView, NextPrevious } from '@/components/Common';
import type { Language, Lesson, Example } from '@/lib/types';

function useSpeech(){
 const [notice,setNotice]=useState(''),serial=useRef(0);
 useEffect(()=>()=>{serial.current++;if('speechSynthesis'in window)window.speechSynthesis.cancel();},[]);
 function stop(){serial.current++;if('speechSynthesis'in window)window.speechSynthesis.cancel();setNotice('');}
 function play(text:string,lang:Language,rate=1,repeat=1){
  stop();if(!('speechSynthesis'in window)||!('SpeechSynthesisUtterance'in window)){setNotice('이 브라우저에서 음성 합성을 사용할 수 없습니다.');return;}
  const token=serial.current;let count=0;const next=()=>{if(token!==serial.current||count++>=repeat){setNotice('');return;}const utterance=new SpeechSynthesisUtterance(text);utterance.lang=lang==='english'?'en-US':'ja-JP';utterance.rate=rate;utterance.onend=next;utterance.onerror=()=>setNotice('음성 재생을 확인해 주세요. 기기의 음성 지원에 따라 달라질 수 있습니다.');setNotice('재생 중');window.speechSynthesis.speak(utterance);};next();
 }
 return {play,stop,notice};
}
function ExampleCard({example:x,lang,index}:{example:Example;lang:Language;index:number}){
 const [pron,setPron]=useState(false),[meaning,setMeaning]=useState(false),speech=useSpeech();
 const sentence=x.en||x.jp||'';
 return <article className="example-card"><span className="example-number">{String(index+1).padStart(2,'0')}</span><p className="foreign-text" lang={lang==='english'?'en':'ja'}>{sentence}</p><div className="example-tools"><Button variant="secondary" size="sm" aria-label={`문장 ${index+1} 듣기`} onClick={()=>speech.play(sentence,lang)}><Volume2/>듣기</Button><Button variant="ghost" size="sm" onClick={()=>speech.play(sentence,lang,.78)}>느리게</Button><Button variant="ghost" size="sm" onClick={()=>speech.play(sentence,lang,.9,3)}><RotateCcw/>3회</Button>{speech.notice==='재생 중'&&<Button variant="ghost" size="sm" onClick={speech.stop} aria-label="음성 중지"><Square/></Button>}<div className="flex-1"/><Button variant="ghost" size="sm" aria-expanded={pron} onClick={()=>setPron(v=>!v)}>발음</Button><Button variant="ghost" size="sm" aria-expanded={meaning} onClick={()=>setMeaning(v=>!v)}>해석</Button></div>{pron&&<p className="pronunciation">{x.pron||'등록된 발음이 없습니다.'}</p>}{meaning&&<p className="translation">{x.ko||'등록된 해석이 없습니다.'}</p>}{speech.notice&&<p role="status" className="muted text-xs">{speech.notice}</p>}</article>;
}
export default function LanguageIndex(){
 const {catalog}=useData(),[params,setParams]=useSearchParams(),[stored]=useStored<unknown>(keys.language,{}),[reviewStored]=useStored<unknown>(keys.review,{}),reviewDone=objectBooleans(reviewStored),language=params.get('language')==='japanese'?'japanese':'english',view=params.get('view'),query=params.get('q')||'',date=params.get('date')||'';
 const items=catalog.languages[language].filter(m=>(!date||m.date===date)&&m.title.toLocaleLowerCase().includes(query.toLocaleLowerCase())).slice().reverse();
 const today=seoulDate(),todayReviews=catalog.review.filter(r=>r.due_date===today&&!reviewDone[reviewId(r)]).length,overdueReviews=catalog.review.filter(r=>r.due_date<today&&!reviewDone[reviewId(r)]).length;
 const latest=catalog.languages[language].at(-1);function update(values:Record<string,string>){const p=new URLSearchParams(params);Object.entries(values).forEach(([k,v])=>v?p.set(k,v):p.delete(k));setParams(p,{replace:true});}
 return <div className="page language-page"><PageHeader eyebrow="LANGUAGE LIBRARY" title="다른 언어로, 더 넓은 세상." description="원문을 읽고, 소리로 듣고, 나만의 속도로 익혀보세요."/><div className="language-overview">{(['english','japanese'] as const).map(lang=>{const last=catalog.languages[lang].at(-1);return <button key={lang} className={'language-card '+(lang===language?'selected':'')} onClick={()=>update({language:lang,date:'',view:''})}><span className="language-glyph">{lang==='english'?'Aa':'あ'}</span><div><h2>{lang==='english'?'English':'日本語'}</h2><p>{catalog.languages[lang].length}개 레슨 · 최근 {last?.date}</p></div><ArrowRight size={19}/></button>;})}</div><div className="section-heading"><h2>{view==='review'?'복습 센터':'레슨 라이브러리'}</h2><Button variant="outline" size="sm" onClick={()=>update({view:view==='review'?'':'review',review:''})}>{view==='review'?'레슨 목록':`오늘 복습 ${todayReviews}개 · 밀림 ${overdueReviews}개`}</Button></div>{view==='review'?<Review/>:<><div className="library-toolbar"><label><span className="sr-only">레슨 검색</span><input value={query} placeholder="레슨 검색" onChange={e=>update({q:e.target.value})}/></label><select aria-label="학습 날짜" value={date} onChange={e=>update({date:e.target.value})}><option value="">전체 날짜</option>{catalog.languages[language].slice().reverse().map(m=><option key={m.id} value={m.date}>{m.date}</option>)}</select>{latest&&<Button asChild size="sm"><Link to={`/language-study/${language}/${latest.id}/`}>최근 레슨 <ArrowRight/></Link></Button>}</div><div className="lesson-list">{items.map(m=>{const p=lessonProgress(stored,language,m);return <Link key={m.id} to={`/language-study/${language}/${m.id}/`}><span className="lesson-index-number">{String(m.id).padStart(2,'0')}</span><div><span className="eyebrow">LESSON {m.id}</span><h3>{m.title}</h3><time>{dateLabel(m.date)}</time></div><span className={'lesson-status '+(p.done?'is-done':'')}>{p.done?<><Check size={15}/>완료</>:p.count?`학습 중 ${p.count}/${p.total}`:'미시작'}</span><ArrowRight size={18}/></Link>;})}</div>{!items.length&&<Empty/>}</>}</div>;
}
function Review(){
 const {catalog}=useData(),[params,setParams]=useSearchParams(),[value,set]=useStored<unknown>(keys.review,{}),state=objectBooleans(value);
 const today=seoulDate(),mode=params.get('review')||'today',due=catalog.review.filter(r=>r.due_date<=today);
 const todayItems=due.filter(r=>r.due_date===today&&!state[reviewId(r)]);
 const overdueItems=due.filter(r=>r.due_date<today&&!state[reviewId(r)]);
 const doneItems=due.filter(r=>state[reviewId(r)]);
 const rows=mode==='overdue'?overdueItems:mode==='done'?doneItems:mode==='all'?due:todayItems;
 function show(next:string){const p=new URLSearchParams(params);if(next==='today')p.delete('review');else p.set('review',next);setParams(p,{replace:true});}
 return <div><div className="review-filters" role="group" aria-label="복습 상태 필터">
 {([{id:'today',name:'오늘',count:todayItems.length},{id:'overdue',name:'밀린 복습',count:overdueItems.length},{id:'done',name:'완료',count:doneItems.length},{id:'all',name:'전체 배정',count:due.length}] as const).map(x=><Button key={x.id} variant={mode===x.id?'secondary':'outline'} size="sm" onClick={()=>show(x.id)}>{x.name} {x.count}</Button>)}
 </div><p className="muted text-xs mb-5">복습 배정일은 한국시간 기준입니다. 새 레슨의 1·3·7·14·30일 일정은 자동 등록됩니다. 체크한 기록은 저장 설정에 따라 동기화됩니다.</p>
 {rows.length?<div className="review-grid">{rows.map(r=>{const id=reviewId(r);return <article className="panel" key={id}><span className="eyebrow">{r.language==='english'?'ENGLISH':'JAPANESE'} · LESSON {r.from_lesson}</span><h3>{r.term}</h3><p className="muted text-xs">복습 배정일 {r.due_date}</p><div className="flex items-center justify-between gap-2 mt-4"><Link className="text-link" to={`/language-study/${r.language}/${r.from_lesson}/`}>원래 레슨</Link><Button variant={state[id]?'secondary':'outline'} size="sm" aria-pressed={!!state[id]} onClick={()=>set({...state,[id]:!state[id]})}><Check/>{state[id]?'완료 취소':'복습 완료'}</Button></div></article>;})}</div>:<Empty title="해당하는 복습 항목이 없습니다." description="다른 복습 상태를 선택하거나 새 레슨을 확인해 주세요."/>}</div>;
}
export function LanguageLesson(){
 const {catalog}=useData(),{language,id}=useParams(),lang=language==='japanese'?'japanese':'english',meta=catalog.languages[lang].find(m=>m.id===Number(id));
 const resource=useResource(meta?'language-study/'+meta.file:null,lessonSchema);
 if(!meta||!['english','japanese'].includes(language||''))return <Empty title="레슨을 찾을 수 없습니다."/>;
 if(resource.error)return <ErrorView message={resource.error}/>;if(!resource.data)return <Loading/>;
 return <LessonView key={lang+id} lang={lang} d={resource.data}/>;
}
function LessonView({lang,d}:{lang:Language;d:Lesson}){
 const {catalog}=useData(),[value,set]=useStored<unknown>(keys.language,{}),state=objectBooleans(value),[params,setParams]=useSearchParams();
 const examples=(items:Example[])=><div className="example-list">{items.map((x,i)=><ExampleCard example={x} lang={lang} index={i} key={i}/>)}</div>;
 const words=(rows:{text:string;meaning:string}[])=><dl className="vocabulary">{rows.map((x,i)=><div key={i}><dt>{x.text}</dt><dd>{x.meaning}</dd></div>)}</dl>;
 const groups: {id:string;title:string;node:React.ReactNode}[]=lang==='english'?[
  {id:'conversation',title:'회화',node:<><h2>Conversation</h2><p className="muted">{d.source_work} · 학습용 재구성</p>{examples(d.conversation)}</>},
  {id:'expressions',title:'핵심 표현',node:<><h2>Expressions</h2>{words(d.expressions)}{d.speaking_points.map((x,i)=><Panel key={i} title={x.title}><p>{x.explanation}</p></Panel>)}</>},
  {id:'reading',title:'읽기',node:<><h2>Reading</h2>{d.article&&<div className="reading-source"><h3>{d.article.title}</h3><p>{d.article.publisher} · {d.article.date}</p>{/^https?:\/\//.test(d.article.url)&&<a href={d.article.url} target="_blank" rel="noopener noreferrer">원문 기사 ↗</a>}</div>}{examples(d.reading)}</>},
  {id:'vocabulary',title:'어휘',node:<><h2>Vocabulary</h2>{words(d.vocabulary.map(x=>({text:x.term,meaning:x.meaning})))}{words(d.useful_expressions)}</>},
  ...(d.reading_check.length?[{id:'check',title:'이해 확인',node:<><h2>Reading Check</h2>{d.reading_check.map((x,i)=><details className="disclosure" key={i}><summary>{i+1}. {x.q}</summary><p>{x.a}</p></details>)}</>}]:[])
 ]:[
  {id:'grammar',title:'문법',node:<><span className="eyebrow">오늘의 문법</span><div className="grammar-block"><h2>{d.grammar?.title}</h2><p>{d.grammar?.explanation}</p></div><p className="muted">예문 탭에서 오늘의 문법을 문장으로 연습해 보세요.</p></>},
  {id:'examples',title:'예문',node:<><h2>문장으로 연습하기</h2>{examples(d.examples)}</>},
  {id:'kana',title:'문자',node:<><h2>오늘의 문자</h2><div className="kana-grid">{d.kana.map((x,i)=><Kana key={i} value={x}/>)}</div></>},
  {id:'words',title:'단어',node:<><h2>Vocabulary</h2>{examples(d.words)}</>}
 ];
 if(d.review_reuse.length)groups.push({id:'review',title:'이전 표현',node:<><h2>이전 레슨에서 다시 보기</h2>{words(d.review_reuse.map(x=>({text:x.text||x.term||'',meaning:x.meaning||''})))}</>});
 const active=params.get('tab')||groups[0].id,current=groups.some(g=>g.id===active)?active:groups[0].id;const storageKey=(id:string)=>`${lang}:${d.id}:lang-${id}`;
 const complete=groups.filter(g=>state[storageKey(g.id)]).length;
 const items=useMemo(()=>catalog.languages[lang].map(m=>({route:`/language-study/${lang}/${m.id}/`,title:`Lesson ${m.id} · ${m.title}`})),[catalog,lang]);
 return <div className="page language-detail"><div className="lesson-breadcrumb"><Link to={'/language-study/?language='+lang}>{lang==='english'?'English':'日本語'} 레슨</Link><span>/</span><span>Lesson {String(d.id).padStart(2,'0')}</span><time>{dateLabel(d.date)}</time></div><div className="lesson-workspace"><article className="lesson-document"><header className="lesson-heading"><p className="eyebrow">{lang==='english'?'ENGLISH':'JAPANESE'} / LESSON {d.id}</p><h1>{d.title}</h1><div className="flex gap-2"><span className="badge">{d.grammar_level||d.conversation_level}</span>{d.reading_level&&<span className="badge">READING {d.reading_level}</span>}</div></header><Tabs value={current} onValueChange={tab=>setParams({tab},{replace:true})}><TabsList aria-label="레슨 내용">{groups.map(g=><TabsTrigger value={g.id} key={g.id}>{g.title}</TabsTrigger>)}</TabsList>{groups.map(g=><TabsContent value={g.id} key={g.id}><div className="lesson-prose">{g.node}</div></TabsContent>)}</Tabs><NextPrevious items={items} current={`/language-study/${lang}/${d.id}/`}/></article><aside className="lesson-tools"><Panel title="학습 체크"><div className="lesson-progress-number">{complete}<span> / {groups.length}</span></div>{complete===groups.length&&<p className="lesson-complete"><Check size={15}/> 학습 완료</p>}<div className="progress-track"><i style={{width:complete/groups.length*100+'%'}}/></div><p className="muted text-xs mt-3">직접 완료한 단계에 표시해 주세요.</p><div className="lesson-checks">{groups.map(g=><label className="check-row" key={g.id}><input type="checkbox" checked={!!state[storageKey(g.id)]} onChange={e=>set({...state,[storageKey(g.id)]:e.target.checked})}/>{g.title}</label>)}</div></Panel><Panel title="이번 레슨"><nav className="toc" aria-label="레슨 목차">{groups.map(g=><button key={g.id} aria-current={g.id===current?'step':undefined} onClick={()=>setParams({tab:g.id},{replace:true})}>{g.title}<ArrowRight size={13}/></button>)}</nav></Panel><div className="study-note"><BookOpen size={20}/><p>먼저 원문을 읽고,<br/>발음과 해석은 필요할 때.</p><small>학습 기록은 로컬에 보존되며, 동기화 연결 시 기기 간 공유됩니다.</small></div></aside></div></div>;
}
function Kana({value}:{value:Lesson['kana'][number]}){
 const speech=useSpeech(),x=typeof value==='string'?{jp:value,pron:'',romaji:''}:value;return <div className="kana-tile"><strong lang="ja">{x.jp}</strong><p>{x.pron}</p><small>{x.romaji}</small><Button variant="ghost" size="sm" aria-label={x.jp+' 듣기'} onClick={()=>speech.play(x.jp,'japanese')}><Volume2/>듣기</Button>{speech.notice&&<small role="status">{speech.notice}</small>}</div>;
}
