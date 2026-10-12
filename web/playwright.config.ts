import {defineConfig,devices} from '@playwright/test';
export default defineConfig({
 testDir:'./tests',testMatch:'**/*.spec.ts',timeout:30000,expect:{timeout:8000,toHaveScreenshot:{maxDiffPixelRatio:.012,animations:'disabled'}},
 fullyParallel:true,forbidOnly:!!process.env.CI,retries:process.env.CI?1:0,workers:process.env.CI?3:2,
 reporter:[['list'],['html',{open:'never'}],['json',{outputFile:'test-results/results.json'}]],
 use:{baseURL:'http://127.0.0.1:4173/um.github.io/',serviceWorkers:'block',locale:'ko-KR',timezoneId:'Asia/Seoul',colorScheme:'dark',trace:'retain-on-failure',screenshot:'only-on-failure',launchOptions:{executablePath:process.env.PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH}},
 projects:[{name:'desktop',use:{viewport:{width:1440,height:1000}}},{name:'mobile',use:{...devices['Pixel 7'],defaultBrowserType:'chromium',viewport:{width:390,height:844}}}],
 webServer:{command:'node tests/server.mjs',url:'http://127.0.0.1:4173/um.github.io/',reuseExistingServer:!process.env.CI,timeout:10000}
});
