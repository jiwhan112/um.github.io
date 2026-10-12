import http from 'node:http';
import {readFile,stat} from 'node:fs/promises';
import {resolve,extname,join} from 'node:path';
const root=resolve('dist'),port=Number(process.env.PORT||4173),prefix='/um.github.io';
const types={'.html':'text/html; charset=utf-8','.js':'text/javascript; charset=utf-8','.css':'text/css; charset=utf-8','.json':'application/json; charset=utf-8','.svg':'image/svg+xml','.png':'image/png','.md':'text/plain; charset=utf-8','.xml':'application/xml','.txt':'text/plain; charset=utf-8'};
http.createServer(async(req,res)=>{
 try{const u=new URL(req.url,'http://localhost');if(!u.pathname.startsWith(prefix+'/')){res.writeHead(404);return res.end('Not Found');}
 let file=resolve(root,'.'+decodeURIComponent(u.pathname.slice(prefix.length)));if(file!==root&&!file.startsWith(root+'/'))throw Error('invalid path');
 if((await stat(file)).isDirectory())file=join(file,'index.html');const bytes=await readFile(file);res.writeHead(200,{'Content-Type':types[extname(file)]||'application/octet-stream','Cache-Control':'no-store'});res.end(bytes);
 }catch{res.writeHead(404,{'Content-Type':'text/html; charset=utf-8'});res.end(await readFile(join(root,'404.html'),'utf8'));}
}).listen(port,'0.0.0.0',()=>console.log('Atlas static preview on '+port));
