import { useMemo } from 'react';
import { Link, useSearchParams } from 'react-router-dom';
import { ArrowRight, Search, RotateCcw } from 'lucide-react';
import { Button } from './ui/button';
import { Empty } from './Common';
import { cleanTitle, dateLabel } from '@/lib/utils';
import { filterDocuments, matchesDocument, readLibraryFilter } from '@/lib/document-library';
import type { DocumentMeta } from '@/lib/types';

export function DocumentLibrary({ docs, thinking }: { docs: DocumentMeta[]; thinking: boolean }) {
  const [params, setParams] = useSearchParams();
  const filter = readLibraryFilter(params);
  const categories = [...new Set(docs.map(d => d.category).filter(Boolean))].sort((a,b) => a.localeCompare(b,'ko'));
  const tags = [...new Set(docs.flatMap(d => d.tags))].sort((a,b) => a.localeCompare(b,'ko'));
  const visible = useMemo(() => filterDocuments(docs, filter), [docs, params.toString()]);
  const counts = new Map(tags.map(tag => [tag, docs.filter(d => d.tags.includes(tag) && matchesDocument(d, { ...filter, tags: [] })).length]));
  function change(key: string, value: string, replace = false) {
    const next = new URLSearchParams(params);
    if (value) next.set(key, value); else next.delete(key);
    setParams(next, { replace });
  }
  function toggleTag(tag: string) {
    const next = new URLSearchParams(params), selected = new Set(filter.tags);
    if (selected.has(tag)) selected.delete(tag); else selected.add(tag);
    next.delete('tag'); [...selected].sort().forEach(t => next.append('tag', t)); setParams(next);
  }
  function reset() {
    const next = new URLSearchParams(params); ['q','category','tag','sort'].forEach(k => next.delete(k)); setParams(next);
  }
  const active = !!(filter.query || filter.category || filter.tags.length || filter.sort !== 'number');
  return <section className="problem-library" id="problem-library" aria-labelledby="problem-library-heading">
    <div className="section-heading"><h2 id="problem-library-heading">{thinking ? '문제 라이브러리' : '문서 라이브러리'}</h2><span className="muted text-xs" role="status" aria-live="polite" data-testid="library-count">{visible.length} / {docs.length}개</span></div>
    <div className="problem-filters">
      <div className="problem-filter-row">
        <label className="problem-query"><span>{thinking ? '문제 검색' : '문서 검색'}</span><span className="search-input"><Search size={16}/><input aria-label={thinking ? '문제 검색' : '문서 검색'} type="search" value={filter.query} onChange={e => change('q', e.target.value, true)} placeholder="번호, 제목, 주제, 태그로 검색"/></span></label>
        <label><span>분류</span><select aria-label="문제 분류" value={filter.category} onChange={e => change('category', e.target.value)}><option value="">전체 분류</option>{categories.map(c => <option key={c}>{c}</option>)}{filter.category && !categories.includes(filter.category) && <option>{filter.category}</option>}</select></label>
        <label><span>정렬</span><select aria-label="문제 정렬" value={filter.sort} onChange={e => change('sort', e.target.value)}><option value="number">문항 순서</option><option value="newest">학습일 최신순</option><option value="oldest">학습일 오래된순</option></select></label>
        <Button variant="outline" size="sm" onClick={reset} disabled={!active}><RotateCcw size={14}/>필터 초기화</Button>
      </div>
      <fieldset className="problem-tags"><legend>태그 · 여러 개 선택하면 모두 포함한 문제를 표시합니다.</legend><div className="problem-tag-list">{[...new Set([...tags, ...filter.tags])].map(tag => <button className="problem-tag" type="button" key={tag} aria-label={'태그 '+tag} aria-pressed={filter.tags.includes(tag)} onClick={() => toggleTag(tag)}>{tag}<span className="tag-count">{counts.get(tag)||0}</span></button>)}</div></fieldset>
      <p className="problem-filter-caption">제목·요약·주제를 검색합니다. 정답 본문은 검색 결과에 노출하지 않습니다. 검색 조건은 주소에 저장됩니다.</p>
    </div>
    {visible.length ? <div className="document-grid">{visible.map(d => <article className="document-card" data-testid="library-card" key={d.id}>
      <div className="flex flex-wrap gap-2"><span className="badge">{d.number}</span>{d.category && <span className="badge">{d.category}</span>}</div>
      <h3><Link to={d.route}>{cleanTitle(d.title)}</Link></h3><p>{d.subtitle || '기술 구조와 설계 원리를 살펴봅니다.'}</p>
      <div className="document-tags">{d.tags.map(tag => <button key={tag} type="button" className="problem-tag" aria-label={'이 태그로 필터 '+tag} aria-pressed={filter.tags.includes(tag)} onClick={() => toggleTag(tag)}>{tag}</button>)}</div>
      <div className="document-card-footer"><time dateTime={d.date}>{dateLabel(d.date)}</time><Link className="text-link" to={d.route}>학습 열기 <ArrowRight size={14}/></Link></div>
    </article>)}</div> : <><Empty title="조건에 맞는 문제가 없습니다." description="검색어나 선택한 태그를 줄여보세요."/><Button variant="outline" onClick={reset}>전체 문제 보기</Button></>}
  </section>;
}
