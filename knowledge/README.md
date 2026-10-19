# ATLAS NOTE — 프로그래밍 지식 라이브러리

- [장기 학습 목차·집필 설계 v2](CURRICULUM.md): 최종적으로 새로 공부하고 작성할 과정에 대한 설계. **기존 Notion 이관 계획과 구분**.
- [실제 공개 문서 원장](data/catalog.json): 독립된 학습 문서의 경로·Notion 출처·분류.
- [WinAPI/게임 수학 첫 이관 감사](MIGRATION_WINAPI_MATH_20261019.md): 원본·표·접기·Mermaid·소스 복원·미이관 자산 상태.
- [DX9 렌더링 파이프라인 01–05 이관 감사](MIGRATION_DX9_PIPELINE_20261019.md): 클립·래스터·깊이·블렌딩 문서 보존 상태.

## 원본과 파생물

2026-10-19 초기 프로토타입 3개(`dx11-frustum`, `algorithm-lis`, `python-html`)는 원본과 공개 URL을 그대로 유지했습니다.

후속 작업에서 WinAPI 01–04와 CPU 실습 04A, 게임 수학 01–05 **10개**, DX9 렌더링 파이프라인 01–05 **5개**, 합계 **기존 Notion 본문 15개**를 이관했습니다. 여기에 두 공통 실습 **전체 소스 탐색 문서**와 초기 프로토타입 3개를 더해, GitHub 학습 문서는 **20개**입니다. 모든 단원 디렉터리에는 다음 파일을 둡니다.

- `source-notion.md`: 원래 Notion enhanced Markdown의 **원본 본문**
- `content.json`: React Typed Blocks(문단·코드·Mermaid 원본·표·접힌 문답)로 변환한 화면용 문서
- WinAPI04A 전용 `main.cpp` 및 `CMakeLists.txt`: 원본 네이티브 소스에서 바이트·FNV64를 대조해 복원

추가 학습 문서가 검증되면 `knowledge/data/catalog.json`에 명시적으로 추가합니다. 빌드가 내부 문서 색인과 직접 접근 URL을 생성하며, 기존 언어·브리핑·사고력 자료와 Supabase/localStorage 키는 변경하지 않습니다.

## 현재 한계

- [공통 실습 소스·CI 무결성 원장](SOURCE_RESTORE_AND_CPU_20261019.md): 개인 라이브러리의 파일별 원본에서 WinAPI·게임 수학 17개와 DX9 파이프라인 12개, 총 29개 원본 소스·지원 파일을 복원했습니다. **원본 ZIP 바이너리와 Notion 첨부의 원시 파일 바이트는 확보하지 못했습니다.**
- `knowledge/shared/game-math-winapi-lab/`에 Win32 데모·manifest·CMake·헤더·6개 CPU 실습 전체 텍스트를, `knowledge/shared/dx9-pipeline-lab/`에 5개 CPU 실습·HLSL 참고 예제·CMake 등 전체 텍스트를 저장했습니다. Win32 GUI는 소스 복원만 수행했으며 실제 Windows/MSVC·GUI 실행은 미검증입니다.
- WinAPI04A의 순수 CPU 소스는 바이트 대조 보관했고 GCC C++20·C++11·Clang ASan/UBSan CTest 각각 1/1 통과했습니다. 두 공통 실습의 GitHub CPU CI 또한 GCC/Clang ASan·UBSan에서 게임 수학 6/6, DX9 파이프라인 5/5 통과했으며 새 CMake·CTest 원시 로그 4개를 GitHub에 보존했습니다.
- Windows/MSVC·D3D9/11·GPU·Unity 테스트는 이번 저장/CI로 검증된 것이 아닙니다. Mermaid 및 수식은 원본 코드를 보존하며 시각적 렌더링은 후속 UI 개선 대상입니다.
- 이관 전 Notion 원본 문서, 기존 시범 자료, 다른 자동 발행 작업을 수정하지 않습니다. 이 사이트는 공개 GitHub Pages입니다.
