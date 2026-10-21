import { useId, useState } from 'react';
import './cpp-bitmask-lab.css';

type Operation = 'set' | 'clear' | 'toggle';
type Step = { before: number; mask: number; after: number; bit: number; operation: Operation };
const operations: Record<Operation, { label: string; symbol: string; explanation: string }> = {
  set: { label: '켜기', symbol: '|=', explanation: '이미 켜진 비트에 다시 적용해도 켜진 상태를 유지합니다.' },
  clear: { label: '끄기', symbol: '&= ~', explanation: '이미 꺼진 비트에 다시 적용해도 꺼진 상태를 유지합니다.' },
  toggle: { label: '뒤집기', symbol: '^=', explanation: '현재 상태를 반대로 바꿉니다. 두 번 적용하면 원래 상태로 돌아옵니다.' },
};
const bits = [7, 6, 5, 4, 3, 2, 1, 0];
const binary = (value: number) => value.toString(2).padStart(8, '0');

export function CppBitmaskLab() {
  const id = useId();
  const [flags, setFlags] = useState(0);
  const [selectedBit, setSelectedBit] = useState(2);
  const [last, setLast] = useState<Step | null>(null);

  function apply(operation: Operation) {
    // 범위를 0~255, 비트 번호를 0~7로 제한한 설명용 모형입니다.
    const mask = 1 << selectedBit;
    const after = operation === 'set' ? flags | mask : operation === 'clear' ? flags & ~mask : flags ^ mask;
    setLast({ before: flags, mask, after, bit: selectedBit, operation });
    setFlags(after);
  }

  return <section className="cpp-bitmask-lab" aria-labelledby={id + '-title'}>
    <div className="bitmask-lab-heading"><span>직접 바꿔 보기</span><h3 id={id + '-title'}>켜기·끄기·뒤집기는 어떻게 다를까?</h3></div>
    <p>비트 하나를 선택한 뒤 연산을 실행하세요. 같은 버튼을 두 번 눌러 보면 차이가 드러납니다.</p>
    <div className="bitmask-current"><span>현재 flags</span><output aria-label="현재 flags의 십진수 값">{flags}</output><code aria-label="현재 flags의 이진수 값">{binary(flags)}</code></div>
    <div className="bitmask-bits" role="group" aria-label="조작할 비트 선택">
      {bits.map(bit => <button key={bit} type="button" aria-label={`비트 ${bit} 선택 · 값 ${2 ** bit}`} aria-pressed={selectedBit === bit} onClick={() => setSelectedBit(bit)}>
        <span className="bitmask-index">비트 {bit}</span>
        <strong className={(flags & (1 << bit)) ? 'is-on' : ''}>{(flags & (1 << bit)) ? '1' : '0'}</strong>
        <span className="bitmask-weight">{2 ** bit}</span>
      </button>)}
    </div>
    <p className="bitmask-selection">선택한 마스크: <code>{binary(1 << selectedBit)}</code> · 값 {1 << selectedBit}</p>
    <div className="bitmask-actions" role="group" aria-label="선택한 비트에 적용할 연산">
      {(Object.entries(operations) as [Operation, typeof operations[Operation]][]).map(([operation, info]) => <button type="button" key={operation} onClick={() => apply(operation)}>{info.label} <code>{info.symbol}</code></button>)}
    </div>
    <div className="bitmask-last" role="status" aria-live="polite" aria-atomic="true">
      {last ? <>
        <strong>마지막 실행 · 비트 {last.bit} {operations[last.operation].label}</strong>
        <dl>
          <div><dt>이전 값</dt><dd><code>{binary(last.before)}</code><span>{last.before}</span></dd></div>
          <div><dt>마스크</dt><dd><code>{binary(last.mask)}</code><span>{last.mask}</span></dd></div>
          <div><dt>결과</dt><dd><code>{binary(last.after)}</code><span>{last.after}</span></dd></div>
        </dl>
        <p>{operations[last.operation].explanation}</p>
        <code className="bitmask-cpp">flags {operations[last.operation].symbol} (std::uint32_t&#123;1&#125; &lt;&lt; {last.bit});</code>
      </> : <p>아직 실행하지 않았습니다. 비트 2를 켜면 00000100, 십진수 4가 됩니다.</p>}
    </div>
    <button className="bitmask-reset" type="button" onClick={() => { setFlags(0); setSelectedBit(2); setLast(null); }}>처음 상태로</button>
    <details><summary>8칸 모형과 C++ 코드의 차이</summary><p>이 브라우저 실습은 0~255의 하위 8비트만 보여 주는 설명용 모형입니다. 아래 C++ 예제의 std::uint32_t는 제공되는 환경에서 32비트이며, 비트 번호는 0~31 범위로 제한해야 합니다. 여기서 C++를 컴파일하거나 정수 승격·메모리 배치를 재현하지는 않습니다.</p></details>
  </section>;
}
