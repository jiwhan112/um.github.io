import { useState } from 'react';
import { Link } from 'react-router-dom';
import { ArrowUpRight } from 'lucide-react';
import { useData } from '@/lib/data';
import { PageHeader, Empty } from '@/components/Common';
import { cleanTitle } from '@/lib/utils';
export default function Archive(){
 const {catalog,articles}=useData(),[query,setQuery]=useState(''),[category,setCategory]=useState('all');
 const rows=[...catalog.documents.map(d=>({id:d.id,title:d.title,date:d.date,category:d.kind==='thinking'?'사고력':d.kind==='opensource'?'오픈소스':'AI 실험실',to:d.route})),...(['english','japanese'] as const).flatMap(lang=>catalog.languages[lang].map(m=>({id:lang+m.id,title:m.title,date:m.date,category:'언어 학습',to:`/language-study/${lang}/${m.id}/`}))),...articles.map(a=>({id:a.id,title:a.title,date:a.date,category:'브리핑',to:`/daily-briefing/?date=${a.date}&article=${encodeURIComponent(a.id)}`}))].filter(r=>(category==='all'||r.category===category)&&r.title.toLocaleLowerCase().includes(query.toLocaleLowerCase())).sort((a,b)=>b.date.localeCompare(a.date));
 return <div className="page"><PageHeader eyebrow="KNOWLEDGE ARCHIVE" title="쌓인 배움, 다시 찾기." description="날짜와 분야를 넘어 지금까지의 기록을 탐색하세요."/><div className="library-toolbar"><input aria-label="아카이브 검색" value={query} placeholder="제목으로 검색" onChange={e=>setQuery(e.target.value)}/><select value={category} aria-label="아카이브 분류" onChange={e=>setCategory(e.target.value)}><option value="all">모든 분야</option>{['브리핑','언어 학습','사고력','오픈소스','AI 실험실'].map(c=><option key={c}>{c}</option>)}</select><span className="muted text-sm">{rows.length}개 자료</span></div><div className="archive-list">{rows.map(r=><Link key={r.id} to={r.to}><time>{r.date}</time><span className="badge">{r.category}</span><strong>{cleanTitle(r.title)}</strong><ArrowUpRight size={15}/></Link>)}</div>{!rows.length&&<Empty/>}</div>;
}
