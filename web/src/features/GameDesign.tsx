import { Bookmark, PlayCircle } from 'lucide-react';
import { keys, useStored } from '@/lib/storage';
import { Button } from '@/components/ui/button';
import { LearningIndex } from './Learning';

export function GameDesignBookmark({route}:{route:string}){
 const [value,setValue]=useStored<unknown>(keys.gameDesignBookmarks,[]);
 const saved=Array.isArray(value)?value.filter((v):v is string=>typeof v==='string'):[];
 const active=saved.includes(route);
 return <Button size="sm" variant={active?'secondary':'outline'} aria-pressed={active} onClick={()=>setValue(active?saved.filter(v=>v!==route):[...saved,route])}><Bookmark size={15}/>{active?'북마크됨':'북마크 저장'}</Button>;
}
export default function GameDesignIndex(){
 return <LearningIndex kind="game-design" title="게임 기획 연구실" eyebrow="GAME DESIGN LAB / MASAHIRO SAKURAI"
  description="사쿠라이의 게임 설계 강의를 읽고, C++ 게임 프로그래머의 구현 관점으로 확장합니다."
  after={<p className="learning-footnote"><a className="text-link" href="https://www.youtube.com/@sora_sakurai_en/videos" target="_blank" rel="noopener noreferrer"><PlayCircle size={15}/>원본 YouTube 채널</a><span>회차 순서는 원본 공개일 기준이며, 주제는 원본 강의의 시리즈 분류를 따릅니다.</span></p>}/>;
}
