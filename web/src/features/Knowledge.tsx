import { Link } from 'react-router-dom';
import { BookOpen, ArrowUpRight } from 'lucide-react';
import { useData } from '@/lib/data';
import { PageHeader, Panel } from '@/components/Common';
import { DocumentLibrary } from '@/components/DocumentLibrary';
export default function Knowledge() {
 const {catalog}=useData();
 const docs=catalog.documents.filter(d=>d.kind==='knowledge');
 const areas=[{name:'DirectX 11',detail:'렌더링·그래픽스 계약'},{name:'알고리즘',detail:'자료구조·문제 해결'},{name:'Python',detail:'자동화·크롤링·검증'}];
 return <div className="page knowledge-index">
  <PageHeader eyebrow="KNOWLEDGE / NOTION MIGRATION · PROTOTYPE" title="프로그래밍 지식 라이브러리" description="Notion에서 옮겨온 학습 자료를 코드·표·도식·복습과 함께 읽습니다. 현재는 분야별 대표 단원 3개의 이관 검증판입니다."/>
  <div className="quick-grid">{areas.map(a=><Panel key={a.name} title={a.name}><p className="muted text-xs">{a.detail}</p>{docs.filter(d=>d.category===a.name).map(d=><Link className="text-link" key={d.id} to={d.route}>{d.title}<ArrowUpRight size={15}/></Link>)}</Panel>)}</div>
  <div className="section-heading"><h2><BookOpen size={19}/> 이관된 학습 자료</h2></div>
  <DocumentLibrary docs={docs} thinking={false}/>
  <p className="muted text-xs mt-6">Notion 원본 URL과 원문은 GitHub 소스 파일에 보존되어 있습니다. 이전 CPU 테스트 결과는 Notion의 과거 실행 기록이며, 이 사이트로 이관한 코드의 Windows·GPU 재검증 결과가 아닙니다. 일부 이미지는 아직 이전하지 않았습니다.</p>
 </div>;
}
