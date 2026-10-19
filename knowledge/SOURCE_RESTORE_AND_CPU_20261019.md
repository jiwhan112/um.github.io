# 공유 실습 원본 보존 및 GitHub CPU 재검증 · 2026-10-19

## 복원된 원본

**GitHub Pages의 독립 원본은 이제 Notion 파일 다운로드가 아니라 GitHub 저장소입니다.** 다음 두 사용자 라이브러리 파일의 원시 텍스트를 재조회하고 상대 경로별로 코드 블록을 복원했습니다.

| 원본 문서 | 보관 폴더 | 정확한 전체 텍스트 파일 수 | 무결성 |
|---|---|---:|---|
| `game_math_winapi_sources_v1.md` · 59,785B | [게임 수학·WinAPI 소스](shared/game-math-winapi-lab/) | 17개 실행·설정·지원 파일 + 원본 해시 원장 | SHA-256 17/17 일치; Git blob 17/17 일치 |
| `dx9_pipeline_sources_v1.md` · 37,379B | [DX9 CPU 파이프라인 소스](shared/dx9-pipeline-lab/) | 12개 전체 텍스트 파일 | Git blob 12/12 일치 |

일치 검증은 원본 Markdown의 파일별 외곽 코드 펜스에서 텍스트를 재구성하고, 원본 SHA-256 목록(게임 수학·WinAPI) 및 복구한 파일의 Git blob SHA-1과 실제 업로드된 GitHub blob SHA-1을 비교한 결과입니다. 원본 파일 목록은 `knowledge/shared/`에 같이 보관했습니다.

**원본 ZIP 바이너리 2개, Notion 파일 블록 첨부 바이트와 과거 실행 로그 ZIP은 확보하지 못했습니다.** PDF나 이미지를 추측해 대체하지 않았고, 원본 ZIP 다운로드·해시 검증 완료로 표시하지 않습니다.

## 새 GitHub CI 실행 (과거 Notion 기록과 독립)

- [GitHub Actions #39213029058](https://github.com/jiwhan112/um.github.io/actions/runs/39213029058) — 4개 환경 Job 성공
- 게임 수학·WinAPI 공통 CPU 실습: GCC 14.2 C++20 CTest **6/6**, Clang 17 C++20 ASan/UBSan CTest **6/6**.
- DX9 파이프라인 CPU 실습: GCC 14.2 C++20 CTest **5/5**, Clang 17 C++20 ASan/UBSan CTest **5/5**.
- 같은 6개/5개 프로그램을 두 환경에서 다시 실행한 것이며 독립 테스트 12개/10개로 중복 집계하지 않았습니다.
- 새 빌드의 원시 GitHub Actions 작업 로그를 텍스트로 [verification/shared/](verification/shared/)에 영구 보관했습니다. CMake configure·build·CTest 콘솔의 실제 출력이며 원본 Notion 로그가 아닙니다.

### 실제 실행 명령

```bash
cmake -S knowledge/shared/game-math-winapi-lab -B /tmp/math-gcc -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DENABLE_SANITIZERS=OFF
cmake --build /tmp/math-gcc --parallel 2
ctest --test-dir /tmp/math-gcc -V --output-on-failure
cmake -S knowledge/shared/dx9-pipeline-lab -B /tmp/dx9-clang-san -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DENABLE_SANITIZERS=ON
cmake --build /tmp/dx9-clang-san --parallel 2
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir /tmp/dx9-clang-san -V --output-on-failure
```

CI의 실제 디렉터리 이름과 빌드 플래그 등 완전한 기록은 `.github/workflows/knowledge-shared-labs-check.yml` 및 원시 로그를 기준으로 합니다. 이 블록은 사용자가 직접 재현하기 편한 독립 예시이며 CI 출력 원문을 가장한 복사본이 아닙니다.

## 플랫폼 미검증과 다음 감사

Win32 데모 `windows/winapi_demo.cpp`, Windows SDK/MSVC·D3D9·D3D11·GPU·MFC는 이 Linux 테스트에서 **컴파일·실행되지 않았습니다**. HLSL 참고 소스도 GPU·FXC로 컴파일하지 않았습니다. 현재 공통 소스가 있다는 사실만으로 플랫폼 검증까지 완료된 것은 아닙니다.

원본 ZIP 및 Notion 첨부 바이트의 회수·해시 대조, DX9의 후속 구현 단원과 DX11 전체 이관, 이미지 감사가 아직 남아 있습니다. 기존 Notion 원본과 타 예약 작업은 수정하지 않았습니다.
