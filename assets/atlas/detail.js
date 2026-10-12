/* ATLAS NOTE detailed views. Moves existing nodes: original content, IDs and handlers survive. */
(() => {
'use strict';
const script = document.currentScript;
const root = new URL(script.dataset.siteRoot || '../../', script.src || document.baseURI);
const pageUrl = new URL(script.dataset.pageUrl || location.href, root);
const rel = pageUrl.pathname.startsWith(root.pathname) ? pageUrl.pathname.slice(root.pathname.length) : '';
const $ = (s, n=document) => n.querySelector(s);
const all = (s, n=document) => [...n.querySelectorAll(s)];
const el = (tag, cls='', text='') => { const e=document.createElement(tag); e.className=cls; if(text)e.textContent=text; return e; };
const url = p => new URL(p,root).href;
const link = (text, href, cls='') => {const a=el('a',cls,text);a.href=href;return a;};
const store = { get(k, fallback={}) {try {const v=JSON.parse(localStorage.getItem(k));return v&&typeof v==='object'?v:fallback;}catch{return fallback;}}, set(k,v) {try{localStorage.setItem(k,JSON.stringify(v));return true;}catch{return false;}} };
const icon = key => {const svg=document.createElementNS('http://www.w3.org/2000/svg','svg');svg.setAttribute('viewBox','0 0 24 24');svg.setAttribute('fill','none');svg.setAttribute('stroke','currentColor');svg.setAttribute('stroke-width','1.6');svg.setAttribute('aria-hidden','true'); const p=document.createElementNS(svg.namespaceURI,'path');p.setAttribute('d',({star:'M12 2l3 7 7 3-7 3-3 7-3-7-7-3 7-3z',home:'M3 10l9-7 9 7M5 9v12h5v-7h4v7h5V9',news:'M5 3h14v18H5zM8 7h8M8 11h8M8 15h5',book:'M3 4h7l2 2 2-2h7v16h-7l-2 2-2-2H3zM12 6v16',think:'M12 3l9 9-9 9-9-9zM9 12h6',lab:'M9 3h6M10 3v6L4 19a1 1 0 001 2h14a1 1 0 001-2L14 9V3M8 14h8',code:'M8 6l-6 6 6 6M16 6l6 6-6 6M14 3l-4 18'})[key]||'M4 4h16v16H4z');svg.append(p);return svg;};
const cleanHeading = s => s.replace(/^\s*\d+[.·]\s*/u,'').replace(/^[\p{Extended_Pictographic}\uFE0F\s]+/u,'').trim();
function textBlock(title, text) { const s=el('section');if(title)s.append(el('h2','',title));if(text)s.append(el('p','',text));return s; }
function toolbox(title) {const e=el('section','ad-toolbox');e.append(el('h2','',title));return e;}
function makeTabs(groups, host, onChange) {
 const nav=el('div','ad-tabs');nav.setAttribute('role','tablist');nav.setAttribute('aria-label','학습 내용');host.append(nav);
 groups.forEach((g,i)=>{g.node.id=g.node.id||'ad-panel-'+g.id;g.node.setAttribute('role','tabpanel');g.node.setAttribute('aria-labelledby','ad-tab-'+g.id);g.btn=el('button','',g.title);g.btn.type='button';g.btn.id='ad-tab-'+g.id;g.btn.setAttribute('role','tab');g.btn.setAttribute('aria-controls',g.node.id);nav.append(g.btn);g.btn.addEventListener('click',()=>select(g.id));g.btn.addEventListener('keydown',e=>{let j=i;if(e.key==='ArrowRight')j=(i+1)%groups.length;else if(e.key==='ArrowLeft')j=(i+groups.length-1)%groups.length;else if(e.key==='Home')j=0;else if(e.key==='End')j=groups.length-1;else return;e.preventDefault();select(groups[j].id);groups[j].btn.focus();});});
 function select(id){groups.forEach(g=>{const yes=g.id===id;g.node.hidden=!yes;g.btn.setAttribute('aria-selected',String(yes));g.btn.tabIndex=yes?0:-1;});onChange?.(id);}
 select(groups[0].id);return {select,nav};
}
function staticView(){
 const main=$('body>main');if(!main||main.dataset.atlasMounted)return;main.dataset.atlasMounted='true';
 if(main.dataset.atlasMarkdown){const nodes=[...main.childNodes],head=el('header','hero');main.append(head);let section=null;for(const n of nodes){if(n.nodeType===1&&n.tagName==='H2'){section=el('section');main.append(section);}if(section)section.append(n);else head.append(n);}}
 const kind=rel.startsWith('thinking-lab/')?'thinking':rel.startsWith('ai-asset-lab/')?'asset':'opensource';
 document.body.classList.add('ad-page');if(kind==='asset')document.body.classList.add('ad-light','ad-asset');
 // The v1 theme was an override layer, not a detail layout. Remove only that layer.
 all('link[href*="atlas-lesson.css"]').forEach(x=>x.remove());
 const sectionRoot=kind==='thinking'?'thinking-lab/':kind==='asset'?'ai-asset-lab/':'opensource-lab/';
 const sectionName={thinking:'사고력 훈련',asset:'AI 리소스 실험실',opensource:'오픈소스 아카이브'}[kind];
 const shell=el('div','ad-shell'),side=el('aside','ad-side'),frame=el('div','ad-frame');side.setAttribute('aria-label','사이트와 학습 목차');
 const brand=link('ATLAS NOTE',url(''),'ad-brand');brand.prepend(icon('star'));side.append(brand);
 const nav=el('nav','ad-site-nav');nav.setAttribute('aria-label','사이트 메뉴');
 [['home','홈',''],['news','데일리 브리핑','daily-briefing/'],['book','언어 학습','language-study/'],['think','사고력 훈련','thinking-lab/'],['lab','AI 리소스 실험실','ai-asset-lab/'],['code','오픈소스 보관함','opensource-lab/']].forEach(([i,t,p])=>{const a=link(t,url(p));a.prepend(icon(i));if(p===sectionRoot)a.setAttribute('aria-current','page');nav.append(a);});
 side.append(nav,el('span','ad-label',kind==='thinking'?'CHAPTER 01 · 문제 목록':kind==='asset'?'EXPERIMENT · 실험 자료':'ARCHIVE · 분석 목록'));
 const chapters=el('nav','ad-chapters');chapters.setAttribute('aria-label','하위 문서');side.append(chapters,el('div','ad-side-foot','ATLAS NOTE / LEARNING ARCHIVE'));
 const menu=el('button','ad-menu-toggle','문서 목록');menu.type='button';menu.setAttribute('aria-expanded','false');menu.addEventListener('click',()=>{const opened=side.classList.toggle('ad-open');menu.setAttribute('aria-expanded',String(opened));});side.append(menu);
 const top=el('header','ad-topbar'),bread=el('div','ad-breadcrumb');bread.append(link(sectionName,url(sectionRoot)),el('span','','/'),el('b','',kind==='thinking'?'문제 풀이':kind==='asset'?'실험 기록':'기술 문서'));top.append(bread,el('span','ad-status','ATLAS NOTE · 상세 보기'));
 const body=el('div','ad-body'),tools=el('aside','ad-tools');tools.setAttribute('aria-label','읽기 도구');
 main.before(shell);shell.append(side,frame);frame.append(top,body);main.classList.add('ad-document');body.append(main,tools);
 all(':scope>nav,:scope>.jump,:scope>.links,:scope>.anchor-nav',main).forEach(n=>n.classList.add('ad-legacy'));
 const header=$(':scope>header',main),footer=$(':scope>footer',main);const nextprev=el('div','ad-nextprev');
 if(footer)footer.before(nextprev);else main.append(nextprev);
 const sections=all(':scope>section,:scope>details',main);sections.forEach((s,i)=>{if(!s.id)s.id='ad-section-'+(i+1);});
 // Keep the very same textarea and save button, so legacy storage keys and listeners remain valid.
 const memo=$('textarea#memo',main);let codeSection=null;
 if(memo){codeSection=memo.closest('section');const editor=el('section','ad-toolbox ad-editor');editor.id='ad-editor';editor.append(el('div','ad-editor-head','내 풀이 · C++11'));editor.append(memo);const actions=el('div','ad-editor-actions');const save=$('#save');if(save)actions.append(save);const status=$('#save-msg')||$('#save-status');if(status)actions.append(status);editor.append(actions,el('p','ad-editor-actions','현재 브라우저에만 저장 · 코드 실행 기능 없음'));tools.append(editor);}
 const groups=[];
 if(kind==='thinking'){
  const bucket={problem:[],hints:[],solution:[],more:[]};
  sections.forEach((s,i)=>{let t=cleanHeading($('h2',s)?.textContent||$('summary',s)?.textContent||'');let b='more';
   if(/^(problem|challenge)$/.test(s.id)||i===0||/핵심 개념/.test(t))b='problem';
   else if(s.id==='hints'||/힌트|직접 생각|스스로 풀기/.test(t))b='hints';
   else if(/^(solution|tests|visual)$/.test(s.id)||/정답|모범|검증|테스트|원리 시각/.test(t))b='solution';
   if(s===codeSection){if($('details',s))b='solution';else {s.classList.add('ad-legacy');return;}}
   bucket[b].push(s);
  });
  for(const [id,title] of [['problem','문제'],['hints','힌트'],['solution','풀이·검증'],['more','확장·다음']]){if(!bucket[id].length)continue;const node=el('div','ad-panel');bucket[id].forEach(s=>node.append(s));groups.push({id,title,node});}
 }else if(kind==='asset'&&$('.asset-gallery',main)){
  sections.forEach((s,i)=>{groups.push({id:'asset-'+i,title:['결과물','크기 비교','실험 과정','평가','자료'][i]||'기록',node:s});});
 }
 let controller=null;
 if(groups.length){const holder=el('div');header?header.after(holder):main.prepend(holder);controller=makeTabs(groups,holder);groups.forEach(g=>holder.after(g.node)); // reinsert in deterministic order below tabs
  let last=holder;groups.forEach(g=>{last.after(g.node);last=g.node;});}
 const toc=toolbox(kind==='thinking'?'문제 살펴보기':'이 문서의 목차'),tocnav=el('nav');toc.append(tocnav);tools.append(toc);
 sections.forEach(s=>{if(s.classList.contains('ad-legacy'))return;const h=$('h2',s)||$('summary',s);if(!h)return;const a=link(cleanHeading(h.textContent),'#'+s.id);tocnav.append(a);});
 if(memo)tocnav.append(link('직접 코딩 / 내 풀이','#ad-editor'));
 function revealHash(){let id;try{id=decodeURIComponent(location.hash.slice(1));}catch{return;}const target=document.getElementById(id);if(!target)return;const group=groups.find(g=>g.node===target||g.node.contains(target));if(group)controller?.select(group.id);if(id==='ad-editor'&&innerWidth>1000){memo.focus();return;}requestAnimationFrame(()=>target.scrollIntoView({block:'start'}));}
 window.addEventListener('hashchange',revealHash);tocnav.addEventListener('click',e=>{const a=e.target.closest('a');if(a&&a.hash===location.hash)revealHash();});
 if(location.hash)revealHash();
 if(kind==='asset'){
  const ep=rel.split('/').slice(0,4).join('/')+'/';
  [['실험 결과',ep],['연구 보고서',ep+'report/'],['프롬프트 원문',ep+'prompts/prompts-v1/']].forEach(([t,p])=>{const a=link(t,url(p));if(new URL(a.href).pathname===pageUrl.pathname)a.setAttribute('aria-current','page');chapters.append(a);});
  const info=toolbox('실험 메모');info.append(el('p','','결과물·검증 범위·실패 기록을 함께 확인하세요. 새 실험은 사용자 요청으로만 진행합니다.'));tools.append(info);
  const imgs=all('.asset-gallery .asset img',main);if(imgs.length){const preview=el('div','ad-preview-tools','미리보기 배경');[['dark','어둡게'],['checker','투명 체크']].forEach(([id,t])=>{const b=el('button','',t);b.type='button';b.addEventListener('click',()=>document.body.classList.toggle('ad-checker',id==='checker'));preview.append(b);});$('.asset-gallery',main).before(preview);
   const dialog=el('dialog'),close=el('button','ad-action ad-secondary','닫기'),large=el('img'),caption=el('p');dialog.setAttribute('aria-label','포션 이미지 확대');close.type='button';close.addEventListener('click',()=>dialog.close());dialog.append(close,large,caption);document.body.append(dialog);
   imgs.forEach(img=>{const b=el('button','ad-image-button');b.type='button';b.setAttribute('aria-label',(img.alt||'리소스')+' 확대');img.before(b);b.append(img);b.addEventListener('click',()=>{large.src=img.src;large.alt=img.alt;caption.textContent=(img.alt||'포션')+' · 원본 '+img.naturalWidth+'×'+img.naturalHeight+' px · 화면에서 2배 확대';dialog.showModal();});});
  }
 }else{
  fetch(url(sectionRoot+'data/catalog.json'),{cache:'no-store'}).then(r=>{if(!r.ok)throw Error();return r.json();}).then(c=>{const eps=(c.episodes||[]).filter(x=>/^ep-[a-z0-9-]+\/$/.test(x.path||''));eps.sort((a,b)=>String(a.number).localeCompare(String(b.number),undefined,{numeric:true}));const current=eps.findIndex(e=>pageUrl.pathname===new URL(url(sectionRoot+e.path)).pathname);eps.forEach((e,i)=>{const a=link(e.number+' · '+(e.topic_en||e.title),url(sectionRoot+e.path));if(i===current)a.setAttribute('aria-current','page');chapters.append(a);});for(const [offset,t] of [[-1,'이전 문서'],[1,'다음 문서']]){const e=eps[current+offset];if(current<0||!e)continue;const a=link(e.number+' · '+(e.topic_en||e.title),url(sectionRoot+e.path));a.prepend(el('small','',t));nextprev.append(a);}}).catch(()=>chapters.append(link('전체 목록 보기',url(sectionRoot))));
 }
 const done=toolbox('내 학습 기록'),state=store.get('atlas-reading-v1'),key=rel;const label=el('label','ad-progress-check'),cb=el('input');cb.type='checkbox';cb.checked=!!state[key];label.append(cb,document.createTextNode('이 문서 학습 완료'));const notice=el('p','ad-read-status','이 브라우저에만 저장됩니다.');cb.addEventListener('change',()=>{state[key]=cb.checked;notice.textContent=store.set('atlas-reading-v1',state)?'저장했습니다.':'저장하지 못했습니다. 브라우저 설정을 확인하세요.';});done.append(label,notice);tools.append(done);
 all('pre',main).forEach(pre=>{const b=el('button','ad-action ad-secondary','코드 복사');b.type='button';b.style.marginBottom='8px';pre.before(b);b.addEventListener('click',async()=>{try{await navigator.clipboard.writeText(pre.textContent);b.textContent='복사됨';}catch{b.textContent='코드를 선택해서 복사하세요';}});});
 document.dispatchEvent(new CustomEvent('atlas-detail-ready'));
}
let languageCatalog=null, languageGroups=[], languageDone={}, languageKey='';
function leaveLanguage(){document.body.classList.remove('ad-language');$('.ad-lang-chapters')?.remove();}
function initLanguage(catalog){languageCatalog=catalog;window.addEventListener('hashchange',openLanguageHash);openLanguageHash();}
function openLanguageHash(){const m=location.hash.match(/^#(english|japanese)\/(\d+)$/);if(!languageCatalog)return;if(!m){const t=location.hash.slice(1);if(['home','english','japanese','review'].includes(t))window.route(t);return;}const item=languageCatalog[m[1]]?.find(x=>x.id===Number(m[2]));if(item)window.openLesson(m[1],item.file);}
function example(row,lang){const card=el('div','card'),foreign=String(row[lang==='english'?'en':'jp']??'');const line=el('div','lang',foreign),pron=el('div','pron',row.pron||''),ko=el('div','ko',row.ko||''),actions=el('div','row');card.append(line,pron,ko,actions);
 for(const [title,rate,repeat] of [['듣기',.96,1],['천천히',.78,1],['3회 반복',.9,3]]){const b=el('button',title==='듣기'?'':'ghost',title);b.type='button';b.setAttribute('aria-label',title+': '+foreign);b.addEventListener('click',()=>{if(!('speechSynthesis' in window)){b.textContent='음성 미지원';return;}window.speak(foreign,lang==='english'?'en-US':'ja-JP',rate,repeat);});actions.append(b);}
 for(const [name,node] of [['발음',pron],['해석',ko]]){const b=el('button','ghost',name);b.type='button';b.setAttribute('aria-expanded','false');b.addEventListener('click',()=>b.setAttribute('aria-expanded',String(node.classList.toggle('show'))));actions.append(b);}return card;}
function language(lang,d){
 if(!languageCatalog)return;document.body.classList.add('ad-language');languageKey=lang+':'+d.id;languageDone=store.get('atlas-language-sections-v1');
 const hash='#'+lang+'/'+d.id;if(location.hash!==hash)history.pushState(null,'',hash);
 const app=$('#app');app.replaceChildren();const layout=el('div','ad-lang-layout'),doc=el('article','ad-lang-document'),aside=el('aside','ad-lang-aside');app.append(layout);layout.append(doc,aside);
 const switcher=el('div','ad-lesson-switcher');switcher.append(link('← '+(lang==='english'?'영어':'일본어')+' 레슨 목록','#'+lang));switcher.firstChild.addEventListener('click',e=>{e.preventDefault();window.route(lang);});switcher.append(el('span','','LESSON '+String(d.id).padStart(2,'0')+' / '+d.date));doc.append(switcher);
 const header=el('header','ad-lang-head');header.append(el('div','ad-eyebrow','언어 학습 / '+(lang==='english'?'English':'日本語')),el('h1','',d.title));for(const t of lang==='english'?['회화 '+d.conversation_level,'읽기 '+d.reading_level]:[d.grammar_level])header.append(el('span','pill',t));doc.append(header);
 const groups=[];const add=(id,title)=>{const node=el('section','ad-lang-panel');node.id='ad-lang-'+id;groups.push({id:'lang-'+id,title,node});return node;};
 const cards=(n,arr,render)=>{(arr||[]).forEach(x=>n.append(render(x)));};
 if(lang==='english'){
  let n=add('conversation','회화');n.append(el('h2','','Conversation'),el('p','muted',String(d.source_work||'')+' · 학습용 재구성'));cards(n,d.conversation,x=>example(x,lang));
  n=add('expressions','핵심 표현');n.append(el('h2','','Expressions'));cards(n,d.expressions,x=>textBlock(x.text,x.meaning));cards(n,d.speaking_points,x=>textBlock(x.title,x.explanation));
  n=add('reading','읽기');n.append(el('h2','','Reading'));if(d.article){const c=el('div','card');c.append(el('b','',d.article.title||''),el('p','muted',[d.article.publisher,d.article.date].filter(Boolean).join(' · ')));try{const u=new URL(d.article.url);if(['https:','http:'].includes(u.protocol)){const a=link('원문 기사 ↗',u.href);a.target='_blank';a.rel='noopener noreferrer';c.append(a);}}catch{}n.append(c);}cards(n,d.reading,x=>example(x,lang));
  n=add('vocabulary','어휘');n.append(el('h2','','Vocabulary'));cards(n,d.vocabulary,x=>textBlock(x.term,x.meaning));cards(n,d.useful_expressions,x=>textBlock(x.text,x.meaning));
  if(d.reading_check?.length){n=add('check','이해 확인');n.append(el('h2','','Reading Check'));d.reading_check.forEach((x,i)=>{const c=el('details','card');c.append(el('summary','',(i+1)+'. '+x.q),el('p','',x.a));n.append(c);});}
 }else{
  let n=add('grammar','문법'),g=el('div','ad-grammar');g.append(el('h2','',d.grammar?.title||'문법'),el('p','',d.grammar?.explanation||''));n.append(g,el('p','muted','예문 탭에서 오늘의 문법을 직접 읽고 들어보세요.'));
  n=add('examples','예문');n.append(el('h2','','문장으로 연습하기'));cards(n,d.examples,x=>example(x,lang));
  n=add('kana','문자');n.append(el('h2','','오늘의 문자'));const grid=el('div','kana-grid');(d.kana||[]).forEach(x=>{const jp=typeof x==='string'?x:x.jp,c=el('div','kana-card');c.append(el('div','kana-jp',jp),el('div','kana-pron',typeof x==='string'?'':x.pron||''),el('div','kana-romaji',typeof x==='string'?'':x.romaji||''));const b=el('button','ghost','듣기');b.type='button';b.addEventListener('click',()=>{if('speechSynthesis' in window)window.speak(jp,'ja-JP');});c.append(b);grid.append(c);});n.append(grid);
  n=add('words','단어');n.append(el('h2','','Vocabulary'));cards(n,d.words,x=>example(x,lang));
 }
 if(d.review_reuse?.length){const n=add('review','이전 표현');n.append(el('h2','','이전 레슨에서 다시 보기'));cards(n,d.review_reuse,x=>textBlock(x.text||x.term||'',x.meaning||''));}
 languageGroups=groups;const tabsHost=el('div');doc.append(tabsHost);let controller;const toc=toolbox('이번 레슨'),tocnav=el('nav');toc.append(tocnav);
 controller=makeTabs(groups,tabsHost,id=>{all('button',tocnav).forEach(b=>b.setAttribute('aria-current',b.dataset.id===id?'step':'false'));});groups.forEach(g=>doc.append(g.node));
 groups.forEach(g=>{const b=el('button','ad-action ad-secondary',g.title);b.type='button';b.dataset.id=g.id;b.addEventListener('click',()=>controller.select(g.id));tocnav.append(b);});aside.append(toc);controller.select(groups[0].id);
 const progress=toolbox('학습 체크'),status=el('p','muted'),bar=el('div','ad-lang-progress'),fill=el('i');bar.append(fill);progress.append(status,bar);
 const checked=()=>{const n=groups.filter(g=>languageDone[languageKey+':'+g.id]).length;status.textContent=n+' / '+groups.length+' 단계 완료';fill.style.width=n/groups.length*100+'%';};
 groups.forEach(g=>{const l=el('label','ad-progress-check'),c=el('input');c.type='checkbox';c.checked=!!languageDone[languageKey+':'+g.id];c.addEventListener('change',()=>{languageDone[languageKey+':'+g.id]=c.checked;if(!store.set('atlas-language-sections-v1',languageDone))status.textContent='저장할 수 없습니다.';else checked();});l.append(c,document.createTextNode(g.title));progress.append(l);});progress.append(el('small','','완료한 단계에 직접 표시하세요. 체크는 이 브라우저에만 저장됩니다.'));aside.append(progress);checked();
 const guide=toolbox('듣고, 이해하고, 다시 읽기');guide.append(el('p','muted','발음과 해석은 기본 숨김입니다. 먼저 원문을 읽고 필요한 정보만 펼쳐 보세요.'));aside.append(guide);
 const items=(languageCatalog[lang]||[]).slice().sort((a,b)=>a.id-b.id),i=items.findIndex(x=>x.id===d.id),footer=el('div','ad-nextprev');for(const [j,t] of [[i-1,'이전 레슨'],[i+1,'다음 레슨']]){if(!items[j])continue;const a=link('Lesson '+items[j].id+' · '+items[j].title,'#'+lang+'/'+items[j].id);a.prepend(el('small','',t));footer.append(a);}doc.append(footer);
 $('.ad-lang-chapters')?.remove();const chapters=el('nav','ad-lang-chapters');chapters.setAttribute('aria-label','언어 레슨 목록');chapters.append(el('span','ad-label','LESSONS · 회차별 학습'));const select=el('select');select.setAttribute('aria-label','언어 선택');[['english','English'],['japanese','日本語']].forEach(([v,t])=>{const o=el('option','',t);o.value=v;select.append(o);});select.value=lang;select.addEventListener('change',()=>{const last=languageCatalog[select.value]?.slice().sort((a,b)=>b.id-a.id)[0];if(last)location.hash=select.value+'/'+last.id;});chapters.append(select);items.forEach(x=>{const a=link(x.id+'. '+x.title,'#'+lang+'/'+x.id);if(x.id===d.id)a.setAttribute('aria-current','page');chapters.append(a);});$('.atlas-side')?.append(chapters);window.scrollTo(0,0);
}
window.AtlasDetail={initLanguage,language,leaveLanguage};
if(!rel.startsWith('language-study/'))staticView();
})();
