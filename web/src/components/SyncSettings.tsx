import { useState, useSyncExternalStore } from 'react';
import { Cloud, Download, RefreshCw } from 'lucide-react';
import { Dialog, DialogContent, DialogDescription, DialogTitle } from './ui/dialog';
import { Button } from './ui/button';
import { cloudSettings, saveCloudSettings, cloudStatus, cloudSubscribe, signInCloud, signOutCloud, syncCloud, canSyncKey } from '@/lib/cloud';

export function SyncSettings({open,onOpenChange}:{open:boolean;onOpenChange:(value:boolean)=>void}){
 const status=useSyncExternalStore(cloudSubscribe,cloudStatus,()=> '미연결');
 const [config,setConfig]=useState(cloudSettings);
 const [notice,setNotice]=useState('');
 function save(){try{saveCloudSettings(config);setNotice('설정을 저장했습니다. 이제 GitHub 계정으로 로그인해 주세요.');}catch(e){setNotice(String(e instanceof Error?e.message:e));}}
 function login(){try{signInCloud();}catch(e){setNotice(String(e instanceof Error?e.message:e));}}
 function exportLocal(){
  const result:Record<string,string>={};
  for(const key of Object.keys(localStorage).filter(canSyncKey))result[key]=localStorage.getItem(key)||'';
  const blob=new Blob([JSON.stringify({version:1,exported_at:new Date().toISOString(),records:result},null,2)],{type:'application/json'});
  const url=URL.createObjectURL(blob),link=document.createElement('a');link.href=url;link.download='atlas-learning-backup.json';link.click();URL.revokeObjectURL(url);
  setNotice('현재 브라우저의 학습 기록 백업 파일을 만들었습니다.');
 }
 return <Dialog open={open} onOpenChange={onOpenChange}><DialogContent className="sync-settings"><DialogTitle>나의 학습 기록 · 동기화</DialogTitle><DialogDescription>학습자 번호 1 · 브리핑 읽음/북마크, 언어 레슨/복습, 사고력/문서 체크와 풀이 메모를 한곳에서 관리합니다.</DialogDescription>
 <p className="sync-status" role="status"><Cloud size={16}/>{status}</p>
 <p className="muted text-xs">동기화 설정 전에도 기존 기록은 브라우저에 보존됩니다. 다른 기기와 공유하려면 개인 Supabase 프로젝트를 연결해야 합니다.</p>
 <label className="sync-field">Supabase Project URL<input value={config.url} autoComplete="url" placeholder="https://xxxx.supabase.co" onChange={e=>setConfig({...config,url:e.target.value})}/></label>
 <label className="sync-field">Publishable / anon key<input value={config.key} autoComplete="off" placeholder="sb_publishable_... 또는 anon key" onChange={e=>setConfig({...config,key:e.target.value})}/></label>
 <p className="muted text-xs">절대로 service_role / secret key를 입력하지 마세요. GitHub 로그인 및 사용자 1번 전용 RLS 설정이 필요합니다.</p>
 <div className="sync-actions"><Button variant="outline" onClick={save}>연결 설정 저장</Button><Button onClick={login}>GitHub 로그인</Button><Button variant="outline" onClick={()=>{void syncCloud().catch(e=>setNotice(e instanceof Error?e.message:String(e)));}}><RefreshCw size={15}/>지금 동기화</Button></div>
 <div className="sync-actions"><Button variant="ghost" onClick={exportLocal}><Download size={15}/>로컬 백업</Button><Button variant="ghost" onClick={()=>{void signOutCloud().catch(()=>{});setNotice('이 기기에서 로그아웃했습니다. 로컬 학습 기록은 유지됩니다.');}}>로그아웃</Button></div>
 {notice&&<p role="status" className="muted text-xs">{notice}</p>}
 <p className="muted text-xs">준비 안내: 저장소의 <code>web/SYNC_SETUP.md</code>에서 데이터베이스·GitHub OAuth 초기 설정을 확인하세요.</p>
 </DialogContent></Dialog>;
}
