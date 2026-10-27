import type { DocumentMeta } from './types';
export type LibrarySort = 'number' | 'newest' | 'oldest';
export type LibraryFilter = { query: string; category: string; tags: string[]; sort: LibrarySort };
export const normalizeSearch = (value: string): string => value.normalize('NFKC').toLocaleLowerCase('ko-KR').replace(/\s+/g, ' ').trim();
export function matchesDocument(doc: DocumentMeta, filter: Pick<LibraryFilter, 'query' | 'category' | 'tags'>): boolean {
  const haystack = normalizeSearch([doc.number || '', doc.title, doc.subtitle, doc.topic || '', doc.category || '', ...doc.tags].join(' '));
  const words = normalizeSearch(filter.query).split(' ').filter(Boolean);
  return (!filter.category || doc.category === filter.category) && words.every(word => /^\d+(?:\.\d+)+$/.test(word)
    ? String(doc.number || '') === word || String(doc.number || '').startsWith(word + '.')
    : haystack.includes(word)) && filter.tags.every(tag => doc.tags.includes(tag));
}
export function filterDocuments(docs: DocumentMeta[], filter: LibraryFilter): DocumentMeta[] {
  return docs.filter(doc => matchesDocument(doc, filter)).sort((a, b) => {
    const byNumber = String(a.number || a.title).localeCompare(String(b.number || b.title), 'ko', { numeric: true });
    return filter.sort === 'newest' ? b.date.localeCompare(a.date) || -byNumber : filter.sort === 'oldest' ? a.date.localeCompare(b.date) || byNumber : byNumber;
  });
}
export function readLibraryFilter(params: URLSearchParams): LibraryFilter {
  const sort = params.get('sort');
  return { query: params.get('q') || '', category: params.get('category') || '', tags: [...new Set(params.getAll('tag').filter(Boolean))], sort: sort === 'newest' || sort === 'oldest' ? sort : 'number' };
}
