import { normalizeSearch } from './document-library';

/** Search metadata, not answer bodies; the displayed title remains unchanged. */
export function matchesSearchRow(row: { title: string; topic?: string }, query: string): boolean {
  return normalizeSearch([row.title, row.topic || ''].join(' ')).includes(normalizeSearch(query));
}
