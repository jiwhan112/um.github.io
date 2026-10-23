/** Retry only explicit temporary gateway/service HTTP responses, never assertion or navigation errors. */
export async function gotoWithTransientRetry(page,url,{sleep=ms=>new Promise(resolve=>setTimeout(resolve,ms))}={}) {
  for(let attempt=0;attempt<3;attempt++) {
    const response=await page.goto(url);
    if(![502,503,504].includes(response?.status())||attempt===2)return response;
    console.log(`Temporary HTTP ${response.status()} for ${url}; retry ${attempt+1}/2`);
    await sleep(1000*(attempt+1));
  }
}

/** Run the complete strict route check again only after observed same-origin transient HTTP.
 * Fresh pages isolate late events and page errors from earlier attempts. Response metadata is
 * recorded synchronously (no response-body await), including fetch/XHR after navigation.
 */
export async function verifyRouteWithTransientRetry(context,url,verify,{
 sleep=ms=>new Promise(resolve=>setTimeout(resolve,ms)),
 onFailure=async()=>{}
}={}) {
 const history=[];
 for(let attempt=1;attempt<=3;attempt++) {
  const page=await context.newPage();
  const evidence={url,attempt,responses:[],requestFailures:[],pageErrors:[]};
  const response=r=>evidence.responses.push({url:r.url(),status:r.status(),resourceType:r.request().resourceType()});
  const failed=r=>evidence.requestFailures.push({url:r.url(),resourceType:r.resourceType(),error:r.failure()?.errorText??null});
  const pageerror=e=>evidence.pageErrors.push(e.message);
  page.on('response',response);page.on('requestfailed',failed);page.on('pageerror',pageerror);
  try {
   return await verify(page,evidence.pageErrors);
  } catch(error) {
   evidence.error=String(error);history.push(evidence);
   const sameOrigin=evidence.responses.filter(r=>new URL(r.url).origin===new URL(url).origin);
   const transient=sameOrigin.some(r=>[502,503,504].includes(r.status));
   const permanent=sameOrigin.some(r=>r.status>=400&&![502,503,504].includes(r.status));
   const retry=transient&&!permanent&&evidence.pageErrors.length===0&&attempt<3;
   try {await onFailure({page,history,retry,error});}catch(diagnosticError){console.warn('Could not record live diagnostics:',diagnosticError);}
   // Diagnostics may await a screenshot; fail closed if a hard error arrived meanwhile.
   const lateHardError=evidence.pageErrors.length>0||evidence.responses.some(r=>new URL(r.url).origin===new URL(url).origin&&r.status>=400&&![502,503,504].includes(r.status));
   if(!retry||lateHardError)throw error;
   console.log(`Observed same-origin transient HTTP during failed check ${url}; retry ${attempt}/2`);
  } finally {
   page.off('response',response);page.off('requestfailed',failed);page.off('pageerror',pageerror);
   await page.close().catch(closeError=>console.warn('Could not close live-check page:',closeError));
  }
  await sleep(1000*attempt);
 }
}
