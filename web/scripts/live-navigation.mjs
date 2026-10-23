/** Retry only explicit temporary gateway/service HTTP responses, never assertion or navigation errors. */
export async function gotoWithTransientRetry(page,url,{sleep=ms=>new Promise(resolve=>setTimeout(resolve,ms))}={}) {
  for(let attempt=0;attempt<3;attempt++) {
    const response=await page.goto(url);
    if(![502,503,504].includes(response?.status())||attempt===2)return response;
    console.log(`Temporary HTTP ${response.status()} for ${url}; retry ${attempt+1}/2`);
    await sleep(1000*(attempt+1));
  }
}
