import { useEffect, useId, useRef, useState, type ReactNode } from 'react';
import type { MermaidConfig } from 'mermaid';
import { RefreshCw, ZoomIn } from 'lucide-react';
import { Button } from './ui/button';
import { Dialog, DialogContent, DialogDescription, DialogTitle, DialogTrigger } from './ui/dialog';
import './MermaidDiagram.css';

// 문서 안의 directive가 사이트의 보안·표시 설정을 덮어쓰지 못하게 고정합니다.
const configuration: MermaidConfig = {
  startOnLoad: false,
  securityLevel: 'strict',
  htmlLabels: false,
  theme: 'dark',
  look: 'classic',
  layout: 'dagre',
  fontFamily: 'Arial, "Noto Sans KR", "Malgun Gothic", sans-serif',
  fontSize: 16,
  maxTextSize: 50_000,
  maxEdges: 500,
  suppressErrorRendering: true,
  arrowMarkerAbsolute: false,
  flowchart: { htmlLabels: false, useMaxWidth: false, wrappingWidth: 220 },
  secure: [
    'secure', 'securityLevel', 'startOnLoad', 'maxTextSize', 'maxEdges',
    'suppressErrorRendering', 'htmlLabels', 'flowchart', 'theme', 'themeVariables',
    'themeCSS', 'fontFamily', 'fontSize', 'look', 'layout', 'arrowMarkerAbsolute',
    'dompurifyConfig',
  ],
};

let renderer: Promise<(typeof import('mermaid'))['default']> | undefined;
function getRenderer() {
  renderer ??= import('mermaid').then(({ default: mermaid }) => {
    mermaid.initialize(configuration);
    return mermaid;
  }).catch((error: unknown) => {
    renderer = undefined;
    throw error;
  });
  return renderer;
}

type SvgImage = { svg: string; width: number; height: number; title: string; description: string };
type DiagramState = { status: 'loading' } | { status: 'ready'; image: Omit<SvgImage, 'svg'> & { url: string } } | { status: 'error'; message: string };
type Zoom = 'fit' | 1 | 1.5 | 2;

function cleanText(value: string | null | undefined) {
  return (value ?? '').replace(/\s+/g, ' ').trim();
}

function checkSource(code: string) {
  if (code.length > 50_000) throw new Error('도식 원문이 표시 한도인 50,000자를 넘었습니다.');
  // 외부 자원을 불러올 수 있는 구문은 임시 렌더 단계에 들어가기 전에 거부합니다.
  // 원문은 그대로 남기며, 본문 그림은 별도의 image 블록으로 다룹니다.
  if (/<\s*(?:img|image|script|iframe|object|embed|svg)\b|(?:^|[\s,{])["']?(?:img|image)["']?\s*:|url\s*\(|@import\b/i.test(code)) {
    throw new Error('도식 안의 이미지·외부 리소스·HTML 실행 요소는 지원하지 않습니다. 아래 원문을 확인해주세요.');
  }
}

function svgImage(svg: string): SvgImage {
  if (svg.length > 2_000_000 || /<!DOCTYPE|<!ENTITY/i.test(svg)) {
    throw new Error('안전하게 표시할 수 없는 SVG 형식입니다.');
  }
  const doc = new DOMParser().parseFromString(svg, 'image/svg+xml');
  const root = doc.documentElement;
  if (doc.querySelector('parsererror') || root.localName !== 'svg' || root.namespaceURI !== 'http://www.w3.org/2000/svg') {
    throw new Error('생성된 도식의 SVG 형식을 읽지 못했습니다.');
  }
  const forbidden = new Set(['script', 'foreignobject', 'iframe', 'object', 'embed', 'image', 'a', 'animate', 'animatemotion', 'animatetransform', 'set']);
  const checkCss = (value: string) => {
    if (/@import\b/i.test(value)) throw new Error('도식에 외부 스타일 참조가 포함되어 있습니다.');
    for (const match of value.matchAll(/url\s*\(([^)]*)\)/gi)) {
      const target = match[1].trim().replace(/^['"]|['"]$/g, '');
      if (!target.startsWith('#')) throw new Error('도식에 외부 리소스 참조가 포함되어 있습니다.');
    }
  };
  for (const element of [root, ...root.querySelectorAll('*')]) {
    if (element.namespaceURI !== 'http://www.w3.org/2000/svg' || forbidden.has(element.localName.toLowerCase())) {
      throw new Error('도식에 지원하지 않는 요소가 포함되어 있습니다. 원문을 확인해주세요.');
    }
    for (const attribute of element.attributes) {
      const name = attribute.localName.toLowerCase();
      if (name.startsWith('on') || (name === 'href' && !attribute.value.trim().startsWith('#'))) {
        throw new Error('도식의 상호작용 또는 외부 링크를 허용할 수 없습니다.');
      }
      checkCss(attribute.value);
    }
    if (element.localName === 'style') checkCss(element.textContent ?? '');
  }

  const viewBox = (root.getAttribute('viewBox') ?? '').trim().split(/[\s,]+/).map(Number);
  const [, , width, height] = viewBox;
  if (viewBox.length !== 4 || !viewBox.every(Number.isFinite) || width <= 0 || height <= 0 || width > 50_000 || height > 50_000) {
    throw new Error('도식의 표시 크기를 확인하지 못했습니다.');
  }
  const title = cleanText(root.querySelector('title')?.textContent) || '학습 도식';
  const labels = [...new Set([...root.querySelectorAll('text')].map((node) => cleanText(node.textContent)).filter(Boolean))];
  const description = cleanText(root.querySelector('desc')?.textContent)
    || (labels.length ? `도식에 표시된 항목: ${labels.join(' · ')}` : '본문의 관계와 처리 순서를 보여주는 도식입니다. 아래에서 도식 원문도 확인할 수 있습니다.');

  // SVG 문자열을 페이지 HTML에 삽입하지 않습니다. 독립 이미지의 크기와 내부 참조만 유지합니다.
  root.removeAttribute('style');
  root.setAttribute('xmlns', 'http://www.w3.org/2000/svg');
  root.setAttribute('width', String(Math.ceil(width)));
  root.setAttribute('height', String(Math.ceil(height)));
  return { svg: new XMLSerializer().serializeToString(root), width, height, title, description };
}

export function MermaidDiagram({ code, children }: { code: string; children: ReactNode }) {
  const id = `atlas-mermaid-${useId().replace(/[^a-zA-Z0-9_-]/g, '')}`;
  const stage = useRef<HTMLDivElement>(null);
  const [state, setState] = useState<DiagramState>({ status: 'loading' });
  const [attempt, setAttempt] = useState(0);
  const [zoom, setZoom] = useState<Zoom>('fit');
  const [open, setOpen] = useState(false);

  useEffect(() => {
    let cancelled = false;
    let objectUrl: string | undefined;
    setState({ status: 'loading' });
    void (async () => {
      checkSource(code);
      await document.fonts?.ready;
      const mermaid = await getRenderer();
      if (cancelled || !stage.current) return;
      // Mermaid의 공개 render API는 여러 도식 요청을 내부 큐에서 순차 처리합니다.
      const { svg } = await mermaid.render(`${id}-${attempt}`, code, stage.current);
      if (cancelled) return;
      const { svg: imageSvg, ...image } = svgImage(svg);
      objectUrl = URL.createObjectURL(new Blob([imageSvg], { type: 'image/svg+xml' }));
      setState({ status: 'ready', image: { ...image, url: objectUrl } });
    })().catch((error: unknown) => {
      if (!cancelled) setState({ status: 'error', message: error instanceof Error ? error.message.slice(0,800) : '도식 처리 중 오류가 발생했습니다.' });
    });
    return () => {
      cancelled = true;
      if (objectUrl) URL.revokeObjectURL(objectUrl);
    };
  }, [code, id, attempt]);

  const image = state.status === 'ready' ? state.image : undefined;
  return <figure className="mermaid-diagram" data-diagram-state={state.status}>
    <div ref={stage} className="mermaid-render-stage" aria-hidden="true" inert />
    <Dialog open={open} onOpenChange={(next) => { setOpen(next); if (next) setZoom('fit'); }}>
      <div className="mermaid-toolbar">
        <span className="mermaid-label">{image?.title ?? '학습 도식'}</span>
        <DialogTrigger asChild><Button type="button" variant="outline" size="sm" disabled={!image} aria-label="도식 확대해서 보기"><ZoomIn size={15}/><span>확대해서 보기</span></Button></DialogTrigger>
      </div>
      {state.status === 'loading' && <p className="mermaid-status" role="status">도식을 불러오는 중입니다…</p>}
      {state.status === 'error' && <div className="mermaid-error" role="alert">
        <p>도식을 표시하지 못했습니다. 아래에서 원문을 확인할 수 있습니다.</p>
        <details><summary>오류 내용 보기</summary><pre>{state.message}</pre></details>
        <Button type="button" variant="outline" size="sm" onClick={() => setAttempt((value) => value + 1)}><RefreshCw size={14}/>다시 시도</Button>
      </div>}
      {image && <>
        <div className="mermaid-canvas" tabIndex={0} aria-label="도식 영역 · 큰 도식은 가로와 세로로 스크롤할 수 있습니다">
          <img src={image.url} alt={`${image.title}. ${image.description}`} width={Math.ceil(image.width)} height={Math.ceil(image.height)} onError={() => setState({ status: 'error', message: '생성된 도식 이미지를 브라우저에서 표시하지 못했습니다.' })}/>
        </div>
        <figcaption className="mermaid-caption">큰 도식은 스크롤하거나 확대해서 볼 수 있습니다.</figcaption>
        <details className="mermaid-description"><summary>도식 내용 읽기</summary><p>{image.description}</p></details>
        <DialogContent className="mermaid-dialog">
          <DialogTitle>{image.title} 확대</DialogTitle>
          <DialogDescription>화면에 맞추거나 배율을 선택하세요. 큰 도식은 안쪽에서 스크롤할 수 있습니다.</DialogDescription>
          <div className="mermaid-zoom-controls" aria-label="도식 배율">
            {(['fit', 1, 1.5, 2] as const).map((value) => <Button type="button" key={value} size="sm" variant={zoom === value ? 'default' : 'outline'} aria-pressed={zoom === value} onClick={() => setZoom(value)}>{value === 'fit' ? '화면에 맞춤' : `${value * 100}%`}</Button>)}
            <Button asChild size="sm" variant="outline"><a href={image.url} download="atlas-note-diagram.svg">도식 저장</a></Button>
          </div>
          <div className="mermaid-zoom-canvas" tabIndex={0} aria-label="확대한 도식 · 스크롤 가능">
            <img src={image.url} alt={`${image.title}. ${image.description}`} style={{ width: Math.ceil(image.width * (zoom === 'fit' ? 1 : zoom)), maxWidth: zoom === 'fit' ? '100%' : 'none' }}/>
          </div>
        </DialogContent>
      </>}
    </Dialog>
    <details className="mermaid-source"><summary>도식 원문 보기</summary>{children}</details>
  </figure>;
}
