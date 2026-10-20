<callout icon="📦" color="gray_bg">
	**18A · Deferred MRT CPU 계약 / 원시 코드·테스트 색인**
	상위 본문: <mention-page url="https://app.notion.com/p/3fba99703f4081539f63e9fab7f9bc18">18 · Deferred Rendering — MRT·GBuffer·RTV/SRV 수명·Hazard</mention-page>. 동일한 39개 CPU 조건이 GCC14.2 C++20·C++11 및 Clang17 C++20 ASan/UBSan에서 통과했고, 배치 각 환경 CTest 2/2입니다. Windows/D3D11/GPU 미실행.
</callout>
## 소스 구조·재현
루트 CMakeLists.txt에서 `18_deferred/main.cpp`를 별도 `dx11_18_deferred` 실행 파일로 빌드합니다. CPU 모델은 Texture와 View 범위, MRT/DSV 호환성, SRV·RTV 충돌, Resize generation 및 명시적 Pass 전환을 검사합니다. 전체 코드와 원시 CTest 로그는 이 실행의 `DX11_17_18_CPU_Source_CMake_CTest.zip` 및 `DX11_17_18_FULL_SOURCES_LOGS.md`로 생성됐으나 **Notion 본문에 전체 바이트를 아직 보관하지 못했습니다**. 이후 실행에서 18A의 네이티브 소스 보관과 SHA 비교가 필요합니다.
## 빌드 결과
GCC14.2 C++20: PASS 39, CTest 2/2 (배치). GCC14.2 C++11: PASS 39, CTest 2/2. Clang17 C++20 ASan/UBSan: PASS 39, CTest 2/2. 미실행 환경은 CPU 검사 통과로 대체할 수 없습니다.
## 소스 해시
`18_deferred/main.cpp`: `1a2af872bce89a195dbb9ccb364ee36e91064264435f700eedd7b8b4c4b6225f`.
루트 CMakeLists.txt: `e391797c6268c0ec3a6ea954d8cacd3d93df175063258ef46bb0ae2e1f0df8c1`.
## 공용 Root CMakeLists.txt 전체
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_17_18_cpu_contracts LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "11 or 20")
if(NOT LAB_CXX_STANDARD MATCHES "^(11|20)$")
  message(FATAL_ERROR "LAB_CXX_STANDARD must be 11 or 20")
endif()
set(CMAKE_CXX_STANDARD ${LAB_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()
add_executable(dx11_17_font 17_font/main.cpp)
add_executable(dx11_18_deferred 18_deferred/main.cpp)
if(MSVC)
  target_compile_options(dx11_17_font PRIVATE /W4)
  target_compile_options(dx11_18_deferred PRIVATE /W4)
else()
  target_compile_options(dx11_17_font PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(dx11_18_deferred PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME dx11_17_font_cpu COMMAND dx11_17_font)
add_test(NAME dx11_18_deferred_cpu COMMAND dx11_18_deferred)
```
## 전체 CPU 소스 v2
<mention-page url="https://app.notion.com/p/3fba99703f4081c68669c3d4a2d5eb89"/>
41개 검사 조건, GCC C++20/C++11 및 Clang ASan/UBSan 검증. 기존 39조건 실습과 별개 버전입니다.
