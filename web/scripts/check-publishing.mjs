/** Verify the publishing contract and authored thinking metadata before every release. */
import {readFileSync,existsSync} from 'node:fs';
import {resolve} from 'node:path';
const root=resolve('..'),read=p=>readFileSync(resolve(root,p),'utf8');
const policy=JSON.parse(read('automation/publishing.json'));
if(policy.theme!=='dark'||policy.siteBase!=='/um.github.io/'||policy.branch!=='master')throw Error('Invalid publishing target or theme');
if(policy.jobs.length!==3||new Set(policy.jobs.map(j=>j.automationId)).size!==3)throw Error('Unexpected scheduled jobs');
const expected={thinking:'06:00',briefing:'08:00',language:'09:00'};
for(const j of policy.jobs){
 if(j.localTime!==expected[j.id])throw Error('Unreviewed schedule drift: '+j.id);
 for(const pattern of j.allowedWritePatterns){const re=new RegExp(pattern);for(const p of ['index.html','thinking-lab/index.html','web/src/styles.css','language-study/sw.js','data/atlas/catalog.json'])if(re.test(p))throw Error('Scheduled writer can modify generated/UI file: '+p);}
}
const feed=policy.contentFeeds?.find(f=>f.id==='game-design');
if(!feed||feed.targetEpisodes!==12||feed.allowedWritePatterns?.length!==2)throw Error('Game design feed policy missing');
for(const pattern of feed.allowedWritePatterns){const re=new RegExp(pattern);for(const p of ['index.html','game-design/index.html','web/src/App.tsx','data/atlas/catalog.json'])if(re.test(p))throw Error('Game design scheduled writer may modify UI/generated: '+p);}
const gd=JSON.parse(read('game-design/data/catalog.json'));
const videoIds=new Set();const paths=new Set();for(const e of gd.episodes){if(!e.video_id||!e.video_url||e.video_url!=='https://www.youtube.com/watch?v='+e.video_id||videoIds.has(e.video_id)||paths.has(e.path)||!e.category||!e.tags?.length||e.tags.length>8||!existsSync(resolve(root,'game-design/'+e.path+'content.json')))throw Error('Invalid game design episode: '+e.path);videoIds.add(e.video_id);paths.add(e.path);}
if(gd.episodes.length>12)throw Error('Game design exceeds approved 12 episodes');
const catalog=JSON.parse(read('thinking-lab/data/catalog.json'));
for(const e of catalog.episodes){
 if(!e.category?.trim()||!Array.isArray(e.tags)||e.tags.length<1||e.tags.length>8)throw Error('Missing classification: '+e.path);
 if(new Set(e.tags).size!==e.tags.length||e.tags.some(t=>typeof t!=='string'||!t.trim()||t!==t.trim()||t.length>40||/\d{4}-\d{2}-\d{2}/.test(t)))throw Error('Invalid tags: '+e.path);
 const base='thinking-lab/'+e.path;if(!existsSync(resolve(root,base+'content.json'))&&!existsSync(resolve(root,base+'_content.html')))throw Error('Missing authoring input: '+e.path);
}
const publish=read('.github/workflows/react-pages.yml');
if(!publish.includes('game-design/**')||!publish.includes('automation/**')||!publish.includes('Reconcile language lessons')||publish.includes("- 'language-study/data/**'"))throw Error('Publication triggers no longer match source ownership');
const reconcile=read('.github/workflows/language-study-reconcile.yml');if(!reconcile.includes("cron: '10 0 * * *'"))throw Error('Reconcile fallback must match 09:10 KST');
console.log('Publishing contract passed: 3 scheduled writers, dark theme, classified thinking catalog.');
