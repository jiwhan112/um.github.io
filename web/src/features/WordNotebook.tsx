import { useMemo, useState } from 'react';
import { Link, useSearchParams } from 'react-router-dom';
import { ArrowLeft, Volume2, Search, BookOpen, ChevronLeft, ChevronRight } from 'lucide-react';
import { useResource } from '@/lib/data';
import { notebookSchema, type NotebookEntry } from '@/lib/word-notebook';
import { useSpeech } from './Language';
import { Button } from '@/components/ui/button';
import { PageHeader, Loading, ErrorView, Empty } from '@/components/Common';
import './word-notebook.css';
const labels={english:'영어 단어 노트',japanese:'일본어 단어 노트',kanji:'자주 만난 한자어'};
type Mode=keyof typeof labels;
export default function WordNotebook(){
 const resource=useResource('data/atlas/word-notebook.json',notebookSchema);
 const [params,setParams]=useSearchParams();
 const mode:Mode=params.get('tab')==='japanese'?'japanese':params.get('tab')==='kanji'?'kanji':'english';
 const query=params.get('q')||'',lesson=params.get('lesson')||'',page=Math.max(1,Math.floor(Number(params.get('page')))||1);
 const data=resource.data;
 const rows=useMemo(()=>{const q=query.trim().toLocaleLowerCase();return (data?.[mode]||[]).filter(entry=>(!lesson||entry.sources.some(s=>String(s.lessonId)===lesson))&&(!q||[entry.term,entry.reading,...entry.meanings,...entry.pronunciations,entry.romaji,...entry.sources.map(s=>s.article?.title||'')].join(' ').toLocaleLowerCase().includes(q)));},[data,mode,query,lesson]);
 function update(values:Record<string,string>){const next=new URLSearchParams(params);Object.entries(values).forEach(([k,v])=>v?next.set(k,v):next.delete(k));setParams(next);}
 const pages=Math.max(1,Math.ceil(rows.length/24)),current=Math.min(page,pages),visible=rows.slice((current-1)*24,current*24);
 if(resource.error)return <ErrorView message={resource.error}/>;
 if(!data)return <Loading/>;
 return <div className="page word-notebook"><Link className="text-link notebook-back" to="/language-study/"><ArrowLeft size={15}/>언어 학습으로</Link><PageHeader eyebrow="MY VOCABULARY · 001—030" title="읽었던 말, 내 단어로." description="새 레슨을 늘리지 않고, 지금까지의 영어·일본어 자료를 한곳에서 다시 익혀보세요."/>
 <Link className="text-link" to="/language-study/foundation/">일본어 기초 전체 교재 →</Link><nav className="notebook-tabs" aria-label="단어 노트 종류">{(Object.keys(labels) as Mode[]).map(key=><Button key={key} variant={mode===key?'secondary':'outline'} aria-current={mode===key?'page':undefined} onClick={()=>update({tab:key,q:'',lesson:'',page:''})}>{labels[key]} <span>{data[key].length}</span></Button>)}</nav>
 <section className="notebook-intro"><BookOpen size={22}/><div><h2>{labels[mode]}</h2><p>{mode==='english'?'각 레슨의 Vocabulary를 모았습니다. 출처는 학습 지문이 참고한 기사이며, 단어가 원기사에 그대로 등장한다는 뜻은 아닙니다.':mode==='japanese'?'기존 교재의 단어와 뜻을 모았습니다. 같은 단어의 다른 뜻과 읽기를 보존하고 원래 레슨으로 연결합니다.':'기존 교재에 나온 한자 포함 단어를 레슨 출현 횟수순으로 모았습니다. 일반 일본어 사용 빈도 순위가 아니며, 단어 안에서 쓰인 읽기를 익힙니다.'}</p></div></section>
 <div className="notebook-toolbar"><label className="notebook-search"><Search size={17}/><span className="sr-only">단어 노트 검색</span><input value={query} onChange={e=>update({q:e.target.value,page:''})} placeholder={mode==='english'?'단어, 뜻, 기사 제목 검색':'단어, 읽기, 뜻 검색'}/></label><label><span className="sr-only">원래 레슨 필터</span><select value={lesson} onChange={e=>update({lesson:e.target.value,page:''})}><option value="">모든 레슨</option>{Array.from({length:30},(_,i)=>i+1).map(id=><option key={id} value={id}>Lesson {String(id).padStart(3,'0')}</option>)}</select></label><span role="status">{rows.length}개 항목</span></div>
 <p className="notebook-help">뜻은 접어 두었습니다. 먼저 떠올린 뒤 펼쳐 보세요. 한국어식 발음은 근사치이며, 듣기는 기기의 영어·일본어 음성을 사용합니다.</p>
 <div className="notebook-grid">{visible.map(entry=><WordCard key={mode+entry.id} entry={entry} mode={mode}/>)}</div>{!rows.length&&<Empty title="찾는 단어가 없습니다." description="검색어나 레슨 필터를 바꿔 보세요."/>}
 <nav className="notebook-pagination" aria-label="단어 노트 페이지"><Button variant="outline" disabled={current===1} onClick={()=>update({page:String(current-1)})}><ChevronLeft/>이전</Button><span>{current} / {pages}</span><Button variant="outline" disabled={current===pages} onClick={()=>update({page:String(current+1)})}>다음<ChevronRight/></Button></nav>
 </div>;
}
function WordCard({entry:e,mode}:{entry:NotebookEntry;mode:Mode}){
 const speech=useSpeech(),[reveal,setReveal]=useState(false);
 return <article className="notebook-card" data-word-id={e.id}><div className="notebook-card-top"><span className="eyebrow">{mode==='kanji'?'KANJI IN WORDS':e.language==='english'?'ENGLISH':'日本語'}</span><span className="notebook-frequency">{new Set(e.sources.map(s=>s.lessonId)).size}개 레슨</span></div><h3 lang={e.language==='english'?'en':'ja'}>{e.term}</h3>{e.reading&&e.reading!==e.term&&<p className="notebook-reading" lang="ja">{e.reading}</p>}{e.pronunciations.length>0&&<p className="pronunciation">{e.pronunciations.join(' · ')}</p>}{!e.pronunciations.length&&<small className="muted">별도 발음 표기 없음 · 듣기로 확인</small>}{e.romaji&&<small className="muted">{e.romaji}</small>}
 <div className="notebook-actions"><Button variant="secondary" size="sm" aria-label={e.term+' 듣기'} onClick={()=>speech.play(e.tts,e.language)}><Volume2/>듣기</Button><Button variant="ghost" size="sm" onClick={()=>speech.play(e.tts,e.language,.78)}>느리게</Button>{speech.notice==='재생 중'&&<Button variant="ghost" size="sm" onClick={speech.stop}>중지</Button>}<Button variant="outline" size="sm" aria-expanded={reveal} onClick={()=>setReveal(v=>!v)}>{reveal?'뜻 접기':'뜻 보기'}</Button></div>{speech.notice&&<p role="status" className="muted text-xs">{speech.notice}</p>}
 {reveal&&<ul className="notebook-meanings">{e.meanings.map((meaning,i)=><li key={i}>{meaning}</li>)}</ul>}
 {e.examples.length>0&&<details className="notebook-examples"><summary>교재 속 예문 {e.examples.length}개</summary>{e.examples.map((x,i)=><div className="notebook-example" key={i}><p lang={e.language==='english'?'en':'ja'}>{x.text}</p><Button variant="ghost" size="sm" aria-label={`${e.term} 예문 ${i+1} 듣기`} onClick={()=>speech.play(x.tts,e.language)}><Volume2/>예문 듣기</Button><details><summary>발음 · 해석</summary><p className="pronunciation">{x.pron||'원본에 별도 발음이 없습니다.'}</p><p>{x.ko}</p></details><Link className="text-link" to={`/language-study/${e.language}/${x.lessonId}/`}>Lesson {x.lessonId}</Link></div>)}</details>}
 <details className="notebook-sources"><summary>원래 레슨 · {e.language==='english'?'기사 출처':'단어 출처'}</summary>{e.sources.map((source,i)=><div className="notebook-source" key={i}><Link to={source.route}>Lesson {String(source.lessonId).padStart(3,'0')} · {source.lessonTitle}</Link><time>{source.date}</time><p>교재 뜻: {source.meaning}</p>{source.pronunciation&&<p>발음: {source.pronunciation}</p>}{source.article?<><span className="eyebrow">학습 지문 기반 기사</span><a href={source.article.url} target="_blank" rel="noopener noreferrer">{source.article.title} ↗</a><p>{source.article.publisher} · {source.article.date}</p></>:e.language==='english'?<p>이 레슨에는 확인 가능한 기사 메타데이터가 없습니다.</p>:null}</div>)}</details>
 </article>;
}
