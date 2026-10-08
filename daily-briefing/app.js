/* Daily briefing article reader – also supports historic markdown archives. */
(() => {
"use strict";
const $ = s => document.querySelector(s);
const esc = x => String(x ?? "").replace(/[&<>"']/g,c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
const cats={"game-dev":"게임 개발 · 엔진","game-industry":"게임 산업","ai":"AI · LLM · 에이전트","opensource":"오픈소스 · 기술 자료"};
const emoji={"game-dev":"🎮","game-industry":"🕹️","ai":"🤖","opensource":"🧩"};
const storageKey="jiwhan-briefing-v1";
let stored={saved:[],read:[]};
try { let v=JSON.parse(localStorage.getItem(storageKey)||"{}"); for(const k of ["saved","read"])if(Array.isArray(v[k])) stored[k]=v[k]; } catch(e){}
let days=[],articles=[],selectedDate="",view="daily",category="all",openId=null,tab="easy",limit=25,failed=0;
const ident=(day,index)=>day+":"+index;
const persist=()=>{try{localStorage.setItem(storageKey,JSON.stringify(stored));}catch(e){}};
function safeUrl(url){const s=String(url||"").trim();return /^https?:\/\/[^\s<>"\x27]+$/i.test(s)?s:"";}
function plain(s){return String(s??"").replace(/(?:cite|url|memcite).*?/gu,"").replace(/!\[[^\]]*\]\([^)]+\)/g,"").replace(/\[([^\]]+)\]\((https?:\/\/[^)\s]+)\)/g,"$1").replace(/\*\*([\s\S]*?)\*\*/g,"$1").replace(/\x60([^\x60]+)\x60/g,"$1").replace(/<\/?[^>]+>/g,"").replace(/[ \t]+/g," ").replace(/\s*\n\s*/g," ").trim();}
function clip(s,n=600){let x=plain(s);if(x.length<=n)return x;let z=x.slice(0,n),i=Math.max(z.lastIndexOf("다. "),z.lastIndexOf(". "),z.lastIndexOf("요. "));return (i>n*.5?z.slice(0,i+1):z.trimEnd())+"…";}
function sentence(s,n=2){return (plain(s).match(/[^.!?。]+(?:[.!?。]+|$)/g)||[]).slice(0,n).join(" ").trim();}
const dict=[
["Agent","에이전트: AI가 도구를 실행하고 결과를 확인하며 여러 단계를 스스로 진행하는 프로그램."],
["MCP","MCP: AI가 외부 앱·엔진 기능을 표준 방식으로 호출하게 하는 연결 규격."],
["Harness","하네스: AI의 실행 도구, 권한, 테스트, 재시도, 로그 등을 관리하는 주변 시스템."],
["Sandbox","샌드박스: 파일·네트워크·권한 접근을 제한해 작업을 격리하는 실행 환경."],
["Rust","러스트: C++처럼 빠르게 실행되면서 메모리 소유권을 컴파일러가 강하게 검사하는 언어."],
["DX12","DX12: GPU 메모리와 명령 실행을 개발자가 명시적으로 관리하는 그래픽 API."],
["Vulkan","Vulkan: 여러 플랫폼에서 사용할 수 있는 저수준 그래픽 API."],
["Shader","셰이더: GPU에서 렌더링·병렬 처리를 수행하는 프로그램."],
["셰이더","셰이더: GPU에서 그래픽스 계산을 수행하는 프로그램."],
["Ray Tracing","레이 트레이싱: 빛의 진행 경로를 추적해 반사·그림자·조명을 계산하는 기법."],
["Path Tracing","패스 트레이싱: 빛의 여러 반사 경로를 계산하는 사실적인 조명 방식."],
["LLM","LLM: 대량의 글·코드를 학습해 언어를 이해하고 생성하는 대형 언어 모델."],
["MoE","MoE: 여러 신경망 전문가 중 일부만 선택해 계산하는 모델 구조."],
["SDK","SDK: 플랫폼 기능을 개발할 때 사용하는 라이브러리·도구·문서의 묶음."],
["API","API: 프로그램이 다른 프로그램의 기능을 호출할 때 이용하는 약속된 인터페이스."],
["CI","CI: 코드 변경 때 자동으로 빌드·검사·테스트하는 시스템."],
["LOD","LOD: 거리에 따라 오브젝트의 표현 정밀도를 바꾸는 최적화 기법."],
["PSO","PSO: GPU 파이프라인의 셰이더와 고정 상태를 미리 묶어 놓은 객체."],
["BVH","BVH: 충돌·광선 검사를 빠르게 하기 위한 공간 계층 구조."],
["VRAM","VRAM: 그래픽카드가 텍스처와 버퍼를 보관하는 메모리."],
["GPU","GPU: 대량의 그래픽·병렬 계산을 처리하는 프로세서."],
["AOT","AOT: 프로그램을 실행하기 전에 기계어로 컴파일하는 방식."],
["LSP","LSP: 코드 편집기가 자동완성·정의 찾기 정보를 언어 서버에 요청하는 규격."],
["Serialization","직렬화: 객체의 상태를 파일·네트워크에 저장할 수 있는 형태로 바꾸는 과정."],
["오픈소스","오픈소스: 소스 코드를 공개하고 라이선스에 따라 활용·수정을 허용하는 소프트웨어."]
];
function terms(x){let t=x.toLowerCase(),out=[];for(const [k,v] of dict){if(t.includes(k.toLowerCase())&&!out.includes(v))out.push(v);if(out.length===3)break;}return out;}
function extract(md){
  const all=String(md||""),matches=[...all.matchAll(/^##\s+(\d+)\.\s*(.+)$/gm)],out=[];
  for(let i=0;i<matches.length;i++){
    const m=matches[i];let text=all.slice(m.index+m[0].length,i+1<matches.length?matches[i+1].index:all.length);
    text=text.split(/\n(?:---\s*\n)?###\s*🔍/)[0];
    out[Number(m[1])-1]=parseSection(text,m[2]);
  }return out;
}
function parseSection(section,title){
 const links=[];for(const m of section.matchAll(/\[([^\]]+)\]\((https?:\/\/[^)\s]+)\)/g)){let u=safeUrl(m[2]);if(u&&!links.some(x=>x.url===u))links.push({label:plain(m[1]),url:u});}
 let groups={general:[],technical:[],impact:[]},place="general",code=false,buf=[];
 function flush(){let p=plain(buf.join(" "));buf=[];if(p.length<35||/^(?:원문|출처|GitHub|관련 자료|공식 발표|기술 자료|논문|보도|릴리스)\s*[:：]/.test(p)||/^(?:발표|상태)\s*[:：]/.test(p))return;groups[place].push(p);}
 for(const line of section.split(/\r?\n/)){
   let s=line.trim();
   if(/^(?:\x60{3,}|~{3,})/.test(s)){flush();code=!code;continue;}if(code)continue;
   if(/^###\s/.test(s)){flush();let h=s.replace(/^###\s+/,"");place=/개발자|왜 중요|엔진 관점|의미|적용/.test(h)?"impact":/기술|구조|작동|원리|성능|배경|차이|핵심/.test(h)?"technical":"general";continue;}
   if(!s||/^---+$/.test(s)||/^!\[/.test(s)||/^(?:원문|GitHub|공식 발표|관련 자료|기술 자료|논문|보도|출처|릴리스)[:：]/.test(s)){flush();continue;}
   buf.push(s);
 }flush();
 if(!groups.impact.length&&groups.general.length>1){let g=groups.general;let ix=g.findIndex((p,i)=>i>0&&/개발자 관점|게임 개발자|엔진 개발자|왜 중요/.test(p));groups.impact.push(ix>0?g[ix]:g[g.length-1]);}
 return {title:plain(title),links,groups};
}
function itemOf(d,x,i,sections){
 let s=sections[i]||{links:[],groups:{general:[],technical:[],impact:[]},title:""};
 const g=s.groups.general||[],t=s.groups.technical||[],imp=s.groups.impact||[];
 const lead=x.easy_explainer||(x.details&&!d.archived?x.details:g[0])||(x.summary&&!/원문 보기/.test(x.summary)?x.summary:"")||"현재 자세한 설명이 없습니다. 원문을 확인해 주세요.";
 const technical=x.technical_context||t[0]||(g.length>2?g[1]:"");
 const developer=x.developer_view||x.why_it_matters||x.impact||imp[0]||"";
 const summary=x.short_summary||(x.summary&&!/원문 보기/.test(x.summary)&&x.summary.length>25?x.summary:sentence(lead,2)+(developer?" "+sentence(developer,1):""));
 const url=safeUrl(x.url)||s.links[0]?.url||"",links=[];
 if(url)links.push({url,label:x.source||s.links[0]?.label||"원문"});
 for(const link of s.links)if(links.length<3&&!links.some(k=>k.url===link.url))links.push(link);
 return {id:ident(d.date,i),date:d.date,category:x.category||"game-dev",title:plain(x.title||s.title||"제목 없음"),source:plain(x.source||s.links[0]?.label||"기술 뉴스"),published:plain(x.published||""),lead:clip(lead,780),technical:clip(technical,540),developer:clip(developer,570),summary:clip(summary,345),glossary:terms(String(x.title||"")+" "+lead+" "+technical),links};
}
const tag=x=>'<span class="tag tag-'+esc(x.category)+'">'+esc(emoji[x.category]||"•")+" "+esc(cats[x.category]||"기술 뉴스")+"</span>";
function explanation(x){
 if(tab==="short"){
  let parts=(x.summary.match(/[^.!?。]+(?:[.!?。]+|$)/g)||[x.summary]).filter(Boolean).slice(0,4);
  return '<div class="reader-pane"><div class="reader-kicker">QUICK RECAP</div><h4>핵심만 빠르게</h4><ul class="digest">'+parts.map(p=>"<li>"+esc(p.trim())+"</li>").join("")+'</ul><p class="hint">더 알아보려면 쉽게 설명 탭을 선택하세요.</p></div>';
 }
 return '<div class="reader-pane"><div class="reader-kicker">STEP 01 · 무슨 소식인가요?</div><h4>먼저, 쉽게 이해하기</h4><p>'+esc(x.lead)+'</p>'+
 (x.glossary.length?'<div class="glossary"><div class="glossary-title">💡 알아두면 좋은 용어</div><ul>'+x.glossary.map(t=>"<li>"+esc(t)+"</li>").join("")+"</ul></div>":"")+
 (x.technical&&x.technical!==x.lead?'<div class="reader-kicker second">STEP 02 · 어떻게 작동하나요?</div><h4>기술 배경</h4><p>'+esc(x.technical)+"</p>":"")+
 (x.developer&&x.developer!==x.lead&&x.developer!==x.technical?'<div class="reader-kicker second">STEP 03 · 개발 업무와 연결하면?</div><h4>게임·엔진 개발자 관점</h4><p>'+esc(x.developer)+"</p>":"")+"</div>";
}
function renderCard(x){
 const open=openId===x.id,saved=stored.saved.includes(x.id),read=stored.read.includes(x.id),domId=x.id.replace(":","-");
 let sources=x.links.length?'<div class="source-links"><span class="source-label">원문 / 참고 자료</span>'+x.links.map(s=>'<a target="_blank" rel="noopener noreferrer" href="'+esc(s.url)+'">↗ '+esc(s.label)+"</a>").join("")+"</div>":"";
 return '<article class="news-card '+(open?"expanded ":"")+(read?"is-read":"")+'" id="story-'+esc(domId)+'" data-category="'+esc(x.category)+'">'+
 '<div class="card-top"><div class="card-stamps">'+tag(x)+'<span class="meta">'+esc(x.date)+'</span>'+(read?'<span class="read-check">✓ 읽음</span>':"")+'</div>'+
 '<button type="button" class="card-title" data-open="'+esc(x.id)+'" aria-expanded="'+String(open)+'" aria-controls="content-'+esc(domId)+'">'+esc(x.title)+'<span class="chevron" aria-hidden="true">'+(open?"⌃":"⌄")+'</span></button>'+
 '<p class="preview">'+esc(x.summary)+'</p><div class="mini-meta"><span>출처 · '+esc(x.source)+'</span>'+(x.published?'<span>발행 · '+esc(x.published)+'</span>':"")+'</div></div>'+
 (open?'<div class="card-content" id="content-'+esc(domId)+'"><div class="tabs" role="tablist" aria-label="읽기 방식">'+
 '<button type="button" role="tab" data-tab="easy" aria-selected="'+(tab==="easy")+'" class="'+(tab==="easy"?"selected":"")+'">📖 쉽게 설명</button>'+
 '<button type="button" role="tab" data-tab="short" aria-selected="'+(tab==="short")+'" class="'+(tab==="short"?"selected":"")+'">⚡ 3줄 요약</button></div>'+
 '<div class="reader-content">'+explanation(x)+"</div>"+sources+"</div>":"")+
 '<div class="card-actions"><button type="button" data-action="saved" data-id="'+esc(x.id)+'" aria-pressed="'+saved+'" class="'+(saved?"selected-action":"")+'">'+(saved?"★ 북마크됨":"☆ 북마크")+'</button>'+
 '<button type="button" data-action="read" data-id="'+esc(x.id)+'" aria-pressed="'+read+'" class="'+(read?"selected-action":"")+'">'+(read?"✓ 읽음 완료":"◯ 읽음 표시")+'</button>'+
 (x.links[0]?'<a class="action-source" target="_blank" rel="noopener noreferrer" href="'+esc(x.links[0].url)+'">원문 ↗</a>':"")+"</div></article>";
}
function render(){
 let daily=view==="daily";$("#modeDaily").classList.toggle("active",daily);$("#modeAll").classList.toggle("active",!daily);
 $("#modeDaily").setAttribute("aria-pressed",daily);$("#modeAll").setAttribute("aria-pressed",!daily);$("#dateNav").hidden=!daily;
 $("#briefDate").value=selectedDate;let index=days.findIndex(d=>d.date===selectedDate);
 $("#prevDate").disabled=index<0||index===days.length-1;$("#nextDate").disabled=index<=0;
 document.querySelectorAll("button.chip").forEach(b=>{let y=b.dataset.category===category;b.classList.toggle("active",y);b.setAttribute("aria-pressed",y);});
 let q=$("#search").value.trim().toLocaleLowerCase(),onlySaved=$("#bookmarksOnly").checked,onlyUnread=$("#unreadOnly").checked;
 let filtered=articles.filter(x=>(!daily||x.date===selectedDate)&&(category==="all"||category===x.category)&&(!onlySaved||stored.saved.includes(x.id))&&(!onlyUnread||!stored.read.includes(x.id))&&(!q||[x.title,x.summary,x.lead,x.technical,x.developer,x.source,x.date].join(" ").toLocaleLowerCase().includes(q)));
 $("#totalCount").textContent=articles.length+"개 기사 · "+days.length+"일 기록";$("#resultsTitle").textContent=daily?selectedDate+" 브리핑":"전체 기사 모아보기";
 $("#resultCount").textContent=filtered.length+"개 기사";
 let visible=daily?filtered:filtered.slice(0,limit);
 $("#entries").innerHTML=visible.length?visible.map(renderCard).join(""):'<div class="empty"><strong>조건에 맞는 기사가 없어요.</strong><p>날짜·카테고리·검색 조건을 바꿔 보세요.</p></div>';
 $("#loadMoreWrap").hidden=filtered.length<=visible.length;$("#loadMore").textContent="기사 더 보기 · "+(filtered.length-visible.length)+"개 남음 ↓";
 if(failed)$("#latest").textContent="최근 게시: "+(days[0]?.date||"-")+" · 일부 날짜 "+failed+"건 불러오기 실패";
}
function moveDate(offset){let i=days.findIndex(d=>d.date===selectedDate);if(days[i+offset]){selectedDate=days[i+offset].date;openId=null;render();}}
async function load(){
 try{
  const r=await fetch("data/catalog.json",{cache:"no-store"});if(!r.ok)throw Error("날짜 목록 요청 실패");
  let v=await r.json(),catalog=(Array.isArray(v)?v:v.entries||[]).filter(x=>/^\d{4}-\d{2}-\d{2}$/.test(x.date)&&/^data\/[\d-]+\.json$/.test(x.file)).sort((a,b)=>b.date.localeCompare(a.date));
  let results=await Promise.allSettled(catalog.map(async x=>{let r=await fetch(x.file,{cache:"no-store"});if(!r.ok)throw Error(x.file);return r.json();}));
  failed=results.filter(x=>x.status==="rejected").length;
  days=results.filter(x=>x.status==="fulfilled").map(x=>x.value).filter(x=>x&&Array.isArray(x.items)&&x.date).sort((a,b)=>b.date.localeCompare(a.date));
  selectedDate=days[0]?.date||"";
  articles=days.flatMap(d=>{let sections=extract(d.original_markdown);return d.items.map((x,i)=>itemOf(d,x,i,sections));});
  $("#briefDate").innerHTML=days.map(d=>'<option value="'+esc(d.date)+'">'+esc(d.date)+"</option>").join("");
  $("#latest").textContent=days.length?"최근 게시: "+days[0].date+" · "+days.length+"일 아카이브":"아직 등록된 기사가 없습니다.";
  render();
 }catch(e){$("#latest").textContent="브리핑 데이터 로드 실패";$("#entries").innerHTML='<div class="empty">'+esc(e.message)+"</div>";}
}
document.addEventListener("click",e=>{
 const action=e.target.closest("[data-action]");
 if(action){let type=action.dataset.action,id=action.dataset.id;if(!["saved","read"].includes(type))return;stored[type]=stored[type].includes(id)?stored[type].filter(x=>x!==id):[...stored[type],id];persist();render();return;}
 const opener=e.target.closest("[data-open]");
 if(opener){openId=openId===opener.dataset.open?null:opener.dataset.open;tab="easy";render();return;}
 const tabs=e.target.closest("[data-tab]");
 if(tabs){tab=tabs.dataset.tab;render();return;}
 const chip=e.target.closest("button.chip[data-category]");if(chip){category=chip.dataset.category;limit=25;render();}
});
$("#modeDaily").addEventListener("click",()=>{view="daily";openId=null;render();});
$("#modeAll").addEventListener("click",()=>{view="all";openId=null;limit=25;render();});
$("#briefDate").addEventListener("change",e=>{selectedDate=e.target.value;openId=null;render();});
$("#prevDate").addEventListener("click",()=>moveDate(1));
$("#nextDate").addEventListener("click",()=>moveDate(-1));
$("#latestDate").addEventListener("click",()=>{selectedDate=days[0]?.date||"";openId=null;render();});
$("#search").addEventListener("input",()=>{limit=25;render();});
$("#bookmarksOnly").addEventListener("change",()=>{limit=25;render();});
$("#unreadOnly").addEventListener("change",()=>{limit=25;render();});
$("#loadMore").addEventListener("click",()=>{limit+=25;render();});
$("#resetFilters").addEventListener("click",()=>{$("#search").value="";$("#bookmarksOnly").checked=false;$("#unreadOnly").checked=false;category="all";limit=25;render();});
load();
})();