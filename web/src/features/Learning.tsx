import { useEffect, useMemo, useRef, type ReactNode } from 'react';
import { Link, NavLink, useLocation, useNavigate, useSearchParams } from 'react-router-dom';
import { ArrowLeft, ArrowRight, Bookmark, BookOpen, Check, ChevronLeft, ChevronRight, RotateCcw, Search, SlidersHorizontal } from 'lucide-react';
import { useData } from '@/lib/data';
import { keys, objectBooleans, useStored } from '@/lib/storage';
import { matchesDocument, readLibraryFilter } from '@/lib/document-library';
import { gameTopic, isLearningRead, learningGroups, learningPage, orderedLearningDocuments, PAGE_SIZE, type LearningGroup, type LearningKind } from '@/lib/learning-navigation';
import { cleanTitle } from '@/lib/utils';
import { Button } from '@/components/ui/button';
import { Empty } from '@/components/Common';
import type { Document, DocumentMeta } from '@/lib/types';
import './learning.css';

const savedRoutes = (value: unknown): string[] => Array.isArray(value) ? value.filter((v): v is string => typeof v === 'string') : [];
const groupKey = (kind: LearningKind) => kind === 'knowledge' ? 'course' : 'topic';
const groupLabel = (kind: LearningKind) => kind === 'knowledge' ? '학습 과정' : '강의 주제';

export function LearningIndex({ kind, title, eyebrow, description, after }: { kind: LearningKind; title: string; eyebrow: string; description: string; after?: ReactNode }) {
  const { catalog } = useData();
  const docs = useMemo(() => orderedLearningDocuments(catalog.documents.filter(d => d.kind === kind), kind), [catalog, kind]);
  const groups = useMemo(() => learningGroups(docs, kind), [docs, kind]);
  const [stored] = useStored<unknown>(keys.reading, {}), reading = objectBooleans(stored);
  const [bookmarks] = useStored<unknown>(keys.gameDesignBookmarks, []), saved = savedRoutes(bookmarks);
  const [params, setParams] = useSearchParams(), filter = readLibraryFilter(params);
  const status = params.get('status') === 'unread' ? 'unread' : kind === 'game-design' && params.get('status') === 'saved' ? 'saved' : 'all';
  const done = docs.filter(d => isLearningRead(d, reading)).length;
  const visible = docs.filter(d => matchesDocument(d, filter) && (status === 'all' || (status === 'saved' ? saved.includes(d.route) : !isLearningRead(d, reading))));
  const visibleRoutes = new Set(visible.map(d => d.route));
  const allGroups: LearningGroup[] = kind === 'game-design' ? [{ id: 'all', title: '전체 강의', description: '원본 공개일 순서대로 한 강의씩 학습합니다.', documents: docs }, ...groups] : groups;
  const matching = allGroups.map(group => ({ ...group, documents: group.documents.filter(d => visibleRoutes.has(d.route)) })).filter(group => group.documents.length);
  const hasFilters = !!(filter.query || filter.category || filter.tags.length || status !== 'all');
  const suggested = docs.find(d => !isLearningRead(d, reading)) || docs[0];
  const requested = params.get(groupKey(kind));
  const selected = matching.find(group => group.id === requested) || (!hasFilters && kind === 'knowledge' ? matching.find(group => group.id === suggested?.category) : undefined) || matching[0];
  const fullGroup = allGroups.find(group => group.id === selected?.id);
  const next = fullGroup?.documents.find(d => !isLearningRead(d, reading)) || fullGroup?.documents[0] || suggested;
  const groupDone = fullGroup?.documents.filter(d => isLearningRead(d, reading)).length || 0;
  const groupAllDone = fullGroup ? groupDone === fullGroup.documents.length : docs.length > 0 && done === docs.length;
  const ordered = selected?.documents.slice().sort((a, b) => filter.sort === 'newest' ? b.date.localeCompare(a.date) : filter.sort === 'oldest' ? a.date.localeCompare(b.date) : 0) || [];
  const currentPage = learningPage(params.get('page'), ordered.length), pageCount = Math.max(1, Math.ceil(ordered.length / PAGE_SIZE));
  const pageDocs = ordered.slice((currentPage - 1) * PAGE_SIZE, currentPage * PAGE_SIZE);
  const categories = [...new Set(docs.map(d => d.category).filter(Boolean))].sort((a, b) => a.localeCompare(b, 'ko'));
  const tags = [...new Set([...docs.flatMap(d => d.tags), ...filter.tags])].sort((a, b) => a.localeCompare(b, 'ko'));
  const active = hasFilters || filter.sort !== 'number';
  const listHeading = useRef<HTMLElement>(null), focusNewPage = useRef(false);
  useEffect(() => {
    if (!focusNewPage.current) return;
    focusNewPage.current = false;
    listHeading.current?.focus({ preventScroll: true });
    listHeading.current?.scrollIntoView({ block: 'start', behavior: 'instant' });
  }, [currentPage]);
  function change(key: string, value: string, replace = false) {
    // A previous input can update history before React commits its new render.
    // Compose from the current URL so quick tag/sort changes cannot lose each other.
    const nextParams = new URLSearchParams(window.location.search);
    focusNewPage.current = key === 'page';
    if (value) nextParams.set(key, value); else nextParams.delete(key);
    if (key !== 'page') nextParams.delete('page');
    if (['q', 'category', 'tag', 'status'].includes(key)) nextParams.delete(groupKey(kind));
    setParams(nextParams, { replace });
  }
  function toggleTag(tag: string) {
    focusNewPage.current = false;
    const nextParams = new URLSearchParams(window.location.search), chosen = new Set(nextParams.getAll('tag'));
    if (chosen.has(tag)) chosen.delete(tag); else chosen.add(tag);
    ['tag', 'page', groupKey(kind)].forEach(key => nextParams.delete(key));
    [...chosen].sort().forEach(value => nextParams.append('tag', value)); setParams(nextParams);
  }
  function reset() { focusNewPage.current = false; const nextParams = new URLSearchParams(window.location.search); ['q', 'category', 'tag', 'sort', 'status', 'page', groupKey(kind)].forEach(key => nextParams.delete(key)); setParams(nextParams); }
  return <div className={'page learning-index ' + kind + '-index'}>
    <header className="learning-header"><div><p className="eyebrow">{eyebrow}</p><h1>{title}</h1><p className="page-description">{description}</p></div><div className="learning-stats"><span><strong>{groups.length}</strong>{kind === 'knowledge' ? '개 과정' : '개 주제'}</span><span><strong>{docs.length}</strong>{kind === 'knowledge' ? '개 공개 교재' : '개 공개 강의'}</span></div></header>
    {next && <section className="learning-start" aria-label="학습 시작"><div className="learning-start-icon"><BookOpen size={23}/></div><div className="learning-start-copy"><span className="eyebrow">{groupAllDone ? 'REVIEW' : done ? 'CONTINUE LEARNING' : 'START HERE'}</span><h2>{groupAllDone ? '학습을 마쳤어요. 다시 살펴볼까요?' : kind === 'knowledge' ? `${fullGroup?.title || next.category} · 다음 단원` : '다음 미완료 강의부터 이어 가세요.'}</h2><p>{kind === 'game-design' && next.number + ' · '}{cleanTitle(next.title)}</p></div><Button asChild><Link data-testid="learning-study-entry" to={next.route}>{groupAllDone ? '다시 읽기' : done ? '이어서 학습' : '학습 시작'}<ArrowRight size={16}/></Link></Button><div className="learning-progress" data-testid="learning-progress"><div><span>전체 학습 기록</span><strong>{done} / {docs.length} 완료</strong></div><progress value={done} max={docs.length || 1} aria-label={title + ' 전체 학습 진도'}/></div></section>}
    <section className="learning-explorer problem-library" id="problem-library" aria-labelledby="problem-library-heading">
      <div className="section-heading"><div><p className="eyebrow">CONTENTS</p><h2 id="problem-library-heading">{kind === 'knowledge' ? '과정별 학습 목차' : '강의 학습 목차'}</h2></div><span role="status" aria-live="polite" data-testid="library-count">{hasFilters && '검색 결과 · '}{visible.length} / {docs.length}개</span></div>
      <div className="learning-search-row"><label className="search-input"><Search size={17}/><input type="search" aria-label="문서 검색" value={filter.query} onChange={e => change('q', e.target.value, true)} placeholder={kind === 'knowledge' ? '과정, 단원, 키워드로 교재 찾기' : '회차, 주제, 키워드로 강의 찾기'}/></label><Button variant="outline" size="sm" disabled={!active} onClick={reset}><RotateCcw size={14}/>필터 초기화</Button></div>
      <div className="learning-controls"><div className="learning-status" role="group" aria-label="학습 상태 필터"><button type="button" aria-pressed={status === 'all'} onClick={() => change('status', '')}>전체 <span>{docs.length}</span></button><button type="button" aria-pressed={status === 'unread'} onClick={() => change('status', 'unread')}>미완료 <span>{docs.length - done}</span></button>{kind === 'game-design' && <button type="button" aria-pressed={status === 'saved'} onClick={() => change('status', 'saved')}><Bookmark size={12}/>북마크 <span>{docs.filter(d => saved.includes(d.route)).length}</span></button>}</div><details className="learning-filter-more" open={filter.tags.length > 0 || !!filter.category || filter.sort !== 'number' || undefined}><summary><SlidersHorizontal size={14}/>태그·상세 필터{(filter.tags.length > 0 || filter.category) && <span className="badge">{filter.tags.length + (filter.category ? 1 : 0)}</span>}</summary><div className="learning-filter-panel"><div className="problem-filter-row"><label><span>분류</span><select aria-label="문제 분류" value={filter.category} onChange={e => change('category', e.target.value)}><option value="">전체 분류</option>{categories.map(category => <option key={category}>{category}</option>)}{filter.category && !categories.includes(filter.category) && <option>{filter.category}</option>}</select></label><label><span>정렬</span><select aria-label="문제 정렬" value={filter.sort} onChange={e => change('sort', e.target.value)}><option value="number">{kind === 'knowledge' ? '과정 순서' : '회차 순서'}</option><option value="newest">학습일 최신순</option><option value="oldest">학습일 오래된순</option></select></label></div><fieldset className="problem-tags"><legend>태그 · 여러 개를 선택하면 모두 포함한 자료만 표시합니다.</legend><div className="problem-tag-list">{tags.map(tag => <button type="button" className="problem-tag" key={tag} aria-label={'태그 ' + tag} aria-pressed={filter.tags.includes(tag)} onClick={() => toggleTag(tag)}>{tag}</button>)}</div></fieldset><p className="problem-filter-caption">검색 조건은 주소에 저장됩니다. 교재·강의의 본문과 기존 기록은 바뀌지 않습니다.</p></div></details></div>
      {!selected ? <div className="learning-empty"><Empty title="조건에 맞는 자료가 없습니다." description="검색어와 태그를 줄이거나 전체 자료를 확인해 보세요."/><Button variant="outline" onClick={reset}>전체 자료 보기</Button></div> : <div className="learning-book-layout">
        <nav className="learning-groups" data-testid="learning-group-nav" aria-label={groupLabel(kind) + ' 목록'}><div className="learning-groups-label">{hasFilters ? '검색된 ' : ''}{kind === 'knowledge' ? '과정' : '주제'}<span>{matching.filter(group => group.id !== 'all').length}개</span></div>{matching.map(group => <button type="button" key={group.id} data-group={group.id} aria-current={selected.id === group.id ? 'true' : undefined} onClick={() => change(groupKey(kind), group.id)}><span>{group.title}<small>{group.documents.length}개 {kind === 'knowledge' ? '교재' : '강의'}</small></span><ChevronRight size={14}/></button>)}</nav>
        <div className="learning-group-content" data-testid="learning-current-group" data-group={selected.id}><label className="learning-mobile-group"><span>{groupLabel(kind)}</span><select aria-label={groupLabel(kind)} value={selected.id} onChange={e => change(groupKey(kind), e.target.value)}>{matching.map(group => <option key={group.id} value={group.id}>{group.title} ({group.documents.length})</option>)}</select></label><header className="learning-group-heading" ref={listHeading} tabIndex={-1} data-testid="learning-list-heading"><div><p className="eyebrow">{kind === 'knowledge' ? 'COURSE' : selected.id === 'all' ? 'ALL EPISODES' : 'TOPIC'}</p><h2>{selected.title}</h2><p>{selected.description}</p></div><span className="learning-group-progress">{groupDone} / {fullGroup?.documents.length} 완료</span></header>
          <ol className="learning-lesson-list" start={(currentPage - 1) * PAGE_SIZE + 1}>{pageDocs.map(doc => <li key={doc.id}><Link className={'learning-lesson-link' + (isLearningRead(doc, reading) ? ' is-complete' : '')} data-testid="library-card" to={doc.route}><span className="learning-lesson-number">{kind === 'game-design' ? doc.number : String((fullGroup?.documents.indexOf(doc) ?? 0) + 1).padStart(2, '0')}</span><span className="learning-lesson-copy"><strong>{cleanTitle(doc.title)}</strong><small>{doc.subtitle}</small>{kind === 'game-design' && <span className="learning-lesson-category">{doc.category}</span>}</span><span className="learning-lesson-state">{isLearningRead(doc, reading) ? <Check size={15} aria-label="학습 완료"/> : <ChevronRight size={16}/>}</span></Link></li>)}</ol>
          <footer className="learning-list-footer"><span>{(currentPage - 1) * PAGE_SIZE + 1}–{Math.min(currentPage * PAGE_SIZE, ordered.length)} / {ordered.length}개</span><nav aria-label="학습 목록 페이지"><button type="button" aria-label="이전 목록" disabled={currentPage === 1} onClick={() => change('page', String(currentPage - 1))}><ChevronLeft size={15}/></button><span>{currentPage} / {pageCount}</span><button type="button" aria-label="다음 목록" disabled={currentPage === pageCount} onClick={() => change('page', String(currentPage + 1))}><ChevronRight size={15}/></button></nav></footer>
        </div>
      </div>}
    </section>{after}
  </div>;
}

export function LearningSidebar({ docs, kind, onNavigate }: { docs: DocumentMeta[]; kind: LearningKind; onNavigate?: () => void }) {
  const { pathname, search } = useLocation(), navigate = useNavigate();
  const [stored] = useStored<unknown>(keys.reading, {}), reading = objectBooleans(stored);
  const groups = learningGroups(docs, kind), current = docs.find(doc => doc.route === pathname.replace(/\/?$/, '/'));
  const id = current ? kind === 'knowledge' ? current.category : gameTopic(current) : new URLSearchParams(search).get(groupKey(kind));
  const selected = groups.find(group => group.id === id);
  const listRef = useRef<HTMLElement>(null), activeRef = useRef<HTMLAnchorElement>(null);
  useEffect(() => {
    const list = listRef.current, active = activeRef.current;
    if (!list || !active) return;
    const center = () => { list.scrollTop = active.offsetTop - list.clientHeight / 2 + active.clientHeight / 2; };
    center(); const observer = new ResizeObserver(center); observer.observe(list); return () => observer.disconnect();
  }, [pathname, selected?.id]);
  return <div className="learning-sidebar"><div className="nav-label">{current ? kind === 'knowledge' ? 'CURRENT COURSE' : 'CURRENT TOPIC' : 'STUDY LIBRARY'}</div><Link className="learning-sidebar-home" to={'/' + kind + '/'} onClick={onNavigate}><BookOpen size={14}/>전체 학습 목차<ArrowRight size={13}/></Link><label className="learning-sidebar-select"><span className="sr-only">사이드바 {groupLabel(kind)}</span><select aria-label={'사이드바 ' + groupLabel(kind)} value={selected?.id || ''} onChange={e => { navigate('/' + kind + '/?' + groupKey(kind) + '=' + encodeURIComponent(e.target.value)); onNavigate?.(); }}><option value="">{kind === 'knowledge' ? '과정 선택' : '주제 선택'}</option>{groups.map(group => <option key={group.id} value={group.id}>{group.title} ({group.documents.length})</option>)}</select></label>{selected ? <nav className="chapter-nav learning-sidebar-list" data-testid="learning-sidebar-list" ref={listRef} aria-label={kind === 'knowledge' ? '현재 과정의 교재' : '현재 주제의 강의'}>{selected.documents.map((doc, index) => <NavLink ref={doc.route === current?.route ? activeRef : undefined} key={doc.id} to={doc.route} onClick={onNavigate}><span className="chapter-number">{kind === 'game-design' ? doc.number : String(index + 1).padStart(2, '0')}</span><span>{cleanTitle(doc.title)}</span>{isLearningRead(doc, reading) && <Check className="chapter-complete" size={13} aria-label="학습 완료"/>}</NavLink>)}</nav> : <p className="learning-sidebar-caption">{kind === 'knowledge' ? '과정을 선택하면 해당 교재만 한눈에 볼 수 있어요.' : '주제를 고르면 관련 강의를 모아 볼 수 있어요.'}</p>}</div>;
}

export function LearningLessonNav({ doc }: { doc: Document }) {
  const { catalog } = useData(), kind = doc.kind === 'knowledge' ? 'knowledge' : 'game-design';
  const groups = learningGroups(catalog.documents.filter(item => item.kind === kind), kind);
  const group = groups.find(item => item.id === (kind === 'knowledge' ? doc.category : gameTopic(doc)));
  if (!group) return null;
  const index = group.documents.findIndex(item => item.route === doc.route);
  const route = '/' + kind + '/?' + groupKey(kind) + '=' + encodeURIComponent(group.id) + '&page=' + (Math.floor(index / PAGE_SIZE) + 1);
  return <nav className="learning-lesson-navigation" aria-label="학습 위치"><Link to={route}><ArrowLeft size={14}/>학습 목차</Link><span>{group.title}</span><small>{index + 1} / {group.documents.length}</small></nav>;
}
