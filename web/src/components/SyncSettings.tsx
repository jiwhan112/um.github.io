import { useState, useSyncExternalStore } from 'react';
import { Cloud, Download, RefreshCw } from 'lucide-react';
import { Dialog, DialogContent, DialogDescription, DialogTitle } from './ui/dialog';
import { Button } from './ui/button';
import { siteSupabase } from '@/config/supabase';
import { cloudSettings, useSiteCloudSettings, cloudStatus, cloudSubscribe, signInCloud, signOutCloud, syncCloud, canSyncKey } from '@/lib/cloud';

export function SyncSettings({open,onOpenChange}:{open:boolean;onOpenChange:(value:boolean)=>void}){
 const status=useSyncExternalStore(cloudSubscribe,cloudStatus,()=> '로그인 필요');
 const [notice,setNotice]=useState('');
 const isDefault=cloudSettings().url===siteSupabase.url;
 function login(){try{signInCloud();}catch(e){setNotice(String(e instanceof Error?e.message:e));}}
 function restoreDefault(){try{useSiteCloudSettings();setNotice('기본 서버로 전환했습니다. 로컬 기록은 유지됩니다. GitHub 계정으로 로그인해 주세요.');}catch(e){setNotice(String(e instanceof Error?e.message:e));}}
 function exportLocal(){
  const result:Record<string,string>={};
  for(const key of Object.keys(localStorage).filter(canSyncKey))result[key]=localStorage.getItem(key)||'';
  const blob=new Blob([JSON.stringify({version:1,exported_at:new Date().toISOString(),records:result},null,2)],{type:'application/json'});
  const url=URL.createObjectURL(blob),link=document.createElement('a');link.href=url;link.download='atlas-learning-backup.json';link.click();URL.revokeObjectURL(url);
  setNotice('현재 브라우저의 학습 기록 백업 파일을 만들었습니다.');
 }
 return <Dialog open={open} onOpenChange={onOpenChange}><DialogContent className="sync-settings"><DialogTitle>나의 학습 기록 · 동기화</DialogTitle><DialogDescription>학습자 번호 1 · 브리핑 읽음/북마크, 언어 레슨/복습, 사고력/문서 체크와 풀이 메모를 한곳에서 관리합니다.</DialogDescription>
 <p className="sync-status" role="status"><Cloud size={16}/>{status}</p>
 {isDefault?<p className="muted text-xs">ATLAS NOTE 서버가 기본 설정되어 있습니다. 모바일이나 다른 브라우저에서도 주소·키 입력 없이 GitHub 로그인만 하면 됩니다.</p>:<div><p className="muted text-xs">이 브라우저에는 다른 서버의 연결 설정이 남아 있습니다. 기존 로그인과 기록을 보호하기 위해 자동으로 변경하지 않았습니다. 기본 서버로 전환하면 다시 로그인해야 합니다.</p><Button variant="outline" onClick={restoreDefault}>ATLAS NOTE 기본 서버로 전환</Button></div>}
 <div className="sync-actions"><Button onClick={login}>GitHub 로그인</Button><Button variant="outline" disabled={status.startsWith('동기화 중')} onClick={()=>{setNotice('');void syncCloud().catch(e=>setNotice(e instanceof Error?e.message:String(e)));}}><RefreshCw size={15}/>지금 동기화</Button></div>
 <div className="sync-actions"><Button variant="ghost" onClick={exportLocal}><Download size={15}/>로컬 백업</Button><Button variant="ghost" onClick={()=>{void signOutCloud().catch(()=>{});setNotice('이 기기에서 로그아웃했습니다. 로컬 학습 기록은 유지됩니다.');}}>로그아웃</Button></div>
 {notice&&<p role="status" className="muted text-xs">{notice}</p>}
 <p className="muted text-xs">로그인 전이나 서버 연결이 끊긴 동안에도 기록은 이 브라우저에 저장됩니다. GitHub 로그인과 서버의 학습자 1번 접근 권한이 확인된 뒤 동기화됩니다.</p>
 <p className="muted text-xs">문제가 계속되면 로컬 백업을 먼저 보관하세요. 서버 관리자는 저장소의 <code>web/SYNC_SETUP.md</code>에서 초기 권한 설정을 확인할 수 있습니다.</p>
 </DialogContent></Dialog>;
}
