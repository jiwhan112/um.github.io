## 범위와 재조회
2026-10-19 KST. DX11 Instancing 13A 소스 색인과 상위 13 단원, 현재 CPU 실행 증거를 읽고 **기록된 두 SHA-256을 서로 다른 버전의 증거**로 구분했습니다.
<table header-row="true">
<tr>
<td>대상</td>
<td>SHA-256</td>
<td>판정</td>
</tr>
<tr>
<td>2026-10-16 기록상 main.cpp</td>
<td>fd3839f7c350d1e573cab36a8c3f942f55a8e6c9d4e94b45033e7d3b3b4472ae</td>
<td>원본 바이트 미확보</td>
</tr>
<tr>
<td>현재 Notion 4블록 재조립 6,043B</td>
<td>ef9b0cfe2e764adc896a45451f5607fd751ab0c8b523e6477339994ba758a4b8</td>
<td>후속 CPU 3환경 CTest 1/1 기록 존재</td>
</tr>
</table>
재조립 시 블록 경계·최종 개행의 0\~2개 조합 총 81개를 검사했으나 과거 값은 일치하지 않았다고 현재 색인에 기록되어 있습니다. 이는 과거 소스의 내용이 다르다는 것을 **확정**하지 않으며 단순 개행 차이만으로 증명할 수 없다는 뜻입니다. 과거 파일 실물 또는 불변 백업을 확보하기 전까지 역사적 SHA 불일치를 해결 완료로 처리하지 않습니다.
## 실물 출처
- <mention-page url="https://app.notion.com/p/3fba99703f40810b826cc7eec5b1f039"/>: 13A 4개 분할 소스와 CMake, 역사적 SHA 기록.
- <mention-page url="https://app.notion.com/p/3fda99703f4081d5ae1ac37e1b558bd0"/>: 현행 소스 CPU 재실행·원시 로그 9개.
- <mention-page url="https://app.notion.com/p/3fba99703f4081ef8863c783ed5a6e8f"/>: 오버플로 전제 보완 13B 독립 실습.
## 다음 검사
1. 과거 소스 파일을 식별할 수 있는 첨부·버전 히스토리의 실제 바이트가 있는지 찾아 바이트 길이/SHA-256으로 대조.
2. 찾지 못하면 역사적 동일성은 **영구 미검증**으로 남기고 현재 소스 기준의 실행 검증만 별개 표시.
3. Windows/MSVC·D3D11·GPU/HLSL은 실행하지 않았으므로 미검증.
<details>
<summary>왜 컴파일 성공이 과거 원본의 동일성을 보장하지 않나요?</summary>
	CTest는 현재 전달된 소스가 제한된 조건을 통과했다는 관찰입니다. 과거 파일과 동일한 바이트였는지는 원본의 해시·파일 자체가 있어야 확인할 수 있습니다.
</details>
