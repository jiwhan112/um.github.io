import {test} from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp, mkdir, writeFile, rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join,resolve} from 'node:path';
import {execFileSync,spawnSync} from 'node:child_process';
const script=resolve('scripts/publish.mjs');
test('publication reentry proves exact generated child and rejects altered lineage/artifact',async()=>{
 const dir=await mkdtemp(join(tmpdir(),'atlas-publish-test-'));
 const remote=join(dir,'remote.git'),repo=join(dir,'work'),web=join(repo,'web');
 const git=(...args)=>execFileSync('git',args,{cwd:repo,encoding:'utf8',stdio:['pipe','pipe','pipe']}).trim();
 try{
  await mkdir(repo);git('init','-b','master');git('config','user.name','Test User');git('config','user.email','test@example.invalid');
  await mkdir(web);await mkdir(join(web,'dist/data/atlas'),{recursive:true});
  await writeFile(join(repo,'source.txt'),'authored');git('add','source.txt');git('commit','-m','source');
  const source=git('rev-parse','HEAD');
  execFileSync('git',['init','--bare',remote],{stdio:'ignore'});git('remote','add','origin',remote);git('push','origin','master');
  await writeFile(join(web,'dist/build-info.json'),JSON.stringify({framework:'React',revision:'fixture'}));
  await writeFile(join(web,'dist/index.html'),'<h1>verified</h1>');
  await writeFile(join(web,'dist/data/atlas/routes.json'),'["/"]');
  await writeFile(join(web,'dist/data/atlas/catalog.json'),'{"documents":[]}');
  const run=()=>spawnSync(process.execPath,[script],{cwd:web,encoding:'utf8'});
  let result=run();assert.equal(result.status,0,result.stderr);const runtime=git('rev-parse','HEAD');assert.notEqual(runtime,source);
  // Even bot-shaped single children fail closed for source changes or tampered runtime bytes.
  for(const [path,bytes] of [['source.txt','unauthorized source'],['index.html','tampered runtime']]){
   git('checkout','--detach',runtime);await writeFile(join(repo,path),bytes);git('add',path);git('commit','--amend','--no-edit');
   const tampered=git('rev-parse','HEAD');git('push','origin',`HEAD:refs/heads/test-${path.replace('.','-')}`);
   execFileSync('git',['--git-dir',remote,'update-ref','refs/heads/master',tampered]);
   git('checkout','--detach',source);result=run();assert.notEqual(result.status,0,path);
   execFileSync('git',['--git-dir',remote,'update-ref','refs/heads/master',runtime]);
  }
  git('checkout','--detach',runtime);
  // Same parent and identical runtime bytes are insufficient for a user-authored commit.
  git('commit','--amend','--no-edit','--author','A User <user@example.invalid>');
  const userRuntime=git('rev-parse','HEAD');git('push','origin','HEAD:refs/heads/test-user-runtime');
  execFileSync('git',['--git-dir',remote,'update-ref','refs/heads/master',userRuntime]);
  git('checkout','--detach',source);result=run();assert.notEqual(result.status,0);
  execFileSync('git',['--git-dir',remote,'update-ref','refs/heads/master',runtime]);
  git('checkout','--detach',source);result=run();assert.equal(result.status,0,result.stderr);assert.match(result.stdout,/Reusing exact/);assert.equal(git('rev-parse','HEAD'),source);
  await writeFile(join(web,'dist/index.html'),'changed artifact');result=run();assert.notEqual(result.status,0);await writeFile(join(web,'dist/index.html'),'<h1>verified</h1>');
  git('checkout','--detach',runtime);await writeFile(join(repo,'source.txt'),'new user source');git('add','source.txt');git('commit','-m','user update');git('push','origin','HEAD:master');
  git('checkout','--detach',source);result=run();assert.notEqual(result.status,0);assert.match(result.stderr,/Refusing stale/);
 }finally{await rm(dir,{recursive:true,force:true});}
});
