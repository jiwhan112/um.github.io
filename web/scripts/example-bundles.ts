/** Explicit, hash-pinned teaching archives only. Never extracts or executes archives. */
import {readFile,realpath,lstat} from 'node:fs/promises';
import {resolve,join} from 'node:path';
import {createHash} from 'node:crypto';
import {z} from 'zod';
const safeMember=z.string().min(1).refine(p=>!/[\x00-\x1f\x7f]/.test(p)&&!p.startsWith('/')&&!p.includes('\\')&&!p.includes(':')&&!p.split('/').some(part=>!part||part==='.'||part==='..'),'Unsafe archive member path');
const manifestSchema=z.object({version:z.literal(1),bundles:z.array(z.object({
 id:z.string().regex(/^[a-z0-9]+(?:-[a-z0-9]+)*$/),
 title:z.string().optional(),bytes:z.number().int().nonnegative().optional(),readme:safeMember.optional(),
 path:z.string(),sha256:z.string().regex(/^[a-f0-9]{64}$/),
 files:z.array(safeMember).min(1),sourceFiles:z.array(z.object({path:safeMember,source:safeMember,sha256:z.string().regex(/^[a-f0-9]{64}$/)}).strict()).optional(),pages:z.array(z.string().regex(/^[a-z0-9]+(?:-[a-z0-9]+)*$/)).optional()
}).strict())}).strict();
export async function validatedExampleBundles(root:string):Promise<Map<string,Buffer>>{
 const manifest=join(root,'knowledge/examples/catalog.json');
 let raw:string;
 try{raw=await readFile(manifest,'utf8');}catch(error){if((error as NodeJS.ErrnoException).code==='ENOENT')return new Map();throw error;}
 if(await realpath(manifest)!==resolve(manifest))throw Error('Example manifest must not be a symlink');
 const data=manifestSchema.parse(JSON.parse(raw)),result=new Map<string,Buffer>();
 for(const bundle of data.bundles){
  if(bundle.path!==`knowledge/examples/${bundle.id}.zip`||result.has(bundle.path))throw Error('Invalid or duplicate example bundle path: '+bundle.path);
  if(new Set(bundle.files).size!==bundle.files.length)throw Error('Duplicate example bundle member');
  const file=resolve(root,bundle.path);
  if(!(await lstat(file)).isFile()||await realpath(file)!==file)throw Error('Example bundle must be a regular contained file: '+bundle.path);
  const bytes=await readFile(file);
  if(createHash('sha256').update(bytes).digest('hex')!==bundle.sha256)throw Error('Example bundle SHA-256 mismatch: '+bundle.path);
  if(bundle.bytes!==undefined&&bundle.bytes!==bytes.length)throw Error('Example bundle byte length mismatch');
  result.set(bundle.path,bytes);
 }
 return result;
}
