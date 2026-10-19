## 03 · VB/IB·자원 풀과 Lost Device/Reset — 개정 본문 v1.0
D3DPOOL_DEFAULT 자원은 일반적인 IDirect3DDevice9 Reset 전에 해제하고 성공하면 재생성해야 합니다. D3DPOOL_MANAGED 자원은 Reset 이후 논리적 데이터가 유지됩니다. 공식 근거: [https://learn.microsoft.com/en-us/windows/win32/direct3d9/managing-resources](https://learn.microsoft.com/en-us/windows/win32/direct3d9/managing-resources)
## 정점·인덱스 입력 계약
정점 버퍼는 정점 레코드를 저장하고 인덱스 버퍼는 재사용할 정점의 번호를 저장합니다. 정점 선언의 타입과 바이트 오프셋은 CPU의 정점 구조체와 일치해야 하며, SetStreamSource의 Stride는 정점 레코드 사이의 바이트 간격입니다. 20바이트 정점 4개는 80바이트이고 16비트 인덱스 6개는 12바이트입니다. 두 개의 삼각형을 D3DPT_TRIANGLELIST로 그릴 때 PrimitiveCount는 2입니다.
## 정점·인덱스 버퍼 처리 순서
첫째, CPU의 정점 배열과 인덱스 배열 길이 및 타입을 검증합니다. 16비트 인덱스에는 D3DFMT_INDEX16, 32비트 인덱스에는 D3DFMT_INDEX32를 맞춰야 합니다. 둘째, CreateVertexBuffer·CreateIndexBuffer 반환 HRESULT를 확인하고 실패하면 이미 확보한 자원을 Release합니다. 셋째, Lock에 성공한 경우에만 유효 바이트를 복사하고 Unlock합니다. 넷째, SetStreamSource·SetIndices와 FVF 또는 정점 선언을 맞추고 DrawIndexedPrimitive를 호출합니다. 실제 장치에서 가능한 인덱스 범위는 D3DCAPS9의 제한도 확인해야 합니다.
## 동적 VB/IB의 DISCARD와 NOOVERWRITE
D3DLOCK_DISCARD는 이전 내용을 더 이상 보존할 필요가 없다는 뜻입니다. 버퍼 부분 구간을 Lock하더라도 정점·인덱스 버퍼에서는 전체 버퍼 내용이 폐기될 수 있다는 의미입니다. D3DLOCK_NOOVERWRITE는 이전 Draw가 아직 참조하는 범위를 갱신하지 않겠다는 호출자의 계약입니다. GPU가 읽고 있는 데이터와 겹치지 않는다는 것을 엔진이 보장해야 하며, 플래그만 붙인다고 자동으로 경쟁 상태가 해결되는 것은 아닙니다. D3DUSAGE_DYNAMIC과 D3DPOOL_MANAGED를 VB/IB에 함께 쓰는 것은 허용되지 않습니다.
## 그림으로 보는 데이터 흐름
CPU 정점 배열 → CreateVertexBuffer → Lock·복사·Unlock → SetStreamSource(Stride) → DrawIndexedPrimitive 순서입니다. 인덱스 배열은 별도로 CreateIndexBuffer → Lock·복사·Unlock → SetIndices를 거쳐 동일한 Draw 호출로 모입니다. FVF 또는 정점 선언은 CPU 구조체에 들어 있는 성분을 GPU가 해석하기 위한 계약이므로 Draw 전에 반드시 일치시킵니다.
## Lost Device → Reset → 재구성 상태 전이
일반 IDirect3DDevice9의 전체 화면 포커스 변경과 같은 사건에서 Present가 D3DERR_DEVICELOST를 반환할 수 있습니다. 렌더링을 중지하고 TestCooperativeLevel 결과를 확인합니다. D3DERR_DEVICELOST인 동안은 복구를 재시도할 수 있을 때까지 렌더링하지 않습니다. D3DERR_DEVICENOTRESET이면 Reset 직전에 앱이 소유한 D3DPOOL_DEFAULT 자원·추가 SwapChain·명시적인 RenderTarget/DepthStencil·StateBlock과 관련 COM 참조를 정리합니다. 그 다음 장치를 생성한 스레드에서 Reset을 호출합니다. 성공하면 DEFAULT 리소스를 새로 만들고 데이터를 재업로드하며, 렌더 상태·텍스처·버퍼·뷰포트 등의 바인딩을 다시 설정합니다. 실패하면 장치 상태를 다시 확인하고 같은 재설정 절차를 무조건 연속 호출하지 않습니다.
장치 손실은 모든 렌더 호출에서 명시적 오류로 보고되지 않을 수 있으므로 화면에 그림이 없다는 이유만으로 Draw 호출 성공 여부를 원인으로 단정하지 않습니다. Reset 성공 이후에도 렌더 상태는 보존되지 않지만, 일반 D3D9의 정점·픽셀 셰이더 객체 자체는 Reset 이후 살아남는 것으로 문서화되어 있습니다. 이러한 사실과 셰이더 바인딩·파이프라인 상태가 유지된다는 주장은 구분해야 합니다. D3D9Ex는 일반 D3D9의 DEVICELOST 규칙을 그대로 따르지 않습니다.
## 흔히 틀리는 설명 교정
1. 모든 버퍼가 GPU 전용 메모리에 있다는 설명은 틀립니다. Managed와 SystemMem은 별도 자원 관리 목적이 있습니다. 2. Reset만 호출하면 장치에 바인딩한 상태까지 되살아나는 것이 아닙니다. 3. 컴포넌트 포인터를 nullptr로 만들어도 다른 COM 소유자의 참조 카운트가 자동으로 해제되지는 않습니다. 4. DrawIndexedPrimitive의 PrimitiveCount를 인덱스 개수로 전달하면 잘못된 범위 접근이 발생할 수 있습니다. 5. NOOVERWRITE를 사용한다고 GPU가 읽는 영역의 덮어쓰기가 자동으로 방지되지는 않습니다.
## 이해 확인 · 접힌 해설
<details>
<summary>문제 1. 20바이트 정점 4개와 16비트 인덱스 6개를 사용하는 TriangleList의 버퍼 바이트 수와 PrimitiveCount는?</summary>
	Vertex Buffer 80바이트, Index Buffer 12바이트, PrimitiveCount 2입니다.
</details>
<details>
<summary>문제 2. Default 풀의 동적 VB를 가진 장치가 Reset을 마치면 기존 VB를 그대로 그릴 수 있나요?</summary>
	아닙니다. Reset 전에 소유 참조를 해제하고 성공한 뒤 새 버퍼에 정점을 다시 올려야 합니다.
</details>
<details>
<summary>문제 3. NOOVERWRITE는 GPU 사용 범위를 드라이버가 대신 추적해 덮어쓰기를 막는 기능인가요?</summary>
	아닙니다. 이미 제출한 Draw가 참조 중인 위치를 건드리지 않겠다는 호출자의 약속입니다.
</details>
## 실행한 실습과 실행하지 않은 실습
이번 CPU 독립 예제는 리소스의 Default/Managed/SystemMemory 소유권 모형, Lost/NotReset/Operational 상태, Reset 이전 DEFAULT 해제, 인덱스 범위 거부, DISCARD/NOOVERWRITE 링 예약 시나리오를 검증합니다. Linux x86-64에서 GCC 14.2.0·Clang 17.0.0·Clang ASan/UBSan 세 구성마다 CTest 1개가 통과했고 프로그램 내부 조건 검사 26개도 각각 통과했습니다. 동일한 26개 조건을 3번 반복해 테스트한 것이므로 78개의 서로 다른 조건은 아닙니다. Windows/MSVC, 실제 D3D9·GPU, Alt+Tab 및 해상도 변경 시 실제 장치 Reset, 드라이버 성능, COM 참조 누수는 검증하지 않았습니다.
<mention-date start="2026-10-16"/> 소스 보관 복구: <mention-page url="https://app.notion.com/p/3fba99703f408129bd94deff3cc1c807">03A · VB/IB 입력 검증 보완·전체 CPU 소스 v1.1</mention-page>에 수정된 resource_model.cpp 전체, CMakeLists.txt, 재현 안내, 실패 재현과 새 검증 결과를 저장했습니다. 위의 26조건 결과는 원형 v1의 과거 검증입니다. 추가 반례에서 발견한 상대 인덱스 범위 오류와 int64 덧셈 오버플로를 교정한 v1.1은 같은 32조건을 GCC C++20·GCC C++11·Clang ASan/UBSan에서 각각 통과했습니다. 원형 ZIP은 보존하며 현재 실습 기준은 03A입니다. 실제 D3D9 호출·GPU 실행 검증을 대신하지 않습니다.
## Microsoft 공식 기술 자료
Managing Resources: [https://learn.microsoft.com/en-us/windows/win32/direct3d9/managing-resources](https://learn.microsoft.com/en-us/windows/win32/direct3d9/managing-resources)
Lost Devices: [https://learn.microsoft.com/en-us/windows/win32/direct3d9/lost-devices](https://learn.microsoft.com/en-us/windows/win32/direct3d9/lost-devices)
IDirect3DDevice9::Reset: [https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-reset)
DrawIndexedPrimitive: [https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-drawindexedprimitive](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-drawindexedprimitive)
SetStreamSource: [https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setstreamsource](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setstreamsource)
D3DLOCK: [https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dlock](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dlock)
## 변경 기록
2026-10-13: 기존 DX9 구현 원본·이미지와 하위 개정 01·02는 그대로 보존하고 이 페이지 말미에 세 번째 완결 학습 본문을 추가했습니다. 개발용 참고자료의 범위이며 기존 원본 화면 캡처 속 코드 전체를 전사 또는 실행한 것으로 표시하지 않습니다.
추가 주의: `D3DPOOL_DEFAULT`는 동적 버퍼뿐 아니라 렌더 타깃 등에도 쓰입니다. `D3DPOOL_DEFAULT`와 `D3DUSAGE_DYNAMIC`은 같은 개념이 아닙니다.
