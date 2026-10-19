import { useMemo, useState, type ComponentType } from 'react';
import { Link } from 'react-router-dom';
import { ArrowRight, BookOpen, Braces, CheckCircle2, ChevronRight, Code2, Cpu, FileText, Folder, FolderOpen, Layers3, ListTree, Network, Search, Wrench, X } from 'lucide-react';
import { useData } from '@/lib/data';
import { DocumentLibrary } from '@/components/DocumentLibrary';
import { normalizeSearch } from '@/lib/document-library';
import type { DocumentMeta } from '@/lib/types';

type Topic = { id: string; title: string; description: string; category?: string };
type Area = { id: string; title: string; subtitle: string; icon: ComponentType<{size?:number; className?:string}>; topics: Topic[] };
type VisibleTopic = Topic & { documents: DocumentMeta[] };
type VisibleArea = Omit<Area, 'topics'> & { topics: VisibleTopic[]; count: number };
const areas: Area[] = [
  {id:'cpp',title:'C / C++',subtitle:'언어부터 메모리·객체 수명까지',icon:Braces,topics:[
    {id:'c',title:'C 언어',description:'입력·포인터·메모리'},
    {id:'cpp-core',title:'C++ 기초·심화',description:'RAII·템플릿·STL'},
    {id:'cpp-effective',title:'Effective C++',description:'소유권·예외 안전성·성능'}
  ]},
  {id:'directx',title:'DirectX / Windows',subtitle:'게임 클라이언트의 렌더링 기반',icon:Code2,topics:[
    {id:'dx9',title:'DirectX 9',description:'고정 파이프라인·레거시 엔진'},
    {id:'dx11',title:'DirectX 11',description:'자원·셰이더·가시성 판정',category:'DirectX 11'},
    {id:'winapi',title:'WinAPI·게임 수학',description:'게임 루프·변환·충돌 기초'}
  ]},
  {id:'graphics',title:'그래픽스 최적화',subtitle:'GPU 파이프라인과 데이터 처리',icon:Cpu,topics:[
    {id:'hlsl',title:'HLSL 셰이더',description:'행렬·라이팅·후처리'},
    {id:'simd',title:'SIMD 연산',description:'벡터화·정렬·CPU 경계'},
    {id:'render',title:'렌더링 시스템',description:'컬링·버퍼·렌더 타깃'}
  ]},
  {id:'cs',title:'컴퓨터 과학',subtitle:'코드가 동작하는 시스템의 원리',icon:Network,topics:[
    {id:'computer',title:'컴퓨터 구조',description:'캐시·CPU·메모리 계층'},
    {id:'os',title:'운영체제',description:'스레드·가상 메모리·동기화'},
    {id:'network',title:'네트워크',description:'TCP/UDP·패킷·프로토콜'}
  ]},
  {id:'algorithm',title:'자료구조 / 알고리즘',subtitle:'복잡도·탐색·문제 해결',icon:Layers3,topics:[
    {id:'ds',title:'자료구조',description:'배열·트리·그래프'},
    {id:'algo',title:'알고리즘',description:'이분 탐색·LIS·동적 계획법',category:'알고리즘'},
    {id:'solved',title:'기존 문제풀이',description:'반례·복잡도·검증 기록'}
  ]},
  {id:'architecture',title:'설계 패턴 / Unity',subtitle:'구조화하고 확장 가능한 설계',icon:Wrench,topics:[
    {id:'patterns',title:'디자인 패턴',description:'Factory·Builder·책임 분리'},
    {id:'unity',title:'Unity 모듈 설계',description:'컴포넌트·데이터·이벤트'}
  ]},
  {id:'python',title:'Python / 자동화',subtitle:'반복 작업을 검증 가능한 도구로',icon:BookOpen,topics:[
    {id:'python-basics',title:'Python 기초',description:'언어·타입·파일 입출력'},
    {id:'crawling',title:'웹 크롤링·파싱',description:'BeautifulSoup·오프라인 테스트',category:'Python'},
    {id:'automation',title:'자동화·테스트',description:'스크립트·검증·실행 로그'}
  ]}
];

export default function Knowledge() {
  const {catalog}=useData();
  const docs=catalog.documents.filter(d=>d.kind==='knowledge');
  const [query,setQuery]=useState('');
  const [opened,setOpened]=useState<Record<string,boolean>>({directx:true,algorithm:true,python:true});
  const term=normalizeSearch(query);
  const visible=useMemo<VisibleArea[]>(()=>areas.flatMap(area=>{
    const areaMatches=normalizeSearch(area.title+' '+area.subtitle).includes(term);
    const topics=area.topics.flatMap(topic=>{
      const documents=docs.filter(d=>d.category===topic.category && !!topic.category);
      const topicMatches=normalizeSearch(topic.title+' '+topic.description).includes(term);
      const matchedDocs=documents.filter(d=>normalizeSearch([d.title,d.subtitle,...d.tags].join(' ')).includes(term));
      if(term && !areaMatches && !topicMatches && !matchedDocs.length)return [];
      return [{...topic,documents:term&&!areaMatches&&!topicMatches?matchedDocs:documents}];
    });
    return topics.length?[{...area,topics,count:topics.reduce((total,topic)=>total+topic.documents.length,0)}]:[];
  }),[docs,term]);
  const totalTopics=areas.reduce((n,area)=>n+area.topics.length,0);
  const expandAll=(value:boolean)=>setOpened(Object.fromEntries(areas.map(a=>[a.id,value])));
  return <div className="page knowledge-page">
    <header className="knowledge-hero">
      <div className="knowledge-kicker"><ListTree size={15}/> PROGRAMMING KNOWLEDGE / ATLAS NOTE</div>
      <h1>프로그래밍 지식 라이브러리</h1>
      <p>Notion의 폴더형 목차를 한곳으로 옮겼습니다. 분야를 펼쳐 개별 단원으로 들어가거나, 아래에서 직접 검색할 수 있습니다.</p>
      <div className="knowledge-hero-meta">
        <span><strong>{areas.length}</strong>개 주요 분야</span>
        <span><strong>{totalTopics}</strong>개 하위 주제</span>
        <span><strong>{docs.length}</strong>개 이관 교재</span>
        <span className="knowledge-prototype">프로토타입 · 단계별 이관 중</span>
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
        <div className="knowledge-sidebar-note"><CheckCircle2 size={16}/><span>현재 이관된 문서만 열 수 있습니다. 나머지 주제는 원본을 검수한 뒤 순차 공개합니다.</span></div>
      </aside>
      <div className="knowledge-main-column">
        <section className="knowledge-outline" aria-labelledby="knowledge-map-heading">
          <div className="knowledge-outline-heading">
            <div><span className="eyebrow">TOPIC EXPLORER</span><h2 id="knowledge-map-heading">전체 학습 목차</h2><p>폴더를 펼쳐 세부 주제를 살펴보세요.</p></div>
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
                  <span className="knowledge-count">{area.count?area.count+'개 교재':'이관 예정'}</span>
                </button>
                {isOpen&&<div className="knowledge-topic-list">
                  {area.topics.map(topic=><div className="knowledge-topic" key={topic.id}>
                    <div className="knowledge-topic-row"><Folder size={17}/><div><strong>{topic.title}</strong><small>{topic.description}</small></div><span>{topic.documents.length?topic.documents.length+'개':'준비 중'}</span></div>
                    {topic.documents.map(d=><Link className="knowledge-document-link" to={d.route} key={d.id}><FileText size={16}/><span><strong>{d.title}</strong><small>{d.subtitle}</small></span><ArrowRight size={16}/></Link>)}
                  </div>)}
                </div>}
              </section>;
            })}
            {!visible.length&&<div className="knowledge-search-empty">일치하는 분야나 단원이 없습니다. 다른 검색어를 입력해 주세요.</div>}
          </div>
        </section>
        <section className="knowledge-recent" aria-label="바로 학습하기">
          <div className="section-heading"><h2><FolderOpen size={19}/> 지금 열 수 있는 교재</h2><span className="muted text-xs">{docs.length}개 교재</span></div>
          <div className="knowledge-recent-list">{docs.map(d=><Link key={d.id} to={d.route}><span className="knowledge-recent-icon"><FileText size={17}/></span><span><small>{d.category}</small><strong>{d.title}</strong></span><ArrowRight size={16}/></Link>)}</div>
        </section>
        <DocumentLibrary docs={docs} thinking={false}/>
      </div>
    </div>
    <p className="knowledge-footnote">Notion 원본과 소스 출처는 각 이관 교재에 기록되어 있습니다. 기존 CPU 검증과 GitHub 이관 이후의 실행 검증은 구분하며, 이미지·첨부 전수 이관과 Windows/GPU 검증은 아직 완료되지 않았습니다.</p>
  </div>;
}
