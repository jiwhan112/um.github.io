import {test} from 'node:test';
import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile,rm,symlink} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {createHash} from 'node:crypto';
import {validatedExampleBundles} from '../scripts/example-bundles';
const bytes=Buffer.from('PK\u0003\u0004test-fixture'),sha256=createHash('sha256').update(bytes).digest('hex');
const bundle={id:'lesson-01',path:'knowledge/examples/lesson-01.zip',sha256,files:['main.cpp','README.md'],pages:['cpp-basics-01']};
async function fixture(run:(root:string,save:(data:unknown)=>Promise<void>)=>Promise<void>){
 const root=await mkdtemp(join(tmpdir(),'atlas-bundles-'));await mkdir(join(root,'knowledge/examples'),{recursive:true});
 const save=(data:unknown)=>writeFile(join(root,'knowledge/examples/catalog.json'),JSON.stringify(data));
 try{await writeFile(join(root,bundle.path),bytes);await run(root,save);}finally{await rm(root,{recursive:true,force:true});}
}
test('no manifest means no extra archives; declared bytes hash checked and unlisted ZIP excluded',()=>fixture(async(root,save)=>{
 assert.equal((await validatedExampleBundles(root)).size,0);await save({version:1,bundles:[bundle]});await writeFile(join(root,'knowledge/examples/unlisted.zip'),'unlisted');
 const result=await validatedExampleBundles(root);assert.deepEqual([...result.keys()],[bundle.path]);assert.deepEqual(result.get(bundle.path),bytes);
}));
for(const [label,override] of Object.entries({traversal:{path:'knowledge/examples/../secret.zip'},absolute:{path:'/tmp/secret.zip'},hash:{sha256:'0'.repeat(64)},member:{files:['../secret']},memberAbsolute:{files:['/secret']},duplicateMember:{files:['main.cpp','main.cpp']}}))test('reject '+label,()=>fixture(async(root,save)=>{await save({version:1,bundles:[{...bundle,...override}]});await assert.rejects(validatedExampleBundles(root));}));
test('reject duplicate bundle, missing file, and symlink escape',()=>fixture(async(root,save)=>{
 await save({version:1,bundles:[bundle,bundle]});await assert.rejects(validatedExampleBundles(root));
 await rm(join(root,bundle.path));await save({version:1,bundles:[bundle]});await assert.rejects(validatedExampleBundles(root));
 const target=join(root,'outside.zip');await writeFile(target,bytes);await symlink(target,join(root,bundle.path));await assert.rejects(validatedExampleBundles(root));
}));
test('optional sourceFiles are metadata, not execution instructions',()=>fixture(async(root,save)=>{
 await save({version:1,bundles:[{...bundle,sourceFiles:[{path:'main.cpp',source:'knowledge/cpp-basics-01/main.cpp',sha256}]}]});assert.equal((await validatedExampleBundles(root)).size,1);
}));

// The approved builder derives deterministic ZIP bytes from explicit source mappings.
// This check never executes teaching examples or extracts downloaded archives.
import {existsSync} from 'node:fs';
import {execFileSync} from 'node:child_process';
test('published bundle manifest agrees with deterministic source-only builder', {skip:!existsSync('../knowledge/examples/catalog.json')},()=>{
 execFileSync('python3',['../knowledge/examples/build_bundles.py','--check'],{stdio:'pipe'});
 execFileSync('python3',['../knowledge/examples/test_bundles.py'],{stdio:'pipe'});
});
