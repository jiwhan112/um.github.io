# ATLAS NOTE — 프로그래밍 지식 라이브러리

- [장기 학습 목차·집필 설계 v2](CURRICULUM.md): 최종적으로 새로 공부하고 작성할 과정에 대한 설계. **기존 Notion 이관 계획과 구분**.
- [실제 공개 문서 원장](data/catalog.json): 독립된 학습 문서의 경로·Notion 출처·분류.
- [WinAPI/게임 수학 첫 이관 감사](MIGRATION_WINAPI_MATH_20261019.md): 원본·표·접기·Mermaid·소스 복원·미이관 자산 상태.
- [DX9 렌더링 파이프라인 01–05 이관 감사](MIGRATION_DX9_PIPELINE_20261019.md): 클립·래스터·깊이·블렌딩 문서 보존 상태.
- [DX9 구현 01–12 본문 이관 원장](MIGRATION_DX9_IMPLEMENTATION_20261019.md): 장치·COM·DLL·버퍼/Reset·Scene·카메라·이펙트·셰이더/Frustum·통합 프로젝트.
- [DX9 원본 CPU 소스 8종·21개 실제 CI 실행 로그](DX9_IMPLEMENTATION_CPU_VERIFICATION_20261019.md): GCC C++11/20 및 Clang ASan/UBSan; 플랫폼 검증 한계 포함.
- [DX9 구현 01–12 이관 원장](MIGRATION_DX9_IMPLEMENTATION_20261019.md): 개정 03/04 섹션·원본 하위 페이지·미회수 첨부 등 남은 검증.

## 원본과 파생물

2026-10-19 초기 프로토타입 3개(`dx11-frustum`, `algorithm-lis`, `python-html`)는 원본과 공개 URL을 그대로 유지했습니다.

후속 작업에서 WinAPI 01–04와 CPU 실습 04A, 게임 수학 01–05 **10개**, DX9 렌더링 파이프라인 01–05 **5개**, 합계 **기존 Notion 본문 15개**를 이관했습니다. 그 뒤 DX9 구현 본문 **01~12의 12개 단원**과 01A·02A·03A·04A·05A·06A·07B·11A의 **전체 소스 문서 8개**를 추가했습니다. 두 공통 실습 전체 소스 페이지와 초기 프로토타입 3개를 모두 포함해 GitHub 학습 문서는 **40개**입니다. 모든 단원 디렉터리에는 다음 파일을 둡니다.

- `source-notion.md`: 원래 Notion enhanced Markdown의 **원본 본문**
- `content.json`: React Typed Blocks(문단·코드·Mermaid 원본·표·접힌 문답)로 변환한 화면용 문서
- WinAPI04A 전용 `main.cpp` 및 `CMakeLists.txt`: 원본 네이티브 소스에서 바이트·FNV64를 대조해 복원

DX9 구현의 전체 소스는 `knowledge/shared/dx9-impl-*/`에 독립 보관했고, CMake/CTest 회귀 검증 결과는 별도의 `knowledge/verification/dx9/` 원장에 누적합니다. Notion 원본에 있던 첨부 파일의 실제 다운로드·해시 검증은 별도로 처리해야 합니다.

추가 학습 문서가 검증되면 `knowledge/data/catalog.json`에 명시적으로 추가합니다. 빌드가 내부 문서 색인과 직접 접근 URL을 생성하며, 기존 언어·브리핑·사고력 자료와 Supabase/localStorage 키는 변경하지 않습니다.

## DX11 01–08 전체 소스 복구 및 본문 보강

- [DX11 01–08 전체 소스 복구 원장](shared/DX11_SOURCE_LEDGER_20261019.md) 및 [배치 감사](MIGRATION_DX11_01_08_20261019.md): Notion 01A–08A의 네이티브 코드와 CMake·Win32 예제·HLSL, 과거 원시 CTest를 GitHub에 보존했습니다.
- 기존 02–06 본문은 원본을 유지하며 GitHub 전체 소스 링크를 추가했고, 신규 01·07·08 본문도 독립 이관했습니다. DX11 01의 원본 파일 블록은 signed URL만 확인되어 첨부 바이너리 자체는 미검증입니다.
- **Windows/MSVC/D3D11/GPU/HLSL 실기기 미검증.** 신규 Linux CPU CTest 결과와 과거 Notion 원시 로그는 별도 구분합니다.
- React 학습 문서는 53개 (기존 45개 + DX11 본문 3개 + 전체 소스 페이지 5개)이며 현대 C++·DX12·엔진·Python 신규 학습과는 무관합니다.

## DX11 02–06 본문 이관 배치

- [DX11 원문·하위 소스 잔여 감사](MIGRATION_DX11_PHASE1_20261019.md): 02–06 다섯 본문의 텍스트·코드·Mermaid·표·접기 구조를 GitHub에 저장했습니다. 원본 01에는 미확인 첨부가 있어 보류했습니다.
- **기존 DX11 02–06의 하위 CPU/Windows 소스와 CMake·검증 로그는 아직 GitHub에 독립 이관하지 않았습니다.** 링크만 보존한 상태로 본문과 소스 이관 진행률을 구분합니다.

## 현재 한계

- [공통 실습 소스·CI 무결성 원장](SOURCE_RESTORE_AND_CPU_20261019.md): 개인 라이브러리의 파일별 원본에서 WinAPI·게임 수학 17개와 DX9 파이프라인 12개, 총 29개 원본 소스·지원 파일을 복원했습니다. **원본 ZIP 바이너리와 Notion 첨부의 원시 파일 바이트는 확보하지 못했습니다.**
- `knowledge/shared/game-math-winapi-lab/`에 Win32 데모·manifest·CMake·헤더·6개 CPU 실습 전체 텍스트를, `knowledge/shared/dx9-pipeline-lab/`에 5개 CPU 실습·HLSL 참고 예제·CMake 등 전체 텍스트를 저장했습니다. Win32 GUI는 소스 복원만 수행했으며 실제 Windows/MSVC·GUI 실행은 미검증입니다.
- DX9 구현의 8개 CPU 실습 소스·헤더·CMake는 `knowledge/shared/dx9-impl-*/`에 파일 단위로 보관하고, **21개 CMake/CTest Linux 구성**에서 모두 통과했습니다. **Windows GUI·Direct3D9 장치/Reset 실기기 경로, 부록 notion_example.cpp와 이미지·첨부 바이너리 일부는 미검증/미이관입니다.**
- WinAPI04A의 순수 CPU 소스는 바이트 대조 보관했고 GCC C++20·C++11·Clang ASan/UBSan CTest 각각 1/1 통과했습니다. 두 공통 실습의 GitHub CPU CI 또한 GCC/Clang ASan·UBSan에서 게임 수학 6/6, DX9 파이프라인 5/5 통과했으며 새 CMake·CTest 원시 로그 4개를 GitHub에 보존했습니다.
- Windows/MSVC·D3D9/11·GPU·Unity 테스트는 이번 저장/CI로 검증된 것이 아닙니다. Mermaid 및 수식은 원본 코드를 보존하며 시각적 렌더링은 후속 UI 개선 대상입니다.
- 이관 전 Notion 원본 문서, 기존 시범 자료, 다른 자동 발행 작업을 수정하지 않습니다. 이 사이트는 공개 GitHub Pages입니다.
