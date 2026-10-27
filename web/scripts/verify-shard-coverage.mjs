import assert from 'node:assert/strict';
import {readFileSync,writeFileSync,readdirSync} from 'node:fs';
import {join,resolve} from 'node:path';
import {pathToFileURL} from 'node:url';

export function cases(report,{executed=false}={}) {
 assert.ok(Array.isArray(report.suites),'Missing suites');
 assert.equal((report.errors||[]).length,0,'Reporter contains global errors');
 const out=[];
 function walk(suites){for(const suite of suites){for(const spec of suite.specs||[]){for(const test of spec.tests||[]){
  assert.ok(spec.id&&test.projectName,'Missing stable test/project identity');
  if(executed){
   assert.ok(test.results?.length,'Test has no execution record: '+spec.title);
   assert.ok(['expected','flaky','skipped'].includes(test.status),'Unexpected test outcome: '+spec.title);
   if(test.status==='skipped')assert.ok(spec.file==='mermaid.spec.ts'&&spec.title==='every existing knowledge Mermaid source renders as a loaded diagram'&&test.projectName==='mobile','Unreviewed skipped test: '+test.projectName+'::'+spec.title);
   assert.ok(test.results.every(r=>r.status!=='interrupted'),'Interrupted test: '+spec.title);
  }
  out.push(test.projectName+'::'+spec.id);
 }}walk(suite.suites||[]);}}
 walk(report.suites);assert.ok(out.length,'Empty test collection');assert.equal(new Set(out).size,out.length,'Duplicate test identity');return out.sort();
}
export function makeManifest(full,shards,identity){
 assert.equal(shards.length,4,'Exactly four shards required');
 const expected=cases(full),partition=shards.map(x=>cases(x));
 assert.deepEqual(partition.flat().sort(),expected,'Shard collection is not an exact partition');
 assert.match(identity.source,/^[a-f0-9]{40}$/);assert.match(identity.lockHash,/^[a-f0-9]{64}$/);assert.match(identity.distHash,/^[a-f0-9]{64}$/);
 assert.match(String(identity.runId),/^\d+$/);assert.match(String(identity.attempt),/^\d+$/);
 return {schemaVersion:1,...identity,total:expected.length,shardCount:4,expected,partition};
}
export function verify(manifest,artifacts,identity){
 assert.equal(manifest.schemaVersion,1);assert.equal(manifest.shardCount,4);assert.equal(artifacts.length,4,'Missing or extra shard artifact');
 for(const key of ['source','runId','attempt','lockHash','distHash'])assert.equal(manifest[key],identity[key],'Prepared identity mismatch: '+key);
 const seen=new Set(),all=[];
 for(const {meta,report}of artifacts){
  for(const key of ['source','runId','attempt','lockHash','distHash','inputHash'])assert.equal(meta[key],identity[key],'Shard identity mismatch: '+key);
  assert.ok(Number.isInteger(meta.shard)&&meta.shard>=1&&meta.shard<=4,'Invalid shard number');assert.ok(!seen.has(meta.shard),'Duplicate shard');seen.add(meta.shard);
  const actual=cases(report,{executed:true});assert.deepEqual(actual,manifest.partition[meta.shard-1],'Shard test coverage mismatch');all.push(...actual);
 }
 assert.deepEqual(all.sort(),manifest.expected,'Merged test coverage mismatch');assert.equal(all.length,manifest.total);return {verifiedTests:all.length,shards:seen.size,source:identity.source};
}
function json(p){return JSON.parse(readFileSync(p,'utf8'));}
function identity(){return {source:process.env.SOURCE_SHA,runId:process.env.RUN_ID,attempt:process.env.RUN_ATTEMPT,lockHash:process.env.LOCK_HASH,distHash:process.env.DIST_HASH,inputHash:process.env.INPUT_HASH};}
if(process.argv[1]&&import.meta.url===pathToFileURL(resolve(process.argv[1])).href){
 const [mode,...args]=process.argv.slice(2);
 if(mode==='manifest'){
  const [full,out,...shards]=args;writeFileSync(out,JSON.stringify(makeManifest(json(full),shards.map(json),identity()),null,2)+'\n');
 }else if(mode==='stamp'){
  const [manifestPath,output,shard]=args,m=json(manifestPath),id=identity();for(const k of ['source','runId','attempt','lockHash','distHash'])assert.equal(id[k],m[k]);writeFileSync(output,JSON.stringify({...id,shard:Number(shard)},null,2)+'\n');
 }else if(mode==='verify'){
  const [manifestPath,dir]=args;const artifacts=readdirSync(dir,{withFileTypes:true}).filter(x=>x.isDirectory()).map(x=>({meta:json(join(dir,x.name,'shard-meta.json')),report:json(join(dir,x.name,'test-results/results.json'))}));console.log(JSON.stringify(verify(json(manifestPath),artifacts,identity()),null,2));
 }else throw Error('Expected manifest, stamp or verify');
}
