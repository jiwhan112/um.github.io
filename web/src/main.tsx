import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import App from './App';
import './styles.css';

createRoot(document.getElementById('root')!).render(<StrictMode><App/></StrictMode>);
// Same language-study scope as the previous PWA. Never clear localStorage or unrelated caches.
if('serviceWorker' in navigator){window.addEventListener('load',()=>{
 const root=new URL(import.meta.env.BASE_URL,location.origin);
 navigator.serviceWorker.register(new URL('language-study/sw.js',root).href,{scope:new URL('language-study/',root).pathname,updateViaCache:'none'}).catch(()=>{});
});}
