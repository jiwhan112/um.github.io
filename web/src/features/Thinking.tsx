import { useEffect, useMemo, useRef } from 'react';
import { Link, NavLink, useLocation, useNavigate, useSearchParams } from 'react-router-dom';
import { ArrowLeft, ArrowRight, BookOpen, Check, ChevronRight, Search, RotateCcw, SlidersHorizontal } from 'lucide-react';
import { useData } from '@/lib/data';
import { keys, objectBooleans, useStored } from '@/lib/storage';
import { filterDocuments, matchesDocument, readLibraryFilter } from '@/lib/document-library';
import { chapterNumber, chapterTitle, compareThinking, isThinkingRead, levelLabel, problemTitle, thinkingChapters, topicNumber, type ThinkingTopic } from '@/lib/thinking-navigation';
import { Button } from '@/components/ui/button';
import { Empty } from '@/components/Common';
import type { Document, DocumentMeta } from '@/lib/types';
import './thinking.css';

function TopicRow({ topic, reading, current }: { topic: ThinkingTopic; reading: Record<string, boolean>; current?: string }) {
  const basic = topic.pages.find(d => levelLabel(d) === '기본') || topic.pages[0];
  return <li className={'thinking-topic-row' + (topic.pages.some(d => d.route === current) ? ' is-current' : '')}>
    <span className="thinking-topic-number">{topic.number}</span>
    <div className="thinking-topic-copy"><h3>{topic.title}</h3><p>{problemTitle(basic)}</p></div>
    <div className="thinking-pair">{topic.pages.map(doc => <Link key={doc.id} to={doc.route} data-testid="library-card" aria-current={doc.route === current ? 'page' : undefined} aria-label={`${doc.number} ${levelLabel(doc)} — ${problemTitle(doc)}`} title={problemTitle(doc)} className={isThinkingRead(doc, reading) ? 'is-complete' : ''}>{isThinkingRead(doc, reading) && <Check size={13} aria-label="학습 완료"/>}{levelLabel(doc)}<ChevronRight size={13}/></Link>)}</div>
  </li>;
}

export function ThinkingIndex() {
  const { catalog } = useData();
  const docs = useMemo(() => catalog.documents.filter(d => d.kind === 'thinking').sort(compareThinking), [catalog]);
  const chapters = useMemo(() => thinkingChapters(docs), [docs]);
  const [stored] = useStored<unknown>(keys.reading, {}), reading = objectBooleans(stored);
  const [params, setParams] = useSearchParams(), filter = readLibraryFilter(params);
  const visible = filterDocuments(docs, filter), matching = thinkingChapters(visible);
  const completed = docs.filter(d => isThinkingRead(d, reading)).length;
  const next = docs.find(d => !isThinkingRead(d, reading)) || docs[0];
  const hasFilters = !!(filter.query || filter.category || filter.tags.length);
  const requestedChapter = params.get('chapter');
  const selected = matching.find(c => c.number === requestedChapter) || matching.find(c => c.number === (!hasFilters && next ? chapterNumber(next) : '')) || matching[0];
  const orderedTopics = selected?.topics.slice().sort((a, b) => visible.indexOf(a.pages[0]) - visible.indexOf(b.pages[0])) || [];
  const categories = [...new Set(docs.map(d => d.category).filter(Boolean))].sort((a, b) => a.localeCompare(b, 'ko'));
  const tags = [...new Set([...docs.flatMap(d => d.tags), ...filter.tags])].sort((a, b) => a.localeCompare(b, 'ko'));
  const tagCounts = new Map(tags.map(tag => [tag, docs.filter(d => d.tags.includes(tag) && matchesDocument(d, { ...filter, tags: [] })).length]));
  const active = hasFilters || filter.sort !== 'number';
  function change(key: string, value: string, replace = false) {
    const nextParams = new URLSearchParams(params);
    if (value) nextParams.set(key, value); else nextParams.delete(key);
    if (key !== 'chapter' && key !== 'sort') nextParams.delete('chapter');
    setParams(nextParams, { replace });
  }
  function toggleTag(tag: string) {
    const nextParams = new URLSearchParams(params), selectedTags = new Set(filter.tags);
    if (selectedTags.has(tag)) selectedTags.delete(tag); else selectedTags.add(tag);
    nextParams.delete('tag'); nextParams.delete('chapter'); [...selectedTags].sort().forEach(t => nextParams.append('tag', t)); setParams(nextParams);
  }
  function reset() { const nextParams = new URLSearchParams(params); ['q', 'category', 'tag', 'sort', 'chapter'].forEach(k => nextParams.delete(k)); setParams(nextParams); }
  const allDone = docs.length > 0 && completed === docs.length;
  return <div className="page thinking-index">
    <header className="thinking-header"><div><p className="eyebrow">THINKING LAB / STUDY BOOK</p><h1>한 문제씩, 생각을 단단하게.</h1><p className="page-description">장별로 개념을 익히고, 기본에서 심화로 넓혀 가는 사고력 학습 노트.</p></div><div className="thinking-book-stats"><span><strong>{chapters.length}</strong>개 장</span><span><strong>{chapters.reduce((sum, c) => sum + c.topics.length, 0)}</strong>개 주제</span><span><strong>{docs.length}</strong>개 문제</span></div></header>
    {!next ? <Empty/> : <>
      <section className="thinking-start" aria-label="학습 시작"><div className="thinking-start-icon"><BookOpen size={23}/></div><div className="thinking-start-copy"><span className="eyebrow">{allDone ? 'REVIEW' : completed ? 'CONTINUE LEARNING' : 'START HERE'}</span><h2>{allDone ? '모든 문제를 학습했어요. 다시 살펴볼까요?' : completed ? '다음 미완료 문제부터 이어 가세요.' : '첫 장의 기본 문제부터 시작하세요.'}</h2><p>{next.number} · {problemTitle(next)}</p></div><Button asChild><Link data-testid="thinking-study-entry" to={next.route}>{allDone ? '첫 문제 복습' : completed ? '이어서 학습' : '첫 문제 시작'}<ArrowRight size={16}/></Link></Button><div className="thinking-overall-progress" data-testid="thinking-progress"><div><span>내 학습 기록</span><strong>{completed} / {docs.length} 완료</strong></div><progress value={completed} max={docs.length} aria-label="사고력 전체 학습 진도"/></div></section>
      <section className="thinking-curriculum problem-library" id="problem-library" aria-labelledby="problem-library-heading">
        <div className="section-heading"><div><p className="eyebrow">CONTENTS</p><h2 id="problem-library-heading">학습 목차</h2></div><span role="status" aria-live="polite" data-testid="library-count">{hasFilters ? '검색 결과 · ' : ''}{matching.reduce((sum, c) => sum + c.topics.length, 0)}개 주제 · {visible.length} / {docs.length}개 문제</span></div>
        <div className="thinking-search-row"><label className="search-input"><Search size={17}/><input type="search" aria-label="문제 검색" value={filter.query} onChange={e => change('q', e.target.value, true)} placeholder="번호, 주제, 키워드로 문제 찾기"/></label><Button variant="outline" size="sm" disabled={!active} onClick={reset}><RotateCcw size={14}/>필터 초기화</Button></div>
        <details className="thinking-filter-more" open={filter.tags.length > 0 || !!filter.category || filter.sort !== 'number' || undefined}><summary><SlidersHorizontal size={14}/>태그·상세 필터{(filter.tags.length > 0 || filter.category) && <span className="badge">{filter.tags.length + (filter.category ? 1 : 0)}</span>}</summary><div className="problem-filter-row"><label><span>분류</span><select aria-label="문제 분류" value={filter.category} onChange={e => change('category', e.target.value)}><option value="">전체 분류</option>{categories.map(c => <option key={c}>{c}</option>)}{filter.category && !categories.includes(filter.category) && <option>{filter.category}</option>}</select></label><label><span>정렬</span><select aria-label="문제 정렬" value={filter.sort} onChange={e => change('sort', e.target.value)}><option value="number">문항 순서</option><option value="newest">학습일 최신순</option><option value="oldest">학습일 오래된순</option></select></label></div><fieldset className="problem-tags"><legend>태그 · 여러 개 선택하면 모두 포함한 문제를 표시합니다.</legend><div className="problem-tag-list">{tags.map(tag => <button type="button" key={tag} className="problem-tag" aria-label={'태그 ' + tag} aria-pressed={filter.tags.includes(tag)} onClick={() => toggleTag(tag)}>{tag}<span className="tag-count">{tagCounts.get(tag) || 0}</span></button>)}</div></fieldset><p className="problem-filter-caption">제목·요약·주제를 검색합니다. 정답 본문은 노출하지 않으며 검색 조건은 주소에 저장됩니다.</p></details>
        {!selected ? <><Empty title="조건에 맞는 문제가 없습니다." description="검색어나 선택한 태그를 줄여보세요."/><Button variant="outline" onClick={reset}>전체 문제 보기</Button></> : <div className="thinking-book-layout">
          <nav data-testid="thinking-chapter-nav" className="thinking-chapters" aria-label="학습 장 목록"><div className="thinking-chapters-label">{hasFilters ? '검색된 장' : '장 선택'}<span>{matching.length}개</span></div>{matching.map(chapter => <button type="button" key={chapter.number} aria-current={chapter.number === selected.number ? 'true' : undefined} onClick={() => change('chapter', chapter.number)}><span className="thinking-chapter-number">{chapter.number.padStart(2, '0')}</span><span>{chapter.title}<small>{chapter.topics.length}개 주제</small></span>{chapter.pages.every(d => isThinkingRead(d, reading)) ? <Check size={14} aria-label="장 학습 완료"/> : <ChevronRight size={13}/>}</button>)}</nav>
          <div className="thinking-chapter-content" data-testid="thinking-current-chapter"><label className="thinking-mobile-chapter"><span>학습 장</span><select aria-label="학습 장" value={selected.number} onChange={e => change('chapter', e.target.value)}>{matching.map(c => <option key={c.number} value={c.number}>{c.number}장 · {c.title} ({c.topics.length})</option>)}</select></label><header className="thinking-chapter-heading"><div><p className="eyebrow">CHAPTER {selected.number.padStart(2, '0')}</p><h2>{selected.title}</h2><p>{hasFilters ? '검색 조건에 맞는 ' : ''}{selected.topics.length}개 주제 · 기본으로 이해하고 심화로 확장하세요.</p></div><span className="thinking-chapter-progress">{selected.pages.filter(d => isThinkingRead(d, reading)).length} / {selected.pages.length} 완료</span></header><ol className="thinking-topic-list">{orderedTopics.map(topic => <TopicRow key={topic.number} topic={topic} reading={reading}/>)}</ol><footer className="thinking-chapter-footer"><span>✓ 표시는 학습 완료한 문제입니다.</span><span>기본 → 심화 → 다음 주제</span></footer></div>
        </div>}
      </section>
    </>}
  </div>;
}

export function ThinkingSidebar({ docs, onNavigate }: { docs: DocumentMeta[]; onNavigate?: () => void }) {
  const { pathname } = useLocation(), navigate = useNavigate();
  const topicList = useRef<HTMLElement>(null), activeTopic = useRef<HTMLDivElement>(null);
  useEffect(() => { if (topicList.current && activeTopic.current) topicList.current.scrollTop = activeTopic.current.offsetTop - topicList.current.clientHeight / 2 + activeTopic.current.clientHeight / 2; }, [pathname]);
  const chapters = thinkingChapters(docs), current = docs.find(d => d.route === pathname.replace(/\/?$/, '/'));
  const chapter = chapters.find(c => c.number === (current ? chapterNumber(current) : ''));
  const [stored] = useStored<unknown>(keys.reading, {}), reading = objectBooleans(stored);
  return <div className="thinking-sidebar"><div className="nav-label">{chapter ? 'CURRENT CHAPTER' : 'STUDY BOOK'}</div><Link className="thinking-sidebar-home" to="/thinking-lab/" onClick={onNavigate}><BookOpen size={14}/>전체 학습 목차<ArrowRight size={13}/></Link>{chapter ? <><label className="thinking-sidebar-select"><span className="sr-only">목차 장 이동</span><select aria-label="목차 장 이동" value={chapter.number} onChange={e => { navigate('/thinking-lab/?chapter=' + e.target.value); onNavigate?.(); }}>{chapters.map(c => <option key={c.number} value={c.number}>{c.number}장 · {c.title}</option>)}</select></label><nav ref={topicList} data-testid="thinking-chapter-nav" className="thinking-sidebar-topics" aria-label="현재 장의 문제">{chapter.topics.map(topic => <div ref={topic.number === topicNumber(current!) ? activeTopic : undefined} key={topic.number} className={'thinking-sidebar-topic' + (topic.number === topicNumber(current!) ? ' is-current' : '')}><p><span>{topic.number}</span>{topic.title}</p><div>{topic.pages.map(d => <NavLink key={d.id} to={d.route} onClick={onNavigate} aria-label={`${d.number} ${levelLabel(d)} — ${problemTitle(d)}`}>{levelLabel(d)}{isThinkingRead(d, reading) && <Check size={11} aria-label="학습 완료"/>}</NavLink>)}</div></div>)}</nav></> : <p className="thinking-sidebar-caption">장을 선택하고 기본·심화 문제를 한 주제씩 학습하세요.</p>}</div>;
}

export function ThinkingLessonNav({ doc, related }: { doc: Document; related: DocumentMeta[] }) {
  const number = chapterNumber(doc), index = related.findIndex(d => d.route === doc.route);
  const pair = related.filter(d => topicNumber(d) === topicNumber(doc));
  return <div className="thinking-lesson-navigation"><div data-testid="thinking-current-chapter" className="thinking-lesson-breadcrumb"><Link to={'/thinking-lab/?chapter=' + number}><ArrowLeft size={14}/>학습 목차</Link><span>{number}장 · {chapterTitle(number)}</span><small>{index + 1} / {related.length}</small></div><nav className="thinking-lesson-pair" aria-label="현재 주제의 기본·심화"><span>{topicNumber(doc)} · {doc.topic}</span><div>{pair.map(d => <Link key={d.id} to={d.route} aria-current={d.route === doc.route ? 'page' : undefined}>{levelLabel(d)}</Link>)}</div></nav></div>;
}
