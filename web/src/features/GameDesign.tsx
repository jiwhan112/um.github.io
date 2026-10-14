import { useState } from 'react';
import { Link } from 'react-router-dom';
import { ArrowRight, Bookmark, BookOpen, CheckCircle2, PlayCircle } from 'lucide-react';
import { useData } from '@/lib/data';
import { keys, objectBooleans, useStored } from '@/lib/storage';
import { cleanTitle } from '@/lib/utils';
import { PageHeader, Panel, Empty } from '@/components/Common';
import { DocumentLibrary } from '@/components/DocumentLibrary';
import { Button } from '@/components/ui/button';

const targetCount=12;
export function GameDesignBookmark({route}:{route:string}){
 const [value,setValue]=useStored<unknown>(keys.gameDesignBookmarks,[]);
 const saved=Array.isArray(value)?value.filter((v):v is string=>typeof v==='string'):[];
 const active=saved.includes(route);
 return <Button size="sm" variant={active?'secondary':'outline'} aria-pressed={active} onClick={()=>setValue(active?saved.filter(v=>v!==route):[...saved,route])}><Bookmark size={15}/>{active?'북마크됨':'북마크 저장'}</Button>;
}
export default function GameDesignIndex(){
 const {catalog}=useData();
 const docs=catalog.documents.filter(d=>d.kind==='game-design').sort((a,b)=>String(a.number).localeCompare(String(b.number),undefined,{numeric:true}));
 const [reading]=useStored<unknown>(keys.reading,{});
 const [bookmarks]=useStored<unknown>(keys.gameDesignBookmarks,[]);
 const [mode,setMode]=useState<'all'|'saved'|'unread'>('all');
 const read=objectBooleans(reading);
 const saved=Array.isArray(bookmarks)?bookmarks.filter((v):v is string=>typeof v==='string'):[];
 const done=docs.filter(d=>read[d.route.replace(/^\//,'')]).length;
 const next=docs.find(d=>!read[d.route.replace(/^\//,'')])||docs[0];
 const visible=docs.filter(d=>mode==='all'||(mode==='saved'?saved.includes(d.route):!read[d.route.replace(/^\//,'')]));
 return <div className="page game-design-index">
  <PageHeader eyebrow="GAME DESIGN LAB / MASAHIRO SAKURAI" title="게임 기획 연구실" description="사쿠라이 마사히로의 게임 설계 강의를 한국어로 풀고, C++ 게임 프로그래머의 구현 관점으로 확장합니다."/>
  <div className="challenge-grid">
   <Panel className="challenge-card">
    <span className="eyebrow">NEXT LESSON / 학습 이어가기</span>
    <span className="challenge-number">{next?.number||'01'}</span>
    <h2>{next?cleanTitle(next.title):'첫 강의를 준비하고 있습니다.'}</h2>
    <p>{next?.subtitle||'게임 기획의 핵심 원리를 실전 구현과 연결해 학습합니다.'}</p>
    <div className="flex flex-wrap gap-2"><span className="badge">공개된 강의 {docs.length} / {targetCount}</span><span className="badge">오래된 영상부터 순차 학습</span></div>
    {next?<Button asChild><Link to={next.route}>강의 열기 <ArrowRight size={16}/></Link></Button>:<Empty title="첫 콘텐츠 준비 중"/>}
   </Panel>
   <Panel title="내 학습 진도">
    <div className="metric-row"><span>학습 완료</span><strong>{done} / {targetCount}</strong></div>
    <div className="progress-track"><i style={{width:(done/targetCount*100)+'%'}}/></div>
    <p className="muted text-xs mt-3">완료 기록은 기존 학습 기록에 저장되고, GitHub 로그인 후 Supabase와 동기화됩니다. 북마크도 같은 방식으로 보존합니다.</p>
    <div className="mt-5"><a className="text-link" href="https://www.youtube.com/@sora_sakurai_en/videos" target="_blank" rel="noopener noreferrer"><PlayCircle size={16}/>원본 YouTube 채널 열기</a></div>
   </Panel>
  </div>
  <div className="section-heading"><h2>영상별 학습 자료</h2><span className="muted text-xs">원본 공개일 오름차순</span></div>
  <div className="flex flex-wrap gap-2 items-center mb-4" role="group" aria-label="학습 상태 필터">
   <Button size="sm" variant={mode==='all'?'secondary':'outline'} onClick={()=>setMode('all')}><BookOpen size={14}/>전체 {docs.length}</Button>
   <Button size="sm" variant={mode==='unread'?'secondary':'outline'} onClick={()=>setMode('unread')}><CheckCircle2 size={14}/>미완료 {docs.length-done}</Button>
   <Button size="sm" variant={mode==='saved'?'secondary':'outline'} onClick={()=>setMode('saved')}><Bookmark size={14}/>북마크 {docs.filter(d=>saved.includes(d.route)).length}</Button>
  </div>
  {docs.length?<DocumentLibrary docs={visible} thinking={false}/>:<Empty title="아직 공개된 강의가 없습니다."/>}
 </div>;
}
