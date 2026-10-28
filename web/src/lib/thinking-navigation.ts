import type { DocumentMeta } from './types';

// These are navigation labels, not replacements for the authored page metadata.
const chapterNames: Record<string, string> = {
  '1': '배열과 문자열', '2': '연결 리스트', '3': '스택과 큐', '4': '트리와 그래프',
  '5': '비트 조작', '6': '수학과 논리 퍼즐', '7': '객체 지향 설계', '8': '재귀와 동적 계획법',
  '9': '시스템 설계와 규모 확장', '10': '정렬과 탐색', '11': '테스트', '12': 'C와 C++',
  '13': 'Java', '14': '데이터베이스', '15': '스레드와 락', '16': '종합 문제', '17': '어려운 문제',
};
export type ThinkingTopic = { number: string; title: string; pages: DocumentMeta[] };
export type ThinkingChapter = { number: string; title: string; topics: ThinkingTopic[]; pages: DocumentMeta[] };
export const compareThinking = (a: DocumentMeta, b: DocumentMeta) => String(a.number || a.title).localeCompare(String(b.number || b.title), 'ko', { numeric: true });
export const chapterNumber = (doc: Pick<DocumentMeta, 'number'>) => String(doc.number || '').split('.')[0];
export const topicNumber = (doc: Pick<DocumentMeta, 'number'>) => String(doc.number || '').split('.').slice(0, 2).join('.');
export const chapterTitle = (number: string) => chapterNames[number] || '사고력 연습';
export const levelLabel = (doc: Pick<DocumentMeta, 'number'>) => doc.number?.split('.')[2] === '2' ? '심화' : '기본';
export const problemTitle = (doc: Pick<DocumentMeta, 'title'>) => doc.title.replace(/^\d+(?:\.\d+)*\s*/, '').replace(/^(기본|심화)\s*[—–-]\s*/, '');
export const isThinkingRead = (doc: Pick<DocumentMeta, 'route'>, reading: Record<string, boolean>) => !!reading[doc.route.replace(/^\//, '')];
export function thinkingChapters(docs: DocumentMeta[]): ThinkingChapter[] {
  const chapters = new Map<string, ThinkingChapter>();
  for (const doc of [...docs].sort(compareThinking)) {
    const number = chapterNumber(doc);
    if (!chapters.has(number)) chapters.set(number, { number, title: chapterTitle(number), topics: [], pages: [] });
    const chapter = chapters.get(number)!;
    const topicId = topicNumber(doc);
    let topic = chapter.topics.find(t => t.number === topicId);
    if (!topic) { topic = { number: topicId, title: doc.topic || problemTitle(doc), pages: [] }; chapter.topics.push(topic); }
    topic.pages.push(doc); chapter.pages.push(doc);
  }
  return [...chapters.values()];
}
