import type { DocumentMeta } from './types';
import { compareKnowledgeDocuments } from './knowledge-order';

export type LearningKind = 'knowledge' | 'game-design';
export type LearningGroup = { id: string; title: string; description: string; documents: DocumentMeta[] };
export const PAGE_SIZE = 12;
// Existing Knowledge outline order. Unknown categories are retained, never discarded.
const courseOrder = ['C 언어', 'C++ 기초·심화', 'Modern C++', 'Effective C++', 'Effective Modern C++', 'WinAPI', '게임 수학', 'DirectX 9', 'DirectX 11', 'DirectX 12', '그래픽스 기술', '컴퓨터 구조', '운영체제', '네트워크', '알고리즘', 'Python'];
const topicTitles: Record<string, string> = {
  'Design Specifics': '구체적 게임 설계', 'Game Concepts': '게임 컨셉', 'Planning & Game Design': '기획·게임 디자인',
  'Game Essence': '게임의 본질', UI: 'UI·정보 전달', 'Planning & Design': '기획·디자인',
};
export function gameTopic(doc: Pick<DocumentMeta, 'topic' | 'category'>): string {
  return doc.topic?.match(/\[([^\]]+)\]\s*$/)?.[1] || doc.category || '기타 주제';
}
export function isLearningRead(doc: Pick<DocumentMeta, 'route'>, reading: Record<string, boolean>): boolean {
  return reading[doc.route.replace(/^\//, '')] === true;
}
export function orderedLearningDocuments(docs: DocumentMeta[], kind: LearningKind): DocumentMeta[] {
  if (kind === 'game-design') return docs.slice().sort((a, b) => String(a.number || '').localeCompare(String(b.number || ''), 'en', { numeric: true }));
  return docs.slice().sort((a, b) => {
    if (a.category === b.category) return compareKnowledgeDocuments(a, b);
    const ai = courseOrder.indexOf(a.category), bi = courseOrder.indexOf(b.category);
    return (ai < 0 ? courseOrder.length : ai) - (bi < 0 ? courseOrder.length : bi);
  });
}
export function learningGroups(docs: DocumentMeta[], kind: LearningKind): LearningGroup[] {
  const ordered = orderedLearningDocuments(docs, kind), grouped = new Map<string, DocumentMeta[]>();
  for (const doc of ordered) {
    const id = kind === 'knowledge' ? doc.category || '기타 교재' : gameTopic(doc);
    grouped.set(id, [...(grouped.get(id) || []), doc]);
  }
  return [...grouped].map(([id, documents]) => ({ id, title: kind === 'knowledge' ? id : topicTitles[id] || id,
    description: kind === 'knowledge' ? '공개된 단원과 실습 자료를 과정 순서로 읽습니다.' : id, documents }));
}
export function learningPage(value: string | null, count: number): number {
  const requested = Number(value);
  return Math.min(Math.max(1, Number.isSafeInteger(requested) ? requested : 1), Math.max(1, Math.ceil(count / PAGE_SIZE)));
}
