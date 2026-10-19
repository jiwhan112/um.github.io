# 게임 수학 · MFC · WinAPI 실습 팩 v1.0

개정일: 2026-10-13. C++20 학습 기준. 회사 프로젝트나 외부 SDK의 원본 코드가 아닌 독립 학습 예제입니다.

## 실행한 범위와 실행하지 않은 범위

Linux x86-64 / GCC 14.2.0 / Clang 17.0.0 / CMake 3.31.6에서 플랫폼 독립 프로그램 6개를 빌드하고 실행했습니다. Clang ASan·UBSan 구성도 통과했습니다. `logs/`에는 실제 명령의 구성·빌드·CTest 로그가 있습니다.

**Windows/MSVC와 windows/winapi_demo.cpp는 컴파일하거나 실행하지 않았습니다.** Windows SDK, MFC, GPU, DirectX, FMOD 실행 환경이 이 검증 환경에 없습니다. GUI 소스는 공식 API 계약을 대조한 정적 검토 결과입니다. 실행 확인 목록은 docs/validation.md에 있습니다. MFC GUI 프로젝트, D3DX 또는 FMOD 바이너리는 포함하지 않습니다.

## 폴더

- include/math.hpp: 벡터, 스칼라 행렬, 기하, 카메라, 타일 좌표.
- include/loop.hpp: 입력 에지, 고정 시간 간격, 애니메이션.
- include/grid.hpp: 맵 파싱, 편집/Undo/Redo, A*.
- include/check.hpp: 정상 조건 검사 도우미.
- src/: 독립 실행 가능한 프로그램 6개.
- windows/: 별도의 Win32 GUI 예제와 DPI manifest.
- docs/validation.md: 실제 결과와 미검증 항목.
- logs/: 이번 실제 실행 로그.
- verify.py: 임시 빌드 디렉터리에서 같은 테스트를 재실행하는 도구.

## 빌드

압축을 푼 이 폴더에서:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Clang 검사기 사용:

```shell
cmake -S . -B build-san -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build-san --config Debug
ctest --test-dir build-san -C Debug --output-on-failure
```

선택 사항: Python 3으로 별도의 임시 빌드/로그 만들기.

```shell
python verify.py --compiler g++
python verify.py --compiler clang++ --sanitizers
```

Windows에서는 MSVC C++ 도구와 Windows SDK를 준비하고 x64 개발자 터미널 또는 Visual Studio CMake 기능을 사용합니다. `cmake -S . -B build`와 `cmake --build build --config Debug`로 구성합니다. 자동 선택된 생성기에 따라 실행 파일은 `build/Debug/winapi_demo.exe` 또는 `build/winapi_demo.exe`에 있습니다. 다른 생성기로 이미 구성한 build 폴더는 재사용하지 마세요.

Win32 예제는 Windows 10 version 1703 이상을 대상으로 작성했습니다. PMv2 manifest를 app.rc에 포함하고 MSVC의 자동 manifest 생성을 `/MANIFEST:NO`로 끕니다. 폰트의 정교한 DPI별 재구성은 구현하지 않았습니다.

## GUI 조작과 설계

방향키: 이동. 클릭: 목표를 향해 이동. Space: 위치 초기화. Esc: 종료.
120Hz는 고정 Update 간격이지 화면 주사율 보장이 아닙니다. 한 루프에서 최대 8회, 프레임 경과 입력 최대 0.25초로 제한합니다. 밀린 시간을 일부 버리는 로컬 데모 정책이며 네트워크 결정적 시뮬레이션을 구현한 것이 아닙니다. 최소화·비활성·크기 조정 중에는 정지하고 복귀 시 시간 기준을 재설정합니다.
App 좌표는 x 오른쪽/y 아래쪽의 논리 단위이며 DPI/96으로 출력합니다. math.hpp의 Camera2D는 별도의 x 오른쪽/y 위쪽 월드를 화면으로 변환합니다. 두 규약을 무심코 혼용하지 마세요.

## 수학 및 데이터 계약

행 벡터 p*M, 행별 저장의 4x4 행렬을 사용합니다. Transform은 w를 포함한 4성분 원시 결과이며 자동 원근 나눗셈을 하지 않습니다. 행렬/벡터 기본 산술은 중간값이 유한 범위에 들어가는 보통 크기의 값에 대한 학습용이며 모든 함수가 모든 입력의 범위를 검사하지는 않습니다. 정규화 등 검사 함수도 전체 IEEE754 입력 공간에서 수치 안정성을 증명하는 것은 아닙니다.

역행렬은 부분 피벗 Gauss–Jordan과 절대 임계값 1e-12를 사용합니다. 조건수·스케일 적응형 판정을 포함한 범용 수치 라이브러리가 아닙니다. AABB/원은 접촉도 충돌로 취급합니다. 마름모 타일 소속은 중심 주변 반열린 구간입니다. 카메라/타일 투영의 y축 규약은 헤더 주석과 문서를 함께 읽습니다.

맵 포맷: `MAP1 width height` 뒤 정확히 width*height개의 정수. 폭/높이 1..256, ID 0..4095, 텍스트 4MiB 이하. 행 구분은 저장 출력 형식이며 파서는 공백 구분 토큰을 읽습니다. 임시 모델에 완전히 읽고 검증한 뒤 교체합니다. 파일 시스템 원자적 저장이나 전원 장애 복구까지 제공하지 않습니다.

A*: 0은 통과 가능, 다른 ID는 막힘. 네 방향 이동 비용1, Manhattan 휴리스틱. 반환 경로는 시작과 목표를 포함합니다. 가중치·대각선·움직이는 장애물은 다루지 않습니다.

## 학습 노트

- 게임 수학·MFC: https://app.notion.com/p/3bde90ebffa44ef0a6455ca8e68a7886
- WinAPI 게임 프로그래밍: https://app.notion.com/p/fffca9a4ce864ac7bade7efe90186be7

## 참고 자료

계산 구현과 테스트는 이 실습용으로 작성했습니다. 아래 자료는 API 계약과 알고리즘 설명을 대조한 근거입니다.

- DirectXMath 규약: https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-getting-started
- Win32 창: https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
- QPC: https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
- GDI bitmap: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatiblebitmap
- DPI: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged
- MFC: https://learn.microsoft.com/en-us/cpp/mfc/document-view-architecture
- A*: https://inst.eecs.berkeley.edu/~cs188/textbook/search/informed.html
- 시간 단계: https://gafferongames.com/post/fix_your_timestep/
