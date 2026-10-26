# DirectX 12 01 · DX11에서 명시적 API로 · 장치와 디버그 경계

DX11 immediate context를 호출하던 감각을 버리는 것이 아니라, 드라이버가 대신 숨겨 주던 책임을 목록으로 꺼내는 것이 목표다. 기존 DX11 교재 다음에 읽는다. 새 API로 이름만 치환해도 자동으로 빨라진다는 결론은 내리지 않는다.

## 파일 구성과 실행 경계

- project/: 이 단원의 C++17 CPU 계약 검사. GPU/Windows API를 호출하지 않습니다.
- windows/: 12단원이 공유하는 실제 D3D12 전체 소스. renderer와 compute 두 프로그램입니다.
- verification.json: 실제 CPU 결과. GPU 통과 기록이 아닙니다.

DeviceContext가 debug layer, DXGI factory, 하드웨어 열거 또는 명시적 WARP, D3D12 장치를 생성한다. 고급 기능 조회와 GPU 선호 정책은 미구현이다.

## CPU 빌드·실행

C++17 컴파일러와 CMake 3.16 이상을 준비한 뒤 이 README 폴더에서 실행합니다.

```sh
cmake -S project -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Linux 실행 파일: `./build/dx12_contract`. Visual Studio generator이면 `build/Release/dx12_contract.exe`입니다. Windows CPU 빌드는 이번 환경에서 실행하지 않았습니다.

기대 출력(현재 Linux에서 실제 확인):
```text
PASS DX12-01: 8 CPU contract checks
```

검사 실패는 예외/비정상 종료이며 NDEBUG로 사라지는 assert가 아닙니다. 외부 입력·인터넷·파일 변경을 요구하지 않습니다.

## 실제 Windows D3D12 프로젝트

Windows 10/11 x64, Visual Studio C++ 도구, Windows SDK, CMake 3.20 이상을 준비합니다. Debug 실행에는 Windows Graphics Tools의 D3D12 debug layer가 필요합니다. 설치나 보안 설정 변경을 자동으로 수행하지 않습니다.

```powershell
cmake -S windows -B win-build -A x64
cmake --build win-build --config Debug
.\win-build\Debug\atlas_dx12_renderer.exe
.\win-build\Debug\atlas_dx12_compute.exe
```

WARP를 명시적으로 선택하려면 각 프로그램에 `--warp`를 추가합니다. 기본 실행은 하드웨어만 선택하고 없으면 실패합니다. WARP 성공은 하드웨어 성능 검증이 아닙니다.

renderer 기대 동작: 고정 960×540 창의 어두운 배경 위 8×8 체커 texture 삼각형. compute 기대 출력: `PASS: 64 GPU values (adapter chosen at runtime)`. 이 두 기대 결과는 이번 환경에서 실행·관찰한 결과가 아닙니다.

## Windows 프로그램이 포함하는 것

- D3D12 장치, debug layer, direct queue, 두 backbuffer와 allocator, fence 수명.
- SRV table + root constants + static sampler, graphics PSO, SM5.1 HLSL, 단일 mip texture upload.
- 별도 compute queue → copy queue fence 의존성 → readback 64값 비교.
- HRESULT 실패, device-removed fence 값과 10초 timeout을 성공과 구분합니다.

## 포함하지 않는 것

Resize, DPI/전체화면/HDR/MSAA 정책, mesh/depth renderer, 일반 upload ring, 동적 descriptor allocator, mip 생성, texture 파일 loader, DXC/SM6, Enhanced Barriers, DXR, PIX/DRED 수집, device recovery는 구현하지 않았습니다. CPU 모델과 실제 API의 대응 범위를 교재에서 구분합니다.

## 검증 상태

2026-10-26 g++ (Debian 14.2.0-19) 14.2.0, cmake version 3.31.6, Release CTest 1/1 및 UBSan 통과. ASan/LeakSanitizer 첫 실행은 ptrace 환경 오류로 중단됐고 통과로 기록하지 않았습니다. Windows/MSVC, Windows SDK 선언, HLSL 컴파일, WARP/하드웨어 GPU 실행, debug layer 결과, PIX/DRED와 성능은 모두 미검증입니다. 먼저 Windows 빌드부터 확인한 뒤 화면/수치 검증을 진행해야 합니다.

## 실습

확장 과제: 노트북 GPU 선택 정책은?: EnumAdapterByGpuPreference 등 별도 정책을 설계하고 실제 선택 결과를 기록한다. 현 예제는 첫 적합 하드웨어만 선택한다.

자료의 사용 경계와 실패 상태를 먼저 확인하고, 자원 수명 검사를 지워 오류를 숨기지 마세요.
