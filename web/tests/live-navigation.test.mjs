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
