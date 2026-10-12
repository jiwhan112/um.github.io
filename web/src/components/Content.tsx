import { Fragment, useState } from 'react';
import { Link } from 'react-router-dom';
import { Copy, ZoomIn } from 'lucide-react';
import { Button } from './ui/button';
import { Dialog, DialogContent, DialogDescription, DialogTitle, DialogTrigger } from './ui/dialog';
import { asset } from '@/lib/utils';
import type { Block, Inline } from '@/lib/types';

function ContentLink({href,children}:{href:string;children:React.ReactNode}){
 if(/^https?:\/\//.test(href))return <a href={href} target="_blank" rel="noopener noreferrer">{children}</a>;
 if(href.startsWith('#'))return <a href={href}>{children}</a>;
 if(/\.(json|png|webp|md|zip|wav|glb)(?:$|\?)/i.test(href))return <a href={asset(href)}>{children}</a>;
 return <Link to={href}>{children}</Link>;
}
export function RichText({content}:{content:Inline[]}){return <>{content.map((n,i)=>n.kind==='strong'?<strong key={i}>{n.text}</strong>:n.kind==='em'?<em key={i}>{n.text}</em>:n.kind==='code'?<code key={i}>{n.text}</code>:n.kind==='br'?<br key={i}/>:n.kind==='link'&&n.href?<ContentLink key={i} href={n.href}>{n.text}</ContentLink>:<Fragment key={i}>{n.text}</Fragment>)}</>;}
export function CodeBlock({code,language='cpp'}:{code:string;language?:string}){
 const [status,setStatus]=useState('복사');return <div className="code-block"><div className="code-toolbar"><span>{language.toUpperCase()}</span><Button variant="ghost" size="sm" onClick={async()=>{try{await navigator.clipboard.writeText(code);setStatus('복사됨');}catch{setStatus('직접 선택해서 복사');}}}><Copy/>{status}</Button></div><pre tabIndex={0}><code>{code}</code></pre></div>;
}
export function AssetImage({src,alt,caption}:{src:string;alt:string;caption?:string}){
 const url=/^https?:\/\//.test(src)?src:asset(src);
 return <figure className="content-image"><Dialog><DialogTrigger asChild><button className="image-zoom" aria-label={`${alt} 확대`}><img src={url} alt={alt} loading="lazy"/><ZoomIn size={16}/></button></DialogTrigger><DialogContent className="image-dialog"><DialogTitle>{alt}</DialogTitle><DialogDescription>원본 파일을 화면 크기에 맞춰 확대합니다.</DialogDescription><img src={url} alt={alt}/><Button asChild variant="outline"><a href={url} download>PNG 저장</a></Button></DialogContent></Dialog>{caption&&<figcaption>{caption}</figcaption>}</figure>;
}
function RenderBlock({block:b}:{block:Block}){
 switch(b.type){
 case 'paragraph':return <p><RichText content={b.content}/></p>;
 case 'heading':return b.level===3||b.level===4?<h3 id={b.id}><RichText content={b.content}/></h3>:<h2 id={b.id}><RichText content={b.content}/></h2>;
 case 'code':return <CodeBlock code={b.text} language={b.language}/>;
 case 'image':return <AssetImage src={b.src} alt={b.alt} caption={b.caption}/>;
 case 'details':return <details className="disclosure"><summary>{b.title}</summary><Blocks blocks={b.blocks}/></details>;
 case 'table':return <div className="table-scroll" tabIndex={0}><table><tbody>{b.rows.map((r,i)=><tr key={i}>{r.cells.map((c,j)=>r.header?<th key={j} scope="col"><RichText content={c}/></th>:<td key={j}><RichText content={c}/></td>)}</tr>)}</tbody></table></div>;
 case 'list':{const Tag=b.ordered?'ol':'ul';return <Tag>{b.items.map((item,i)=><li key={i}><Blocks blocks={item}/></li>)}</Tag>;}
 case 'diagram':return <figure className="diagram"><div>{b.items.map((s,i)=><span key={i}>{s}</span>)}</div></figure>;
 case 'grid':return <div className="content-grid"><Blocks blocks={b.blocks}/></div>;
 case 'callout':return <aside className="callout"><Blocks blocks={b.blocks}/></aside>;
 case 'group':return <div className="content-group"><Blocks blocks={b.blocks}/></div>;
 }
}
export function Blocks({blocks}:{blocks:Block[]}){return <>{blocks.map((block,i)=><RenderBlock key={i} block={block}/>)}</>;}
