import { useMemo, useState, type ComponentType } from 'react';
import { Link } from 'react-router-dom';
import { ArrowRight, BookOpen, Braces, CheckCircle2, ChevronRight, Code2, Cpu, FileText, Folder, FolderOpen, Layers3, ListTree, Network, Search, Wrench, X } from 'lucide-react';
import { useData } from '@/lib/data';
import { DocumentLibrary } from '@/components/DocumentLibrary';
import { normalizeSearch } from '@/lib/document-library';
import { compareKnowledgeDocuments } from '@/lib/knowledge-order';
import type { DocumentMeta } from '@/lib/types';

type Phase = 'legacy' | 'new' | 'replan' | 'keep';
type Topic = { id: string; title: string; description: string; phase: Phase; category?: string };
type Area = { id: string; title: string; subtitle: string; icon: ComponentType<{size?:number; className?:string}>; topics: Topic[] };
type VisibleTopic = Topic & { documents: DocumentMeta[] };
type VisibleArea = Omit<Area, 'topics'> & { topics: VisibleTopic[]; count: number };
const phaseLabels: Record<Phase,string> = {
  legacy: '기존 자료 기준', new: '신규 집필 예정', replan: '집필 시 설계', keep: '기존 구성 유지'
};
// This is a long-term curriculum outline, NOT a declaration of migration or publication completion.
const areas: Area[] = [
  {id:'cpp',title:'C / C++',subtitle:'언어 원리부터 최신 C++까지 단계적으로 학습',icon:Braces,topics:[
    {id:'c',title:'C 언어',description:'C 언어 기본·포인터·메모리',phase:'legacy',category:'C 언어'},
    {id:'cpp-core',title:'C++ 기초·심화',description:'빌드·타입·함수·메모리에서 객체·STL·RAII까지',phase:'legacy',category:'C++ 기초·심화'},
    {id:'cpp-modern',title:'Modern C++',description:'C++11 이후 언어·표준 라이브러리의 버전별 변화',phase:'new'},
    {id:'cpp-effective',title:'Effective C++',description:'올바른 자원 관리와 C++ 설계 원칙',phase:'legacy',category:'Effective C++'},
    {id:'cpp-effective-modern',title:'Effective Modern C++',description:'Scott Meyers 저서 중심의 현대 C++ 관용구',phase:'new'}
  ]},
  {id:'windows',title:'Windows·그래픽스 프로그래밍',subtitle:'Windows SDK 기반 게임 클라이언트 · DirectX 구현',icon:Code2,topics:[
    {id:'winapi',title:'WinAPI',description:'메시지 루프·윈도우·입력·시스템 API',phase:'legacy',category:'WinAPI'},
    {id:'game-math',title:'게임 수학',description:'벡터·행렬·변환·충돌 수학',phase:'legacy',category:'게임 수학'},
    {id:'dx9',title:'DirectX 9',description:'기존 DX9 엔진과 렌더링 파이프라인',phase:'legacy',category:'DirectX 9'},
    {id:'dx11',title:'DirectX 11',description:'자원·렌더링 파이프라인·엔진 기능',phase:'legacy',category:'DirectX 11'},
    {id:'dx12',title:'DirectX 12',description:'명시적 GPU 자원·명령 기록·동기화 (추후 집필)',phase:'new'}
  ]},
  {id:'graphics',title:'그래픽스 기술',subtitle:'프레임워크 구현과 분리해 개별 기법을 깊게 연구',icon:Cpu,topics:[
    {id:'graphics-tech',title:'기술 연구 주제',description:'셰이더·그림자·Navigation Mesh·IK 등 — 실제 집필 전 세부 목차 재설계',phase:'replan'}
  ]},
  {id:'cs',title:'컴퓨터 과학',subtitle:'컴퓨터 구조·운영체제·네트워크 기초',icon:Network,topics:[
    {id:'computer',title:'컴퓨터 구조',description:'캐시·CPU·메모리 계층',phase:'keep'},
    {id:'os',title:'운영체제',description:'스레드·가상 메모리·동기화',phase:'keep'},
    {id:'network',title:'네트워크',description:'TCP/UDP·패킷·프로토콜',phase:'keep'}
  ]},
  {id:'algorithm',title:'자료구조 / 알고리즘',subtitle:'세부 학습 과정은 집필 시작 시 재설계',icon:Layers3,topics:[
    {id:'algorithm-plan',title:'학습 순서·단원 구성 재계획',description:'자료구조·탐색·알고리즘·문제풀이의 세부 구성을 작성 전에 확정',phase:'replan'}
  ]},
  {id:'engine',title:'게임 엔진 개발',subtitle:'기존 Notion 설계 패턴·Unity 노트를 이관하지 않고 새로 집필',icon:Wrench,topics:[
    {id:'engine-systems',title:'엔진 파츠·시스템 설계',description:'새로운 게임 엔진 학습 교재로 기획 · 세부 목차는 작성 시 확정',phase:'new'}
  ]},
  {id:'python',title:'Python·개발 도구',subtitle:'기존 Notion Python 문서 대신 새 교육 과정으로 집필',icon:BookOpen,topics:[
    {id:'python-new',title:'Python 파츠·자동화',description:'새 Python 학습·도구 개발 교재를 독립 집필',phase:'new'}
  ]}
];


export default function Knowledge() {
  const {catalog}=useData();
  const docs=catalog.documents.filter(d=>d.kind==='knowledge');
  const [query,setQuery]=useState('');
  const [opened,setOpened]=useState<Record<string,boolean>>({cpp:true,windows:true,algorithm:true});
  const term=normalizeSearch(query);
  const visible=useMemo<VisibleArea[]>(()=>areas.flatMap(area=>{
    const areaMatches=normalizeSearch(area.title+' '+area.subtitle).includes(term);
    const topics=area.topics.flatMap(topic=>{
      const documents=docs.filter(d=>d.category===topic.category && !!topic.category).sort(compareKnowledgeDocuments);
      const topicMatches=normalizeSearch(topic.title+' '+topic.description).includes(term);
      const matchedDocs=documents.filter(d=>normalizeSearch([d.title,d.subtitle,...d.tags].join(' ')).includes(term));
      if(term && !areaMatches && !topicMatches && !matchedDocs.length)return [];
      return [{...topic,documents:term&&!areaMatches&&!topicMatches?matchedDocs:documents}];
    });
    return topics.length?[{...area,topics,count:topics.reduce((total,topic)=>total+topic.documents.length,0)}]:[];
  }),[docs,term]);
  const totalTopics=areas.reduce((n,area)=>n+area.topics.length,0);
  const archived=docs.filter(d=>!term||normalizeSearch([d.title,d.subtitle,d.category,...d.tags].join(' ')).includes(term));
  const expandAll=(value:boolean)=>setOpened(Object.fromEntries(areas.map(a=>[a.id,value])));
  return <div className="page knowledge-page">
    <header className="knowledge-hero">
      <div className="knowledge-kicker"><ListTree size={15}/> PROGRAMMING KNOWLEDGE / ATLAS NOTE</div>
      <h1>프로그래밍 지식 라이브러리</h1>
      <p>사이트의 장기 학습 목차입니다. 기존 공개 교재를 유지하면서 신규 집필 과정은 계획 단계로 표시합니다.</p>
      <div className="knowledge-hero-meta">
        <span><strong>{areas.length}</strong>개 주요 분야</span>
        <span><strong>{totalTopics}</strong>개 하위 주제</span>
        <span><strong>{docs.length}</strong>개 기존 공개 교재</span>
        <span className="knowledge-prototype">목차 설계 v3 · C++ 기초부터 순서대로</span>
      </div>
    </header>
    <div className="knowledge-layout">
      <aside className="knowledge-map-sidebar" aria-label="학습 분야 바로가기">
        <div className="knowledge-map-sidebar-title"><ListTree size={17}/> 목차 <span>CONTENTS</span></div>
        <nav aria-label="주요 학습 분야">
          {areas.map(area=>{
            const Icon=area.icon;
            const count=docs.filter(d=>area.topics.some(t=>t.category===d.category && !!t.category)).length;
            return <a key={area.id} href={'#knowledge-'+area.id}><Icon size={15}/><span>{area.title}</span><small>{count||'—'}</small></a>;
          })}
        </nav>
        <div className="knowledge-sidebar-note"><CheckCircle2 size={16}/><span>추가 목차는 신규 집필 계획입니다. 빈 문서나 가짜 완료 상태를 생성하지 않습니다.</span></div>
      </aside>
      <div className="knowledge-main-column">
        <section className="knowledge-outline" aria-labelledby="knowledge-map-heading">
          <div className="knowledge-outline-heading">
            <div><span className="eyebrow">TOPIC EXPLORER</span><h2 id="knowledge-map-heading">전체 학습 목차</h2><p>분야별 학습 계획과 현재 공개 교재를 구분해 표시합니다.</p></div>
            <div className="knowledge-outline-actions"><button type="button" onClick={()=>expandAll(true)}>모두 펼치기</button><button type="button" onClick={()=>expandAll(false)}>모두 접기</button></div>
          </div>
          <label className="knowledge-search">
            <Search size={17}/>
            <input type="search" aria-label="학습 목차 검색" placeholder="학습 주제 또는 단원 검색" value={query} onChange={e=>setQuery(e.target.value)}/>
            {query&&<button type="button" onClick={()=>setQuery('')} aria-label="목차 검색 초기화"><X size={16}/></button>}
          </label>
          <div className="knowledge-tree">
            {visible.map(area=>{
              const Icon=area.icon;
              const isOpen=!!term||!!opened[area.id];
              return <section className="knowledge-tree-area" id={'knowledge-'+area.id} key={area.id}>
                <button className="knowledge-tree-group" type="button" aria-expanded={isOpen} onClick={()=>setOpened(state=>({...state,[area.id]:!isOpen}))}>
                  <ChevronRight className="knowledge-chevron" size={17}/>
                  <span className="knowledge-area-icon"><Icon size={18}/></span>
                  <span className="knowledge-area-label"><strong>{area.title}</strong><small>{area.subtitle}</small></span>
                  <span className="knowledge-count">{area.count?area.count+'개 공개':'학습 계획'}</span>
                </button>
                {isOpen&&<div className="knowledge-topic-list">
                  {area.topics.map(topic=><div className="knowledge-topic" key={topic.id}>
                    <div className="knowledge-topic-row"><Folder size={17}/><div><strong>{topic.title}</strong><small>{topic.description}</small></div><span>{topic.documents.length?topic.documents.length+'개 공개':phaseLabels[topic.phase]}</span></div>
                    {topic.documents.map(d=><Link className="knowledge-document-link" to={d.route} key={d.id}><FileText size={16}/><span><strong>{d.title}</strong><small>{d.subtitle}</small></span><ArrowRight size={16}/></Link>)}
                  </div>)}
                </div>}
              </section>;
            })}
            {!visible.length&&<div className="knowledge-search-empty">설계 목차에서 일치하는 항목을 찾지 못했습니다. 이전 공개 교재는 아래에서 별도로 검색할 수 있습니다.</div>}
          </div>
        </section>
        <section className="knowledge-recent" aria-label="기존 공개 프로토타입">
          <div className="section-heading"><h2><FolderOpen size={19}/> 공개 학습 교재 · 검색</h2><span className="muted text-xs">{archived.length} / {docs.length}개</span></div>
          <div className="knowledge-recent-list">{archived.map(d=><Link key={d.id} to={d.route}><span className="knowledge-recent-icon"><FileText size={17}/></span><span><small>{d.category}</small><strong>{d.title}</strong></span><ArrowRight size={16}/></Link>)}</div>
          <p className="knowledge-catalog-note">WinAPI·게임 수학 기존 본문을 단계별로 이관하고 있습니다. DX11·LIS·BeautifulSoup 시범 교재의 URL도 유지됩니다. 공통 Win32 GUI 소스 텍스트는 복원했지만 Windows 실기기 실행과 원본 ZIP·첨부 바이너리는 아직 미검증입니다.</p>
        </section>
        <DocumentLibrary docs={docs} thinking={false}/>
      </div>
    </div>
    <p className="knowledge-footnote">기존 Notion 본문을 순차적으로 읽어 GitHub에 독립 보관합니다. 실제 게시 문서만 공개 숫자에 포함하며, 신규 교재 집필 예정 항목은 이관 완료로 간주하지 않습니다. Windows/MSVC·DirectX/GPU 실행은 별도 검증 대상입니다.</p>
  </div>;
}
