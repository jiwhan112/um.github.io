import { useId, useState } from 'react';
import './alpha-blend-lab.css';

type RGB = readonly [number, number, number];
type Mode = 'straight' | 'premultiplied' | 'additive';
const source: RGB = [1, 0, 0];
const background: RGB = [0.2, 0.4, 0.6];
const names: Record<Mode, string> = {
  straight: 'Straight · 원래 RGB에 알파를 곱하기',
  premultiplied: 'Premultiplied · 알파가 곱해진 RGB 사용',
  additive: 'Additive · 배경에 빛을 더하기',
};
const formulas: Record<Mode, string> = {
  straight: '결과 = 소스 × α + 배경 × (1 − α)',
  premultiplied: '결과 = 미리 곱한 소스 + 배경 × (1 − α)',
  additive: '결과 = 소스 × α + 배경',
};

function rgbText(rgb: RGB): string {
  return `(${rgb.map(channel => channel.toFixed(2)).join(', ')})`;
}

// 계산은 선형 RGB로 하고, 모니터에 보여줄 때만 sRGB로 변환한다.
// 1을 넘는 가산 결과는 숫자에 남기고 색 견본에서만 범위를 제한한다.
function displayColor(rgb: RGB): string {
  const srgb = rgb.map(channel => {
    const linear = Math.max(0, Math.min(1, channel));
    const encoded = linear <= 0.0031308 ? 12.92 * linear : 1.055 * Math.pow(linear, 1 / 2.4) - 0.055;
    return Math.round(encoded * 255);
  });
  return `rgb(${srgb.join(' ')})`;
}

function Swatch({ label, rgb, result = false }: { label: string; rgb: RGB; result?: boolean }) {
  return <div className={'blend-swatch' + (result ? ' blend-swatch-result' : '')}>
    <span className="blend-swatch-color" style={{ backgroundColor: displayColor(rgb) }} aria-hidden="true" />
    <strong>{label}</strong>
    <code>{rgbText(rgb)}</code>
  </div>;
}

export function AlphaBlendLab() {
  const id = useId();
  const [alphaPercent, setAlphaPercent] = useState(25);
  const [mode, setMode] = useState<Mode>('straight');
  const alpha = alphaPercent / 100;
  const premultiplied: RGB = source.map(channel => channel * alpha) as unknown as RGB;
  const input = mode === 'premultiplied' ? premultiplied : source;
  const result: RGB = background.map((channel, index) => {
    const src = mode === 'premultiplied' ? input[index] : input[index] * alpha;
    return src + channel * (mode === 'additive' ? 1 : 1 - alpha);
  }) as unknown as RGB;

  return <section className="alpha-blend-lab" aria-labelledby={id + '-title'}>
    <div className="blend-lab-heading"><span>직접 바꿔 보기</span><h3 id={id + '-title'}>알파가 바뀌면 색은 어떻게 달라질까?</h3></div>
    <p>소스는 빨강, 배경은 청회색으로 고정했습니다. 슬라이더를 움직인 뒤 합성 방식을 바꿔 보세요.</p>
    <label className="blend-mode-label" htmlFor={id + '-mode'}>합성 방식</label>
    <select id={id + '-mode'} value={mode} onChange={event => setMode(event.target.value as Mode)}>
      {Object.entries(names).map(([value, label]) => <option key={value} value={value}>{label}</option>)}
    </select>
    <div className="blend-alpha-label"><label htmlFor={id + '-alpha'}>소스 알파 α</label><output htmlFor={id + '-alpha'}>{alpha.toFixed(2)}</output></div>
    <input id={id + '-alpha'} type="range" min="0" max="100" step="1" value={alphaPercent} onChange={event => setAlphaPercent(Number(event.target.value))} aria-valuetext={alpha.toFixed(2)} />
    <div className="blend-range-labels" aria-hidden="true"><span>0 · 소스 기여 없음</span><span>1 · 소스 기여 최대</span></div>
    <div className="blend-swatches">
      <Swatch label={mode === 'premultiplied' ? '미리 곱한 소스 RGB' : '소스 RGB'} rgb={input}/>
      <Swatch label="배경 RGB" rgb={background}/>
      <Swatch label="합성 결과 RGB" rgb={result} result/>
    </div>
    <p className="blend-formula">{formulas[mode]}</p>
    <p className="blend-reading" role="status" aria-live="polite">α = {alpha.toFixed(2)} → 결과 {rgbText(result)}. {mode === 'premultiplied' ? '소스 RGB도 함께 바뀌므로, 올바른 Straight와 결과가 같습니다.' : mode === 'additive' ? '배경을 줄이지 않고 소스의 기여를 더합니다.' : '알파가 커질수록 소스 기여는 늘고 배경 기여는 줄어듭니다.'}</p>
    <details><summary>이 실습이 계산하는 범위</summary><p>선형 RGB의 수학적 합성만 계산하는 브라우저 실습입니다. 색 견본은 sRGB로 변환해 표시하고, 0~1 범위를 넘으면 표시할 때만 제한합니다. 깊이 검사, 알파 채널 저장, GPU 정밀도, MSAA, 실제 Direct3D 9 실행은 포함하지 않습니다.</p></details>
  </section>;
}
