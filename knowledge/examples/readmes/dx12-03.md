# DirectX 12 03 · Swap Chain·Present · 프레임 슬롯의 수명

화면에 보여줄 버퍼, CPU 기록 슬롯, GPU 완료 값의 연결을 표로 그린다. fixed-size flip-model swapchain의 실제 렌더 경로를 읽고 resize가 추가되면 바뀌는 수명 경계를 설명한다.

## 파일 구성과 실행 경계

- project/: 이 단원의 C++17 CPU 계약 검사. GPU/Windows API를 호출하지 않습니다.
- windows/: 12단원이 공유하는 실제 D3D12 전체 소스. renderer와 compute 두 프로그램입니다.
- verification.json: 실제 CPU 결과. GPU 통과 기록이 아닙니다.

CreateSwapChainForHwnd, GetBuffer, GetCurrentBackBufferIndex, RTV, Present, 슬롯별 fence를 실제 renderer에서 사용한다. Resize/HDR/MSAA는 미구현이다.

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
PASS DX12-03: 10 CPU contract checks
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

확장 과제: 세 슬롯 표 만들기: 버퍼 인덱스와 각 lastUse를 나누고, 다음으로 선택된 슬롯만 기다리는 순서를 작성한다.

자료의 사용 경계와 실패 상태를 먼저 확인하고, 자원 수명 검사를 지워 오류를 숨기지 마세요.
