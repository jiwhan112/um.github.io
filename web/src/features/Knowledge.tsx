import { type ComponentType } from 'react';
import { BookOpen, Braces, Code2, Cpu, Layers3, Network, Wrench } from 'lucide-react';
import { LearningIndex } from './Learning';
import { useData } from '@/lib/data';
type Phase = 'legacy' | 'new' | 'replan' | 'keep';
type Topic = { id: string; title: string; description: string; phase: Phase; category?: string };
type Area = { id: string; title: string; subtitle: string; icon: ComponentType<{size?:number; className?:string}>; topics: Topic[] };
const phaseLabels: Record<Phase,string> = {
  legacy: '기존 자료 기준', new: '신규 집필 예정', replan: '집필 시 설계', keep: '기존 구성 유지'
};
// This is a long-term curriculum outline, NOT a declaration of migration or publication completion.
const areas: Area[] = [
  {id:'cpp',title:'C / C++',subtitle:'언어 원리부터 최신 C++까지 단계적으로 학습',icon:Braces,topics:[
    {id:'c',title:'C 언어',description:'C 언어 기본·포인터·메모리',phase:'legacy',category:'C 언어'},
    {id:'cpp-core',title:'C++ 기초·심화',description:'빌드·타입·함수·메모리에서 객체·STL·RAII까지',phase:'legacy',category:'C++ 기초·심화'},
    {id:'cpp-modern',title:'Modern C++',description:'C++11 이후 언어·표준 라이브러리의 버전별 변화',phase:'new',category:'Modern C++'},
    {id:'cpp-effective',title:'Effective C++',description:'올바른 자원 관리와 C++ 설계 원칙',phase:'legacy',category:'Effective C++'},
    {id:'cpp-effective-modern',title:'Effective Modern C++',description:'현대 C++ 관용구를 자체 예제와 설계 판단으로 학습',phase:'new',category:'Effective Modern C++'}
  ]},
  {id:'windows',title:'Windows·그래픽스 프로그래밍',subtitle:'Windows SDK 기반 게임 클라이언트 · DirectX 구현',icon:Code2,topics:[
    {id:'winapi',title:'WinAPI',description:'메시지 루프·윈도우·입력·시스템 API',phase:'legacy',category:'WinAPI'},
    {id:'game-math',title:'게임 수학',description:'벡터·행렬·변환·충돌 수학',phase:'legacy',category:'게임 수학'},
    {id:'dx9',title:'DirectX 9',description:'기존 DX9 엔진과 렌더링 파이프라인',phase:'legacy',category:'DirectX 9'},
    {id:'dx11',title:'DirectX 11',description:'자원·렌더링 파이프라인·엔진 기능',phase:'legacy',category:'DirectX 11'},
    {id:'dx12',title:'DirectX 12',description:'명시적 GPU 자원·명령 기록·동기화와 검증 경계',phase:'new',category:'DirectX 12'}
  ]},
  {id:'graphics',title:'그래픽스 기술',subtitle:'프레임워크 구현과 분리해 개별 기법을 깊게 연구',icon:Cpu,topics:[
    {id:'graphics-tech',title:'그래픽스 기법·탐색·애니메이션',description:'색·조명·그림자·시간축 처리·Navigation Mesh·IK의 원리와 실험',phase:'new',category:'그래픽스 기술'}
  ]},
  {id:'cs',title:'컴퓨터 과학',subtitle:'컴퓨터 구조·운영체제·네트워크 기초',icon:Network,topics:[
    {id:'computer',title:'컴퓨터 구조',description:'캐시·CPU·메모리 계층',phase:'keep',category:'컴퓨터 구조'},
    {id:'os',title:'운영체제',description:'스레드·가상 메모리·동기화',phase:'keep',category:'운영체제'},
    {id:'network',title:'네트워크',description:'TCP/UDP·패킷·프로토콜',phase:'keep',category:'네트워크'}
  ]},
  {id:'algorithm',title:'자료구조 / 알고리즘',subtitle:'불변식·복잡도에서 게임 데이터와 경로 탐색까지',icon:Layers3,topics:[
    {id:'algorithm-plan',title:'자료구조·탐색·알고리즘',description:'정규 과정과 기존 LIS 심화 실습 · 경계 조건과 정확성 검증',phase:'new',category:'알고리즘'}
  ]},
  {id:'engine',title:'게임 엔진 개발',subtitle:'기존 Notion 설계 패턴·Unity 노트를 이관하지 않고 새로 집필',icon:Wrench,topics:[
    {id:'engine-systems',title:'엔진 파츠·시스템 설계',description:'새로운 게임 엔진 학습 교재로 기획 · 세부 목차는 작성 시 확정',phase:'new'}
  ]},
  {id:'python',title:'Python·개발 도구',subtitle:'기존 Notion Python 문서 대신 새 교육 과정으로 집필',icon:BookOpen,topics:[
    {id:'python-new',title:'Python 파츠·자동화',description:'새 Python 학습·도구 개발 교재를 독립 집필',phase:'new'}
  ]}
];

export default function Knowledge() {
  const { catalog } = useData();
  const published = (category?: string) => category ? catalog.documents.filter(doc => doc.kind === 'knowledge' && doc.category === category).length : 0;
  return <LearningIndex kind="knowledge" title="프로그래밍 학습" eyebrow="PROGRAMMING / STUDY LIBRARY"
    description="언어부터 그래픽스까지, 한 과정씩 배우고 직접 구현하는 학습 노트."
    after={<details className="learning-plan"><summary>전체 분야와 집필 계획 <span>7개 분야 · 공개 교재와 별도 안내</span></summary><div className="learning-plan-grid">{areas.map(area => {
      const Icon = area.icon;
      return <section key={area.id} id={'knowledge-' + area.id}><h3><Icon size={16}/>{area.title}</h3><p>{area.subtitle}</p><ul>{area.topics.map(topic => <li key={topic.id}><strong>{topic.title}</strong><span>{published(topic.category) ? published(topic.category) + '개 공개' : phaseLabels[topic.phase]}</span><small>{topic.description}</small></li>)}</ul></section>;
    })}</div><p className="learning-footnote">이 목록은 장기 집필 계획입니다. 위 학습 목차에는 실제 공개된 교재만 표시하며, 신규 집필 계획을 이관·완료 실적으로 계산하지 않습니다.</p></details>}/>
}
