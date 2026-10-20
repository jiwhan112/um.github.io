import { useEffect } from 'react';
import { Link, useLocation, useNavigate } from 'react-router-dom';
import { BookOpen, ListTree } from 'lucide-react';
import { Blocks } from '@/components/Content';
import { Panel, ReadCheck, NextPrevious } from '@/components/Common';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { useData } from '@/lib/data';
import { cleanTitle, dateLabel } from '@/lib/utils';
import type { Document } from '@/lib/types';
import './knowledge-reader.css';

const labels: Record<string, string> = {
  document: '개념과 원리', practice: '실습·복습', sources: '출처·검증',
  history: '원문·변경기록', implementation: '구현', summary: '핵심 요약',
  files: '전체 소스', overview: '해설',
};

export function KnowledgeReader({ doc }: { doc: Document }) {
  const { catalog } = useData();
  const location = useLocation();
  const navigate = useNavigate();
  const tabs = [...new Set(doc.sections.map(section => section.tab))];
  let hash = '';
  try { hash = decodeURIComponent(location.hash.slice(1)); } catch { /* 잘못된 외부 앵커는 무시합니다. */ }
  const requested = new URLSearchParams(location.search).get('tab');
  const selected = doc.sections.find(section => section.id === hash)?.tab || requested || tabs[0];
  const current = tabs.includes(selected) ? selected : tabs[0];
  const visible = doc.sections.filter(section => section.tab === current);
  const related = catalog.documents.filter(item => item.kind === 'knowledge')
    .sort((a, b) => String(a.number || a.title).localeCompare(String(b.number || b.title), undefined, { numeric: true }));

  useEffect(() => {
    if (!hash) return;
    const frame = requestAnimationFrame(() => document.getElementById(hash)?.scrollIntoView({ block: 'start' }));
    return () => cancelAnimationFrame(frame);
  }, [hash, current]);

  const toc = (mobile = false) => <nav className="toc knowledge-reader-toc" aria-label={mobile ? '모바일 단원 목차' : '문서 목차'}>
    {visible.map(section => <Link key={section.id}
      to={{ pathname: location.pathname, search: '?tab=' + encodeURIComponent(section.tab), hash: '#' + encodeURIComponent(section.id) }}
      aria-current={section.id === hash ? 'location' : undefined}
      onClick={event => {
        if (mobile) event.currentTarget.closest('details')?.removeAttribute('open');
        if (section.id === hash) document.getElementById(hash)?.scrollIntoView({ block: 'start' });
      }}>{section.title}</Link>)}
  </nav>;

  return <div className="page document-page knowledge-document knowledge-reader">
    <div className="document-workspace">
      <div className="document-main">
        <header className="document-heading">
          <Link className="knowledge-back" to="/knowledge/"><BookOpen size={15}/>프로그래밍 학습 목차</Link>
          <div className="eyebrow">{doc.category || 'PROGRAMMING'} / ATLAS NOTE</div>
          <h1>{cleanTitle(doc.title)}</h1>
          <p className="page-description">{doc.subtitle}</p>
          <div className="flex flex-wrap gap-2"><span className="badge">{dateLabel(doc.date)}</span><span className="badge">개념 · 구현 · 실습</span></div>
        </header>
        <Tabs value={current} onValueChange={tab => navigate({ pathname: location.pathname, search: '?tab=' + encodeURIComponent(tab) }, { replace: true })}>
          <TabsList aria-label="문서 내용">{tabs.map(tab => <TabsTrigger key={tab} value={tab}>{labels[tab] || tab}</TabsTrigger>)}</TabsList>
          <details className="knowledge-mobile-toc"><summary><ListTree size={16}/>이 탭의 목차 <span>{visible.length}</span></summary>{toc(true)}</details>
          {tabs.map(tab => <TabsContent key={tab} value={tab}>
            {tab === 'history' && <p className="knowledge-history-note">이 영역에는 이전 편집 기록이 보존되어 있습니다. 현재 학습은 ‘개념과 원리’, 실행 가능한 소스는 ‘실습·복습’에서 확인하세요.</p>}
            <div className="prose">{doc.sections.filter(section => section.tab === tab).map(section => <section key={section.id} id={section.id}>
              {section.blocks[0]?.type !== 'heading' && <h2>{section.title}</h2>}
              <Blocks blocks={section.blocks}/>
            </section>)}</div>
          </TabsContent>)}
        </Tabs>
        <NextPrevious items={related} current={doc.route}/>
      </div>
      <aside className="document-tools">
        <Panel title="이 문서의 목차"><p className="knowledge-toc-tab">{labels[current] || current}</p>{toc()}</Panel>
        <Panel title="내 학습 기록"><ReadCheck path={doc.route}/><p className="muted text-xs mt-3">학습 기록은 기존 방식으로 저장하고, 동기화를 연결하면 다른 기기와 공유합니다.</p></Panel>
      </aside>
    </div>
  </div>;
}
