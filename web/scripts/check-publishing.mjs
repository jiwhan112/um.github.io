/** Verify the publishing contract and authored thinking metadata before every release. */
import {readFileSync,existsSync} from 'node:fs';
import {resolve} from 'node:path';
const root=resolve('..'),read=p=>readFileSync(resolve(root,p),'utf8');
const policy=JSON.parse(read('automation/publishing.json'));
if(policy.theme!=='dark'||policy.siteBase!=='/um.github.io/'||policy.branch!=='master')throw Error('Invalid publishing target or theme');
const expectedStages=['thinking','briefing','language'];
const pipeline=policy.pipeline;
if(policy.jobs.length!==3||JSON.stringify(policy.jobs.map(j=>j.id))!==JSON.stringify(expectedStages))throw Error('Unexpected publishing stages');
if(!pipeline?.automationId||pipeline.is_enabled!==true||pipeline.localTime!=='05:00'||pipeline.timezone!=='Asia/Seoul'||pipeline.cadence!=='daily'||pipeline.independentStageSchedulesEnabled!==false||JSON.stringify(pipeline.stages)!==JSON.stringify(expectedStages))throw Error('Unreviewed sequential pipeline drift');
for(const [index,j] of policy.jobs.entries()){
 if(j.execution!=='sequential_pipeline_stage'||j.runAfter!==(index===0?'pipeline_start':expectedStages[index-1])||j.automationId||j.localTime)throw Error('Independent stage schedule or order drift: '+j.id);
 for(const pattern of j.allowedWritePatterns){const re=new RegExp(pattern);for(const p of ['index.html','thinking-lab/index.html','web/src/styles.css','language-study/sw.js','data/atlas/catalog.json'])if(re.test(p))throw Error('Scheduled writer can modify generated/UI file: '+p);}
}
const feed=policy.contentFeeds?.find(f=>f.id==='game-design');
if(!feed||feed.targetEpisodes!==12||feed.allowedWritePatterns?.length!==3)throw Error('Game design feed policy missing');
for(const pattern of feed.allowedWritePatterns){const re=new RegExp(pattern);for(const p of ['index.html','game-design/index.html','web/src/App.tsx','data/atlas/catalog.json'])if(re.test(p))throw Error('Game design scheduled writer may modify UI/generated: '+p);}
const gd=JSON.parse(read('game-design/data/catalog.json'));
function findImages(blocks){
 return blocks.flatMap(b=>b.type==='image'?[b.src]:b.blocks?findImages(b.blocks):b.type==='list'?b.items.flatMap(findImages):[]);
}
const imagePaths=[];
for(const entry of gd.episodes){
 const doc=JSON.parse(read('game-design/'+entry.path+'content.json'));
 const sources=doc.sections.flatMap(section=>findImages(section.blocks));
 for(const src of sources){
  if(!/^\/game-design\/ep-[a-z0-9-]+\/assets\/[a-z0-9-]+\.(svg|png|webp|jpg|jpeg)$/.test(src)||!src.startsWith('/game-design/'+entry.path+'assets/'))throw Error('Unsafe or cross-episode game design illustration: '+src);
  if(!existsSync(resolve(root,src.slice(1))))throw Error('Missing game design illustration: '+src);
  imagePaths.push(src);
  if(src.endsWith('.svg')){
    const svg=read(src.slice(1));if(/<script\b|<foreignObject\b|\son[a-z]+\s*=|(?:href|url)\s*=\s*["']?https?:|<image\b|<iframe\b|<!DOCTYPE|<!ENTITY/i.test(svg))throw Error('Unsafe SVG illustration: '+src);
  }
 }
}

const videoIds=new Set();const paths=new Set();for(const e of gd.episodes){if(!e.video_id||!e.video_url||e.video_url!=='https://www.youtube.com/watch?v='+e.video_id||videoIds.has(e.video_id)||paths.has(e.path)||!e.category||!e.tags?.length||e.tags.length>8||!existsSync(resolve(root,'game-design/'+e.path+'content.json')))throw Error('Invalid game design episode: '+e.path);videoIds.add(e.video_id);paths.add(e.path);}
if(gd.episodes.length>12)throw Error('Game design exceeds approved 12 episodes');
const catalog=JSON.parse(read('thinking-lab/data/catalog.json'));
for(const e of catalog.episodes){
 if(!e.category?.trim()||!Array.isArray(e.tags)||e.tags.length<1||e.tags.length>8)throw Error('Missing classification: '+e.path);
 if(new Set(e.tags).size!==e.tags.length||e.tags.some(t=>typeof t!=='string'||!t.trim()||t!==t.trim()||t.length>40||/\d{4}-\d{2}-\d{2}/.test(t)))throw Error('Invalid tags: '+e.path);
 const base='thinking-lab/'+e.path;if(!existsSync(resolve(root,base+'content.json'))&&!existsSync(resolve(root,base+'_content.html')))throw Error('Missing authoring input: '+e.path);
}
const publish=read('.github/workflows/react-pages.yml');
if(!publish.includes('game-design/**')||!publish.includes('knowledge/**')||!publish.includes('automation/**')||!publish.includes('Reconcile language lessons')||publish.includes("- 'language-study/data/**'"))throw Error('Publication triggers no longer match source ownership');
const reconcile=read('.github/workflows/language-study-reconcile.yml');if(!reconcile.includes("cron: '10 0 * * *'"))throw Error('Reconcile fallback must match 09:10 KST');
console.log('Publishing contract passed: one 05:00 sequential pipeline, 3 scoped content stages, dark theme, classified thinking catalog.');
