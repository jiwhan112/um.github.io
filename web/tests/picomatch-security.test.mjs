import test from 'node:test';
import assert from 'node:assert/strict';
import {createRequire} from 'node:module';
import {spawnSync} from 'node:child_process';
import {readFileSync,readdirSync} from 'node:fs';
import {createHash} from 'node:crypto';
import {fileURLToPath} from 'node:url';
import {resolve,join} from 'node:path';
import {globSync} from 'tinyglobby';
const require=createRequire(import.meta.url),pm=require('picomatch');
const root=fileURLToPath(new URL('../',import.meta.url));
const vendor=join(root,'vendor/picomatch-safe');

test('reviewed local matcher retains upstream bytes except the documented guard',()=>{
 const hashes=JSON.parse(readFileSync(join(vendor,'upstream-sha256.json'),'utf8'));
 for(const [file,sha] of Object.entries(hashes)){
  let data=readFileSync(join(vendor,file));
  if(file==='lib/parse.js'){
   let text=data.toString();
   const patch=`    // ATLAS security policy: repeated extglobs are unsupported and literalized.\n    // This removes the unbounded outer quantifier, including single/multi-branch\n    // variants of CVE-2026-112366. maxExtglobRecursion cannot opt out of this rule.\n    const analysis = (token.type === 'plus' || token.type === 'star')\n      ? { risky: true }\n      : analyzeRepeatedExtglob(body, opts);`;
   assert.equal(text.split(patch).length,2,'exactly the reviewed security guard');
   data=Buffer.from(text.replace(patch,'    const analysis = analyzeRepeatedExtglob(body, opts);'));
  }
  assert.equal(createHash('sha256').update(data).digest('hex'),sha,file);
 }
 assert.equal(require('picomatch/package.json').name,'@atlas-note/picomatch-safe');
 const lock=JSON.parse(readFileSync(join(root,'package-lock.json'),'utf8'));
 for(const [path,entry] of Object.entries(lock.packages)){
  assert.ok(!/registry\.npmjs\.org\/picomatch\//.test(entry.resolved||''),path+' must not reinstall vulnerable upstream');
 }
});

test('repeated extglobs fail closed across matcher, parse and regex APIs within a bounded process',()=>{
 const patterns=['+(*)','*(*)b','+(*a)','*(a*)','+(a*b)','+(*a|b)','*(a|aa)','+(*(a)|*(b))','+(+(a))','src/+(*a).js'];
 const program=`const assert=require('node:assert/strict'),pm=require(${JSON.stringify(require.resolve('picomatch'))});
 for(const pattern of ${JSON.stringify(patterns)})for(const options of [{},{maxExtglobRecursion:false},{maxExtglobRecursion:999},{fastpaths:false},{capture:true},{windows:true}]){
   const inputs=['a'.repeat(64)+'!','a'.repeat(64)+'/', 'src/'+ 'a'.repeat(64)+'.js'];
   const matchers=[pm(pattern,options),s=>pm.isMatch(s,pattern,options),s=>pm.makeRe(pattern,options).test(s),s=>pm.compileRe(pm.parse(pattern,options),options).test(s)];
   for(const input of inputs)for(const matcher of matchers)assert.equal(matcher(input),false,pattern);
   assert.equal(pm(pattern,options)(pattern),true,'repeated expression is literal: '+pattern);
 }`;
 const result=spawnSync(process.execPath,['-e',program],{encoding:'utf8',timeout:3000});
 assert.ifError(result.error);assert.equal(result.status,0,result.stderr);
});

test('ordinary Vite glob, ignore, scan and parse contracts remain intact',()=>{
 const match=pm('**/*.{ts,tsx}',{dot:true,ignore:'**/node_modules/**'});
 assert.equal(match('src/App.tsx'),true);assert.equal(match('.config/tool.ts'),true);
 assert.equal(match('src/app.css'),false);assert.equal(match('node_modules/a/index.ts'),false);
 assert.equal(pm('**/@(foo|bar).js')('src/foo.js'),true);
 assert.equal(pm('**/!(test).js')('src/main.js'),true);
 assert.equal(pm('**/!(test).js')('src/test.js'),false);
 assert.equal(pm.scan('src/**/*.{ts,tsx}').base,'src');
 assert.equal(pm.compileRe(pm.parse('src/**/*.tsx')).test('src/App.tsx'),true);
 assert.equal(pm(['src/**/*.tsx','tests/**/*.mjs'])('tests/picomatch-security.test.mjs'),true);
});

test('tinyglobby still discovers exactly all project TypeScript inputs',()=>{
 const walk=dir=>readdirSync(join(root,dir),{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(dir+'/'+e.name):[/\.tsx?$/.test(e.name)?dir+'/'+e.name:null].filter(Boolean));
 assert.deepEqual(globSync('src/**/*.{ts,tsx}',{cwd:root}).sort(),walk('src').sort());
});
