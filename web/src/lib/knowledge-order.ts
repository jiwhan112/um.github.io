import type { DocumentMeta } from './types';

// Keep course navigation in chapter order regardless of when source/content was migrated.
function dx11ChapterSortKey(doc: DocumentMeta): number {
  const slug=doc.route.match(/\/knowledge\/(dx11-[^/]+)\//)?.[1]??'';
  if(slug==='dx11-frustum')return 1500; // retained original prototype = chapter 15
  const match=slug.match(/^dx11-(sources-)?(\d+)(b)?(?:-\d+)?$/);
  if(!match)return Number.MAX_SAFE_INTEGER;
  // Main chapter, then its source; supplementary 13B follows 13 and its source.
  return Number(match[2])*100+(match[3]?20:0)+(match[1]?10:0);
}
export function compareKnowledgeDocuments(a: DocumentMeta,b: DocumentMeta):number {
  if(a.category!=='DirectX 11'||b.category!=='DirectX 11')return 0;
  return dx11ChapterSortKey(a)-dx11ChapterSortKey(b);
}

