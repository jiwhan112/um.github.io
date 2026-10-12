import { Component, Suspense, lazy, useEffect, type ErrorInfo, type ReactNode } from 'react';
import { BrowserRouter, Routes, Route, useLocation, useNavigate, Link } from 'react-router-dom';
import { DataProvider } from '@/lib/data';
import { Shell } from '@/components/Shell';
import { Loading, Empty } from '@/components/Common';
import { base } from '@/lib/utils';
const Dashboard=lazy(()=>import('@/features/Dashboard'));
const Briefing=lazy(()=>import('@/features/Briefing'));
const LanguageIndex=lazy(()=>import('@/features/Language'));
const LanguageLesson=lazy(()=>import('@/features/Language').then(m=>({default:m.LanguageLesson})));
const DocumentIndex=lazy(()=>import('@/features/Documents').then(m=>({default:m.DocumentIndex})));
const DocumentPage=lazy(()=>import('@/features/Documents'));
const Assets=lazy(()=>import('@/features/Assets'));
const Archive=lazy(()=>import('@/features/Archive'));
function LegacyLinks(){const loc=useLocation(),navigate=useNavigate();useEffect(()=>{
 if(loc.pathname.endsWith('/index.html')){navigate(loc.pathname.replace(/index\.html$/,'')+loc.search+loc.hash,{replace:true});return;}
 if(/^\/language-study\/?$/.test(loc.pathname)&&loc.hash){const m=loc.hash.match(/^#(english|japanese)(?:\/(\d+))?$/);if(m){navigate(m[2]?`/language-study/${m[1]}/${Number(m[2])}/`:`/language-study/?language=${m[1]}`,{replace:true});}else if(loc.hash==='#review')navigate('/language-study/?view=review',{replace:true});else if(loc.hash==='#home')navigate('/language-study/',{replace:true});}
 },[loc,navigate]);return null;}
class Boundary extends Component<{children:ReactNode},{error:boolean}>{state={error:false};static getDerivedStateFromError(){return {error:true};}componentDidCatch(e:Error,i:ErrorInfo){console.error('Atlas render error',e,i.componentStack);}render(){return this.state.error?<div className="boot-screen"><h1>화면을 표시하지 못했습니다.</h1><p>저장한 메모와 북마크는 삭제되지 않았습니다.</p><button onClick={()=>location.reload()}>다시 불러오기</button></div>:this.props.children;}}
export default function App(){return <Boundary><BrowserRouter basename={base.replace(/\/$/,'')}><DataProvider><LegacyLinks/><Shell><Suspense fallback={<Loading/>}><Routes><Route path="/" element={<Dashboard/>}/><Route path="daily-briefing/*" element={<Briefing/>}/><Route path="bookmarks/" element={<Briefing bookmarks/>}/><Route path="archive/" element={<Archive/>}/><Route path="language-study/" element={<LanguageIndex/>}/><Route path="language-study/:language/:id/" element={<LanguageLesson/>}/><Route path="thinking-lab/" element={<DocumentIndex kind="thinking"/>}/><Route path="opensource-lab/" element={<DocumentIndex kind="opensource"/>}/><Route path="ai-asset-lab/" element={<Assets/>}/><Route path="thinking-lab/:slug/*" element={<DocumentPage/>}/><Route path="opensource-lab/:slug/*" element={<DocumentPage/>}/><Route path="ai-asset-lab/*" element={<DocumentPage/>}/><Route path="*" element={<div className="page"><Empty title="페이지를 찾을 수 없습니다."/><Link to="/">홈으로 돌아가기</Link></div>}/></Routes></Suspense></Shell></DataProvider></BrowserRouter></Boundary>;}
