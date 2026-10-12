import { useEffect, useRef, useState } from 'react';
import { Link, useLocation, useSearchParams } from 'react-router-dom';
import { ArrowRight, Code2, Save, TerminalSquare } from 'lucide-react';
import { useData, useResource, documentSchema } from '@/lib/data';
import { useStored } from '@/lib/storage';
import { cleanTitle, dateLabel } from '@/lib/utils';
import { Blocks } from '@/components/Content';
import { Button } from '@/components/ui/button';
import { Tabs, TabsList, TabsTrigger, TabsContent } from '@/components/ui/tabs';
import { PageHeader, Panel, Empty, Loading, ErrorView, ReadCheck, NextPrevious } from '@/components/Common';
import { DocumentLibrary } from '@/components/DocumentLibrary';
import type { Document } from '@/lib/types';
const tabNames:Record<string,string>={problem:'문제',hints:'힌트',solution:'풀이·검증',more:'확장·다음',results:'결과물',compare:'크기 비교',process:'실험 과정',evaluation:'평가',files:'자료',document:'본문'};
export function DocumentIndex({kind}:{kind:'thinking'|'opensource'}){
 const {catalog}=useData();const docs=catalog.documents.filter(d=>d.kind===kind).sort((a,b)=>String(a.number).localeCompare(String(b.number),undefined,{numeric:true}));const latest=docs.at(-1);
 return <div className="page"><PageHeader eyebrow={kind==='thinking'?'THINKING LAB / C++11':'OPEN SOURCE / ARCHIVE'} title={kind==='thinking'?'매일 한 문제, 생각의 깊이.':'코드에서 배우는 설계.'} description={kind==='thinking'?'문제를 이해하고, 직접 작성하고, 반례로 검증합니다.':'정기 발행은 중단되었습니다. 기존 분석 자료를 보관합니다.'}/>{!latest?<Empty/>:<><div className="challenge-grid"><Panel className="challenge-card"><span className="eyebrow">{kind==='thinking'?'LATEST CHALLENGE':'LATEST ARTICLE'}</span><span className="challenge-number">{latest.number}</span><h2>{cleanTitle(latest.title)}</h2><p>{latest.subtitle}</p><div className="flex flex-wrap gap-2"><span className="badge">{dateLabel(latest.date)}</span><span className="badge">{kind==='thinking'?'단계별 힌트':'소스 코드 분석'}</span></div><Button asChild><Link to={latest.route}>학습 시작 <ArrowRight size={16}/></Link></Button></Panel><Panel title={kind==='thinking'?'학습 순서':'분석 회차'}><ol className="roadmap">{docs.map(d=><li key={d.id}><Link to={d.route}><span>{d.number}</span><strong>{cleanTitle(d.title)}</strong><small>{dateLabel(d.date)}</small></Link></li>)}</ol><p className="muted text-xs">등록된 {docs.length}개 문서 · 학습일과 실제 게시일은 각 문서에 구분되어 있습니다.</p></Panel></div><DocumentLibrary docs={docs} thinking={kind==='thinking'}/></>}</div>;
}
function CodeEditor({doc}:{doc:Document}){
 const [saved,setSaved]=useStored(doc.memoKey||'atlas-code-'+doc.id,'',true),[draft,setDraft]=useState(saved),[notice,setNotice]=useState('브라우저에 안전하게 보관합니다. 동기화를 연결하면 다른 기기에서도 사용할 수 있습니다.'),field=useRef<HTMLTextAreaElement>(null);
 const dirty=draft!==saved;
 const previousSaved=useRef(saved);
 useEffect(()=>{if(draft===previousSaved.current&&draft!==saved)setDraft(saved);previousSaved.current=saved;},[draft,saved]);
 useEffect(()=>{const handler=(e:BeforeUnloadEvent)=>{if(dirty){e.preventDefault();e.returnValue='';}};window.addEventListener('beforeunload',handler);return()=>window.removeEventListener('beforeunload',handler);},[dirty]);
 return <section className="panel code-editor" id="write"><div className="editor-heading"><Code2 size={16}/><strong>내 풀이</strong><span>C++11</span></div><label htmlFor="code-draft" className="sr-only">C++11 풀이 작성</label><textarea ref={field} id="code-draft" spellCheck={false} value={draft} placeholder={doc.starter||'// 여기에 풀이를 작성하세요.'} onChange={e=>setDraft(e.target.value)} onKeyDown={e=>{if((e.ctrlKey||e.metaKey)&&e.key==='s'){e.preventDefault();setNotice(setSaved(draft)?'저장했습니다.':'저장 실패 · 코드를 따로 복사해 주세요.');}}}/><div className="editor-bottom"><Button size="sm" onClick={()=>setNotice(setSaved(draft)?'저장했습니다.':'저장 실패 · 코드를 따로 복사해 주세요.')}><Save/>풀이 저장</Button><span className="muted text-xs">{dirty?'저장하지 않은 변경':'저장 상태 유지'}</span></div><p role="status" className="muted text-xs">{notice}</p><div className="editor-note"><TerminalSquare size={14}/>코드 실행 기능은 없습니다.</div></section>;
}
export default function DocumentPage(){
 const {catalog}=useData(),location=useLocation();const route=location.pathname.replace(/\/?$/,'/');const meta=catalog.documents.find(d=>d.route===route);const resource=useResource(meta?.file||null,documentSchema);
 if(!meta)return <div className="page"><Empty title="문서를 찾을 수 없습니다." description="왼쪽 목록에서 다른 문서를 선택해 주세요."/></div>;
 if(resource.error)return <ErrorView message={resource.error}/>;if(!resource.data)return <Loading/>;
 return <DocumentView key={meta.id} doc={resource.data}/>;
}
function DocumentView({doc}:{doc:Document}){
 const {catalog}=useData(),location=useLocation(),[params,setParams]=useSearchParams();const tabs=[...new Set(doc.sections.map(s=>s.tab))];const hash=decodeURIComponent(location.hash.slice(1));const selected=params.get('tab')||doc.sections.find(s=>s.id===hash)?.tab||tabs[0];const current=tabs.includes(selected)?selected:tabs[0];
 const related=catalog.documents.filter(d=>d.kind===doc.kind).sort((a,b)=>String(a.number||a.title).localeCompare(String(b.number||b.title),undefined,{numeric:true}));
 return <div className={'page document-page '+doc.kind+'-document'}><div className="document-workspace"><div className="document-main"><header className="document-heading"><div className="eyebrow">{doc.kind==='thinking'?'THINKING LAB':doc.kind==='opensource'?'OPEN SOURCE ARCHIVE':'AI RESOURCE LAB'}{doc.number&&' / '+doc.number}</div><h1>{cleanTitle(doc.title)}</h1><p className="page-description">{doc.subtitle}</p><div className="flex flex-wrap gap-2"><span className="badge">{dateLabel(doc.date)}</span>{doc.number&&<span className="badge">C++11</span>}<span className="badge">{doc.kind==='thinking'?'문제 → 힌트 → 검증':'기록 · 자료 · 검증'}</span></div></header><Tabs value={current} onValueChange={tab=>setParams({tab},{replace:true})}><TabsList aria-label="문서 내용">{tabs.map(tab=><TabsTrigger value={tab} key={tab}>{tabNames[tab]||tab}</TabsTrigger>)}</TabsList>{tabs.map(tab=><TabsContent value={tab} key={tab}><div className="prose">{doc.sections.filter(s=>s.tab===tab).map(s=><section key={s.id} id={s.id}><Blocks blocks={s.blocks}/></section>)}</div></TabsContent>)}</Tabs><NextPrevious items={related} current={doc.route}/></div><aside className="document-tools">{doc.memoKey&&<CodeEditor key={doc.id} doc={doc}/>}<Panel title="이 문서의 목차"><nav className="toc" aria-label="문서 목차">{doc.sections.map(s=><button key={s.id} aria-current={s.tab===current?'step':undefined} onClick={()=>{setParams({tab:s.tab},{replace:true});}}>{s.title}</button>)}</nav></Panel><Panel title="내 학습 기록"><ReadCheck path={doc.route}/><p className="muted text-xs mt-3">체크와 메모는 로컬에 저장하고, 동기화를 연결하면 다른 기기와 공유합니다.</p></Panel>{doc.kind==='asset'&&<Panel title="실험 자료"><Link className="text-link" to={doc.route+'report/'}>연구 보고서 <ArrowRight size={14}/></Link><Link className="text-link" to={doc.route+'prompts/prompts-v1/'}>프롬프트 원문 <ArrowRight size={14}/></Link></Panel>}</aside></div></div>;
}
