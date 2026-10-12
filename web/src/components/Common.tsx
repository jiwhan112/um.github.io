import { Link } from 'react-router-dom';
import { ArrowRight, CircleCheck } from 'lucide-react';
import { Button } from './ui/button';
import { keys, objectBooleans, useStored } from '@/lib/storage';
export function PageHeader({eyebrow,title,description,children}:{eyebrow:string;title:string;description?:string;children?:React.ReactNode}){return <header className="page-header"><div><p className="eyebrow">{eyebrow}</p><h1>{title}</h1>{description&&<p className="page-description">{description}</p>}</div>{children}</header>;}
export function Panel({title,children,className=''}:{title?:string;children:React.ReactNode;className?:string}){return <section className={'panel '+className}>{title&&<h2 className="panel-title">{title}</h2>}{children}</section>;}
export function Empty({title='아직 자료가 없습니다.',description='다른 날짜나 분류를 선택해 주세요.'}:{title?:string;description?:string}){return <div className="empty-state"><h2>{title}</h2><p>{description}</p></div>;}
export function ReadCheck({path}:{path:string}){
 const [value,set]=useStored<unknown>(keys.reading,{}),state=objectBooleans(value),key=path.replace(/^\//,'');
 const checked=!!state[key];return <label className="check-row"><input type="checkbox" checked={checked} onChange={e=>set({...state,[key]:e.target.checked})}/><CircleCheck size={16}/><span>이 문서 학습 완료</span></label>;
}
export function NextPrevious({items,current}:{items:{route:string;title:string}[];current:string}){
 const index=items.findIndex(i=>i.route===current);return <nav className="next-previous" aria-label="이전 다음 문서">{[index-1,index+1].map((i,j)=>items[i]?<Link key={i} to={items[i].route}><small>{j===0?'이전':'다음'}</small><strong>{items[i].title}</strong><ArrowRight size={16}/></Link>:<span key={'empty'+j}/>)}</nav>;
}
export function Loading(){return <div className="empty-state" role="status">자료를 불러오는 중입니다…</div>;}
export function ErrorView({message}:{message:string}){return <div className="empty-state" role="alert"><h2>자료를 불러오지 못했습니다.</h2><p>{message}</p><Button variant="outline" onClick={()=>location.reload()}>다시 불러오기</Button></div>;}
