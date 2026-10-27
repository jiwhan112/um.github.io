/** Sequential topic pairs, independent of daily briefing/language scheduling. */
export const koreanDate = (now = new Date()) => new Intl.DateTimeFormat('en-CA', {timeZone:'Asia/Seoul',year:'numeric',month:'2-digit',day:'2-digit'}).format(now);
const dateValid = d => typeof d==='string' && /^\d{4}-\d{2}-\d{2}$/.test(d) && Number.isFinite(Date.parse(d+'T00:00:00Z')) && new Date(d+'T00:00:00Z').toISOString().slice(0,10)===d;
const baseNumber = n => typeof n==='string' && /^[1-9]\d*\.[1-9]\d*$/.test(n);
export function validateThinkingPairs(catalog, policy, plan, asOfKstDate = koreanDate()) {
 if(!dateValid(asOfKstDate))throw Error('Invalid Korean as-of date');
 if(policy.publicationUnit!=='topic'||policy.pagesPerTopic!==2||policy.execution!=='continuous_user_requested'||policy.scheduleEnabled!==false||policy.cadence!=='continuous'||policy.curriculumManifest!=='automation/thinking-book-plan.json')throw Error('Thinking continuous publication policy mismatch');
 if(!dateValid(plan.approvedDate)||plan.approvedDate>asOfKstDate||plan.edition!==6||plan.pagesPerTopic!==2||plan.targetTopicCount!==189||!Array.isArray(plan.verifiedPrefix)||!plan.verifiedPrefix.length)throw Error('Invalid thinking curriculum manifest');
 const curriculum=new Map();let previous;
 for(const topic of plan.verifiedPrefix){
  const [chapter,problem]=String(topic.number).split('.').map(Number);
  if(!baseNumber(topic.number)||chapter!==topic.chapter||!topic.topic_en?.trim()||topic.verification!=='numbered_repository_metadata'||!topic.evidence?.length||curriculum.has(topic.number))throw Error('Unverified or duplicate curriculum topic');
  if(previous ? !(chapter===previous.chapter&&problem===previous.problem+1||chapter===previous.chapter+1&&problem===1) : topic.number!=='1.1')throw Error('Curriculum prefix has a numbering gap');
  for(const evidence of topic.evidence){const source=plan.sources.find(s=>s.id===evidence.source);if(!source||!/^https:\/\/github\.com\//.test(source.url)||!/^[a-f0-9]{40}$/.test(source.revision)||!evidence.path)throw Error('Missing pinned curriculum evidence');}
  curriculum.set(topic.number,topic);previous={chapter,problem};
 }
 if(plan.complete===true){if(curriculum.size!==189||plan.unresolved?.length||plan.blockedNext!=null||plan.unverifiedNumberedTopicCount!==0)throw Error('Premature full-book verification');}
 else {
  const blocked=plan.blockedNext,[chapter,problem]=String(blocked?.number).split('.').map(Number);
  const follows=chapter===previous.chapter&&problem===previous.problem+1||chapter===previous.chapter+1&&problem===1;
  if(plan.complete!==false||curriculum.size>=189||!baseNumber(blocked?.number)||blocked.chapter!==chapter||!['blocked_source_conflict','blocked_source_missing'].includes(blocked.status)||typeof blocked.reason!=='string'||!blocked.reason.trim()||!follows)throw Error('Incomplete curriculum must expose exact blocked frontier');
 }
 const baseline=new Map(plan.historicalBaseline.map(e=>[e.number,e]));
 if(baseline.size!==19||plan.historicalBaseline.length!==19||plan.verifiedPrefix.slice(0,19).some(t=>!baseline.has(t.number)))throw Error('Historical baseline is incomplete');
 const groups=new Map(),paths=new Set(),numbers=new Set();
 for(const e of catalog.episodes){
  const match=typeof e.number==='string'?/^([1-9]\d*)\.([1-9]\d*)\.([12])$/.exec(e.number):null;
  if(!match||Number(match[1])!==e.chapter||!dateValid(e.study_date)||!dateValid(e.published_date)||e.study_date>asOfKstDate||e.published_date>asOfKstDate||typeof e.path!=='string'||!/^ep-[a-z0-9-]+\/$/.test(e.path)||numbers.has(e.number)||paths.has(e.path))throw Error('Invalid or duplicate thinking pair page: '+e.number);
  const number=match[1]+'.'+match[2],topic=curriculum.get(number),historic=baseline.get(number);
  if(!topic||e.topic_en!==topic.topic_en)throw Error('Topic outside verified curriculum: '+number);
  if(historic ? e.study_date!==historic.study_date||e.published_date!==historic.published_date : e.study_date!==e.published_date||e.published_date<plan.approvedDate)throw Error('Thinking actual/historical date mismatch: '+number);
  numbers.add(e.number);paths.add(e.path);const pages=groups.get(number)||[];pages.push(e);groups.set(number,pages);
 }
 for(const [number,pages]of groups)if(pages.length!==2||!pages.some(p=>p.number===number+'.1')||!pages.some(p=>p.number===number+'.2')||pages[0].study_date!==pages[1].study_date||pages[0].published_date!==pages[1].published_date)throw Error('Incomplete or inconsistent thinking pair: '+number);
 if(groups.size<19||plan.verifiedPrefix.slice(0,groups.size).some(topic=>!groups.has(topic.number)))throw Error('Published topics must be a complete curriculum prefix');
 const last=plan.verifiedPrefix[groups.size-1],current=groups.get(last.number)[0];
 if(catalog.current?.number!==last.number||catalog.current.chapter!==last.chapter||catalog.current.topic_en!==last.topic_en||catalog.current.study_date!==current.study_date||catalog.current.published_date!==current.published_date)throw Error('Thinking current topic/date mismatch');
 const next=plan.verifiedPrefix[groups.size];
 if(next){if(catalog.next?.number!==next.number||catalog.next.chapter!==next.chapter||catalog.next.topic_en!==next.topic_en||catalog.next.status)throw Error('Thinking next must follow exact verified sequence');}
 else if(plan.complete){if(catalog.next!==null)throw Error('Completed curriculum must have null next');}
 else if(!catalog.next||['number','chapter','status','reason'].some(key=>catalog.next[key]!==plan.blockedNext[key]))throw Error('Unverified frontier must remain explicitly blocked, not complete');
 if(catalog.next&&('study_date'in catalog.next||'published_date'in catalog.next))throw Error('Thinking next must be undated');
 return {topics:groups.size,pages:catalog.episodes.length};
}
