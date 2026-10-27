/** One learning topic contains exactly two independently addressable pages. */
export function validateThinkingPairs(catalog, policy) {
 if(policy.publicationUnit!=='topic'||policy.pagesPerTopic!==2||policy.limit!=='one new topic (two separate problem pages) per Korean date')throw Error('Thinking pair publication policy mismatch');
 const groups=new Map(), paths=new Set(), numbers=new Set();
 const dateValid=d=>typeof d==='string'&&/^\d{4}-\d{2}-\d{2}$/.test(d)&&Number.isFinite(Date.parse(d+'T00:00:00Z'))&&new Date(d+'T00:00:00Z').toISOString().slice(0,10)===d;
 for(const e of catalog.episodes){
  const m=typeof e.number==='string'?/^([1-9]\d*)\.([1-9]\d*)\.([12])$/.exec(e.number):null;
  if(!m||Number(m[1])!==e.chapter||!dateValid(e.study_date)||!dateValid(e.published_date)||!e.topic_en?.trim()||typeof e.path!=='string'||!/^ep-[a-z0-9-]+\/$/.test(e.path)||numbers.has(e.number)||paths.has(e.path))throw Error('Invalid or duplicate thinking pair page: '+e.number);
  numbers.add(e.number);paths.add(e.path);
  const topic=m[1]+'.'+m[2];const pages=groups.get(topic)||[];pages.push(e);groups.set(topic,pages);
 }
 const dates=new Map();
 for(const [topic,pages]of groups){
  if(pages.length!==2||!pages.some(p=>p.number===topic+'.1')||!pages.some(p=>p.number===topic+'.2'))throw Error('Incomplete thinking pair: '+topic);
  if(pages[0].study_date!==pages[1].study_date||pages[0].published_date!==pages[1].published_date||pages[0].topic_en!==pages[1].topic_en)throw Error('Thinking pair metadata mismatch: '+topic);
  const date=pages[0].study_date;
  if(dates.has(date)&&dates.get(date)!==topic)throw Error('Multiple thinking topics on a learning date: '+date);
  dates.set(date,topic);
 }
 if(!/^[1-9]\d*\.[1-9]\d*$/.test(catalog.current.number)||!/^[1-9]\d*\.[1-9]\d*$/.test(catalog.next.number))throw Error('Thinking progress must count topics, not page suffixes');
 if(!groups.has(catalog.current.number)||groups.has(catalog.next.number))throw Error('Thinking current/next topic mismatch');
 const current=groups.get(catalog.current.number)[0];
 if(current.study_date!==catalog.current.study_date||current.study_date!==[...dates.keys()].sort().at(-1))throw Error('Thinking current date mismatch or stale topic');
 if(!dateValid(catalog.next.study_date)||catalog.next.study_date<=current.study_date)throw Error('Thinking next topic must follow current learning date');
 return {topics:groups.size,pages:catalog.episodes.length};
}
