import { Fragment, useState } from 'react';
import { Link } from 'react-router-dom';
import { Copy, ZoomIn } from 'lucide-react';
import { Button } from './ui/button';
import { Dialog, DialogContent, DialogDescription, DialogTitle, DialogTrigger } from './ui/dialog';
import { asset } from '@/lib/utils';
import type { Block, Inline } from '@/lib/types';
import { MermaidDiagram } from './MermaidDiagram';
import { AlphaBlendLab } from './AlphaBlendLab';
import { CppBitmaskLab } from './CppBitmaskLab';

function ContentLink({href,children}:{href:string;children:React.ReactNode}){
 if(/^https?:\/\//.test(href))return <a href={href} target="_blank" rel="noopener noreferrer">{children}</a>;
 if(href.startsWith('#'))return <a href={href}>{children}</a>;
 if(/\.(json|png|webp|jpe?g|gif|svg|md|zip|wav|glb|cpp|cxx|cc|c|hpp|hxx|hh|h|hlsl|py|rc|manifest|fx|sh|txt|log|pdf)(?:$|[?#])/i.test(href))return <a href={asset(href)}>{children}</a>;
 return <Link to={href}>{children}</Link>;
}
export function RichText({content}:{content:Inline[]}){return <>{content.map((n,i)=>n.kind==='strong'?<strong key={i}>{n.text}</strong>:n.kind==='em'?<em key={i}>{n.text}</em>:n.kind==='code'?<code key={i}>{n.text}</code>:n.kind==='br'?<br key={i}/>:n.kind==='link'&&n.href?<ContentLink key={i} href={n.href}>{n.text}</ContentLink>:<Fragment key={i}>{n.text}</Fragment>)}</>;}
export function CodeBlock({code,language='cpp'}:{code:string;language?:string}){
 const [status,setStatus]=useState('복사');return <div className="code-block"><div className="code-toolbar"><span>{language.toUpperCase()}</span><Button variant="ghost" size="sm" onClick={async()=>{try{await navigator.clipboard.writeText(code);setStatus('복사됨');}catch{setStatus('직접 선택해서 복사');}}}><Copy/>{status}</Button></div><pre tabIndex={0}><code>{code}</code></pre></div>;
}
export function AssetImage({src,alt,caption}:{src:string;alt:string;caption?:string}){
 const [originalSize,setOriginalSize]=useState(false),[naturalWidth,setNaturalWidth]=useState(0);
 const url=/^https?:\/\//.test(src)?src:asset(src);
 const gameDesignIllustration=src.startsWith('/game-design/');
 const knowledgeIllustration=src.startsWith('/knowledge/');
 return <figure className={gameDesignIllustration?'content-image game-design-illustration':knowledgeIllustration?'content-image knowledge-illustration':'content-image'}>
  <Dialog onOpenChange={(open)=>{if(open)setOriginalSize(false);}}>
   <DialogTrigger asChild><button type="button" className="image-zoom" aria-label={`${alt} 확대`}><img src={url} alt={alt} loading="lazy" onLoad={knowledgeIllustration?(event)=>setNaturalWidth(event.currentTarget.naturalWidth):undefined}/><ZoomIn size={16}/></button></DialogTrigger>
   <DialogContent className={gameDesignIllustration?'image-dialog game-design-image-dialog':knowledgeIllustration?'image-dialog knowledge-image-dialog':'image-dialog'}>
    <DialogTitle>{alt}</DialogTitle>
    <DialogDescription>{knowledgeIllustration?'화면에 맞추거나 원본 크기로 읽을 수 있습니다. 큰 그림은 안쪽에서 스크롤하세요.':'원본 파일을 화면 크기에 맞춰 확대합니다.'}</DialogDescription>
    {knowledgeIllustration?<>
     <div className="knowledge-image-controls" aria-label="그림 표시 크기">
      <Button type="button" size="sm" variant={originalSize?'outline':'default'} aria-pressed={!originalSize} onClick={()=>setOriginalSize(false)}>화면에 맞춤</Button>
      <Button type="button" size="sm" variant={originalSize?'default':'outline'} aria-pressed={originalSize} disabled={!naturalWidth} onClick={()=>setOriginalSize(true)}>원본 크기</Button>
     </div>
     <div className={'knowledge-image-viewport'+(originalSize?' is-original':'')} tabIndex={0} aria-label="확대한 그림 · 원본 크기에서 스크롤 가능">
      <img src={url} alt={alt} onLoad={(event)=>setNaturalWidth(event.currentTarget.naturalWidth)} style={originalSize?{width:naturalWidth,maxWidth:'none',height:'auto'}:undefined}/>
     </div>
    </>:<img src={url} alt={alt}/>}
    <Button asChild variant="outline"><a href={url} download>원본 이미지 저장</a></Button>
   </DialogContent>
  </Dialog>
  {caption&&<figcaption>{caption}</figcaption>}
 </figure>;
}
function RenderBlock({block:b}:{block:Block}){
 switch(b.type){
 case 'paragraph':return <p><RichText content={b.content}/></p>;
 case 'heading':return b.level===3||b.level===4?<h3 id={b.id}><RichText content={b.content}/></h3>:<h2 id={b.id}><RichText content={b.content}/></h2>;
 case 'code':return b.language.trim().toLowerCase()==='mermaid'?<MermaidDiagram code={b.text}><CodeBlock code={b.text} language={b.language}/></MermaidDiagram>:<CodeBlock code={b.text} language={b.language}/>;
 case 'image':return <AssetImage src={b.src} alt={b.alt} caption={b.caption}/>;
 case 'details':return <details className="disclosure"><summary>{b.title}</summary><Blocks blocks={b.blocks}/></details>;
 case 'table':return <div className="table-scroll" tabIndex={0}><table><tbody>{b.rows.map((r,i)=><tr key={i}>{r.cells.map((c,j)=>r.header?<th key={j} scope="col"><RichText content={c}/></th>:<td key={j}><RichText content={c}/></td>)}</tr>)}</tbody></table></div>;
 case 'list':{const Tag=b.ordered?'ol':'ul';return <Tag>{b.items.map((item,i)=><li key={i}><Blocks blocks={item}/></li>)}</Tag>;}
 case 'diagram':return <figure className="diagram"><div>{b.items.map((s,i)=><span key={i}>{s}</span>)}</div></figure>;
 case 'alpha-blend-lab':return <AlphaBlendLab/>;
 case 'cpp-bitmask-lab':return <CppBitmaskLab/>;
 case 'grid':return <div className="content-grid"><Blocks blocks={b.blocks}/></div>;
 case 'callout':return <aside className="callout"><Blocks blocks={b.blocks}/></aside>;
 case 'group':return <div className="content-group"><Blocks blocks={b.blocks}/></div>;
 }
}
export function Blocks({blocks}:{blocks:Block[]}){return <>{blocks.map((block,i)=><RenderBlock key={i} block={block}/>)}</>;}
