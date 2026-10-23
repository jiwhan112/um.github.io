import test from 'node:test';
import assert from 'node:assert/strict';
import {gotoWithTransientRetry} from '../scripts/live-navigation.mjs';
function fixture(statuses){
  const calls=[],delays=[];
  const page={async goto(url){calls.push(url);const next=statuses.shift();if(next instanceof Error)throw next;return next===null?null:{status:()=>next};}};
  return {page,calls,delays,options:{sleep:async ms=>{delays.push(ms);}}};
}
test('503 then 200 retries navigation and returns the actual successful response',async()=>{
  const f=fixture([503,200]);const response=await gotoWithTransientRetry(f.page,'https://example.test/',f.options);
  assert.equal(response.status(),200);assert.equal(f.calls.length,2);assert.deepEqual(f.delays,[1000]);
});
test('temporary responses exhaust after three attempts and remain failures',async()=>{
  const f=fixture([502,504,503,200]);const response=await gotoWithTransientRetry(f.page,'https://example.test/',f.options);
  assert.equal(response.status(),503);assert.equal(f.calls.length,3);assert.deepEqual(f.delays,[1000,2000]);
});
for(const status of [200,400,401,403,404,429,500,null])test(`${status} is not retried`,async()=>{
  const f=fixture([status,200]);const response=await gotoWithTransientRetry(f.page,'https://example.test/',f.options);
  assert.equal(response?.status()??null,status);assert.equal(f.calls.length,1);assert.deepEqual(f.delays,[]);
});
test('navigation exceptions propagate without retry',async()=>{
  const error=new Error('navigation failed');const f=fixture([error,200]);
  await assert.rejects(gotoWithTransientRetry(f.page,'https://example.test/',f.options),error);
  assert.equal(f.calls.length,1);assert.deepEqual(f.delays,[]);
});

import {EventEmitter} from 'node:events';
import {verifyRouteWithTransientRetry} from '../scripts/live-navigation.mjs';
function routeFixture(){
 const pages=[],diagnostics=[],delays=[];
 const context={async newPage(){const p=new EventEmitter();p.close=async()=>{p.closed=true;};pages.push(p);return p;}};
 const respond=(p,status=503,type='stylesheet',url='https://example.test/asset')=>p.emit('response',{url:()=>url,status:()=>status,request:()=>({resourceType:()=>type})});
 return {context,pages,diagnostics,delays,respond,options:{sleep:async ms=>delays.push(ms),onFailure:async d=>diagnostics.push(JSON.parse(JSON.stringify(d.history)))}};
}
for(const type of ['stylesheet','script','fetch'])test(`strict failed check retries observed transient ${type} on a fresh page`,async()=>{
 const f=routeFixture();let count=0;
 const result=await verifyRouteWithTransientRetry(f.context,'https://example.test/',async(p,errors)=>{
  assert.deepEqual(errors,[]);if(count++===0){await Promise.resolve();f.respond(p,503,type);throw Error('strict content check failed');}return 'verified';
 },f.options);
 assert.equal(result,'verified');assert.equal(f.pages.length,2);assert.deepEqual(f.delays,[1000]);assert.ok(f.pages.every(p=>p.closed&&p.listenerCount('response')===0));
 assert.equal(f.diagnostics[0][0].responses[0].resourceType,type);
});
test('an assertion without transient evidence is not retried',async()=>{
 const f=routeFixture(),error=Error('CSS assertion');await assert.rejects(verifyRouteWithTransientRetry(f.context,'https://example.test/',async()=>{throw error;},f.options),e=>e===error);assert.equal(f.pages.length,1);
});
test('request failure alone is diagnostic, not retry evidence',async()=>{
 const f=routeFixture();await assert.rejects(verifyRouteWithTransientRetry(f.context,'https://example.test/',async p=>{p.emit('requestfailed',{url:()=> 'https://example.test/a.js',resourceType:()=> 'script',failure:()=>({errorText:'net::ERR_FAILED'})});throw Error('missing React');},f.options));assert.equal(f.pages.length,1);assert.equal(f.diagnostics[0][0].requestFailures[0].error,'net::ERR_FAILED');
});
for(const blocker of ['404','pageerror','foreign503'])test(`does not hide ${blocker} with retries`,async()=>{
 const f=routeFixture();await assert.rejects(verifyRouteWithTransientRetry(f.context,'https://example.test/',async p=>{
  if(blocker==='foreign503')f.respond(p,503,'script','https://other.test/a');
  else {f.respond(p);if(blocker==='404')f.respond(p,404,'fetch');else p.emit('pageerror',Error('application bug'));}
  throw Error('strict failure');
 },f.options));assert.equal(f.pages.length,1);
});
test('exhausted transient subresource attempts retain original failure and evidence',async()=>{
 const f=routeFixture(),error=Error('still missing');await assert.rejects(verifyRouteWithTransientRetry(f.context,'https://example.test/',async p=>{f.respond(p,504,'fetch');throw error;},f.options),e=>e===error);
 assert.equal(f.pages.length,3);assert.deepEqual(f.delays,[1000,2000]);assert.equal(f.diagnostics.at(-1).length,3);
});
test('a permanent response arriving during diagnostic capture also blocks retry',async()=>{
 const f=routeFixture();f.options.onFailure=async({page})=>{await Promise.resolve();f.respond(page,404,'fetch');};
 await assert.rejects(verifyRouteWithTransientRetry(f.context,'https://example.test/',async p=>{f.respond(p);throw Error('strict failure');},f.options));assert.equal(f.pages.length,1);
});
