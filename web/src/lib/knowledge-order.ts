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
  if(a.category!==b.category)return 0;
  if(a.category==='DirectX 11')return dx11ChapterSortKey(a)-dx11ChapterSortKey(b);
  if(a.category==='C++ 기초·심화')return cppChapterSortKey(a)-cppChapterSortKey(b);
  return 0;
}

function cppChapterSortKey(doc: DocumentMeta): number {
  const match=doc.route.match(/^\/knowledge\/cpp-(basics|advanced)-(\d+)\/$/);
  if(!match)return Number.MAX_SAFE_INTEGER;
  // 기본 개념을 학습한 뒤 심화로 이어 간다. URL의 알파벳 순서를 사용하지 않는다.
  return (match[1]==='advanced'?10000:0)+Number(match[2]);
}

