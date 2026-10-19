<callout icon="📦" color="gray_bg">
	**수동 DX11 개정 소스 지속 보관**
	<mention-date start="2026-10-16"/>. 전체 소스·CMake·원시 GCC/Clang CTest 결과. Windows D3D11·GPU·FXC/DXC 실행 검증은 수행하지 않았습니다. 아래는 재현 가능한 CPU 모델 기준이며 오류 수정 전 시행착오와 수정 내용을 구분합니다.
</callout>
Standalone source source-of-truth. CPU-only tests, except uncompiled Windows and HLSL listed explicitly.
Save each code block to the filename in the heading. The root CMakeLists.txt registers four units; individual CMakeLists.txt are subdir fragments.
## Changes and test limitations
# DX11 05–08 CPU source validation
- Date: 2026-10-16 (KST); environment Linux x86-64, GCC 14.2.0, Clang 17.0.0, CMake 3.31.6.
- Default standalone learning: C++20; GCC C++11 compatibility explicitly compiled.
- These tests validate only the CPU contracts. Windows/MSVC, actual D3D11 GPU, HLSL FXC/DXC were NOT COMPILED OR RUN.
- 05 Windows source and HLSL are provided uncompiled with API static review; no claim of GPU success.
- Tests per program: 05=29, 06=30, 07=28, 08=33. The 120 checks are distinct within these four programs. Each configuration reruns the same 120, not 360 independent cases.
- Pre-fix GCC build failed on 06 due to `-Werror=unused-variable`; removed the unused local variable.
- Pre-fix CPU test 08 aborted because the constructor moved `h` into member `height` and then checked the moved-from parameter `h.size()`; fixed to check `height.size()` after move.
- All three complete configurations were rebuilt and CTest 4/4 passed after fixes.
```bash
cmake -S . -B build-gcc20 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DLAB_CXX_STANDARD=20
cmake --build build-gcc20 -j 4
ctest --test-dir build-gcc20 --output-on-failure -V
cmake -S . -B build-gcc11 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DLAB_CXX_STANDARD=11
cmake --build build-gcc11 -j 4
ctest --test-dir build-gcc11 --output-on-failure -V
cmake -S . -B build-clangsan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DLAB_CXX_STANDARD=20 '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer' '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'
cmake --build build-clangsan -j 4
ctest --test-dir build-clangsan --output-on-failure -V
```
## Results
<table header-row="true">
<tr>
<td>Configuration</td>
<td>Result</td>
<td>Internal conditions</td>
</tr>
<tr>
<td>GCC 14.2 C++20 Debug</td>
<td>CTest 4/4 passed</td>
<td>29+30+28+33</td>
</tr>
<tr>
<td>GCC 14.2 C++11 Debug</td>
<td>CTest 4/4 passed</td>
<td>Same 120 conditions</td>
</tr>
<tr>
<td>Clang 17 C++20 ASan/UBSan Debug</td>
<td>CTest 4/4 passed; no sanitizer diagnostics</td>
<td>Same 120 conditions</td>
</tr>
</table>
## Windows manual checklist (not executed)
1. Windows 10/11 with Visual Studio 2022, Windows SDK, d3d11.lib, dxgi.lib, d3dcompiler.lib and CMake.
2. Configure with `cmake -S . -B build -A x64`, build Debug, run CTest, then run `build\05_textured_rect\Debug\dx11_05_windows.exe` (actual location depends on generator).
3. Verify checker quad, aspect behavior, ResizeBuffers on resize, minimized window handling, HRESULT/debug layer messages, no COM leaks.
4. Compile quad.hlsl and skybox_lambert.hlsl using actual FXC/DXC as supported; HLSL compilation has not been performed here.
## CMakeLists.txt
```plain text
add_executable(dx11_07_cpu main.cpp)
set_target_properties(dx11_07_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_07_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_07_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME dx11_07_cpu COMMAND dx11_07_cpu)

```
## main.cpp
```c++
// CPU-only skybox/directional light + render pass state teaching model; not GPU D3D11.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
struct V {double x,y,z;};
V operator-(V a,V b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V operator*(V a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;}
double len(V a){return std::sqrt(dot(a,a));}
V unit(V v){double l=len(v);if(!(l>1e-12)||!std::isfinite(l))throw std::invalid_argument("invalid direction");return v*(1/l);}
bool finite(V v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
struct RGB{double r,g,b;};
RGB lambert(V normal,V towardLight,RGB albedo,RGB radiance,double ambient){
 if(!finite(normal)||!finite(towardLight)||!std::isfinite(ambient)||ambient<0||!std::isfinite(albedo.r)||!std::isfinite(albedo.g)||!std::isfinite(albedo.b))throw std::invalid_argument("bad light");
 const double amount=std::max(0.0,dot(unit(normal),unit(towardLight)));
 return {albedo.r*(ambient+radiance.r*amount),albedo.g*(ambient+radiance.g*amount),albedo.b*(ambient+radiance.b*amount)};
}
struct State{bool depthTest,depthWrite,blend;enum Compare{Less,LessEqual} compare;};
State opaque(){return {true,true,false,State::Less};}
State skybox(){return {true,false,false,State::LessEqual};}
struct SkyboxCube{unsigned arraySize;bool textureCube;unsigned mips;};
bool validCube(SkyboxCube c){return c.textureCube&&c.arraySize==6&&c.mips>0;}
bool depthAccept(double sample,double stored,State s){
 if(!std::isfinite(sample)||!std::isfinite(stored))throw std::invalid_argument("depth finite");
 if(!s.depthTest)return true;
 return s.compare==State::Less?(sample<stored):(sample<=stored);
}
V skyboxPosition(V vertex,V camera){return {vertex.x+camera.x,vertex.y+camera.y,vertex.z+camera.z};}
int main(){unsigned n=0;auto ck=[&](bool b,const char* s){++n;if(!b)throw std::runtime_error(s);};
 auto eq=[](double a,double b){return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<1e-9;};
 State o=opaque(),s=skybox();ck(o.depthWrite,"opaque writes depth");ck(!o.blend,"opaque not blended");
 ck(s.depthTest,"skybox depth test");ck(!s.depthWrite,"skybox not write depth");ck(s.compare==State::LessEqual,"skybox inclusive far");
 ck(!depthAccept(1,1,o),"default LESS reject far");ck(depthAccept(1,1,s),"LEQUAL accepts far");
 ck(!depthAccept(1,.3,s),"skybox behind object");ck(depthAccept(.2,.3,s),"depth accepts near");
 ck(validCube({6,true,1}),"cube 6 faces");ck(!validCube({5,true,1}),"cube missing face");
 ck(!validCube({6,false,1}),"array not cube");ck(!validCube({6,true,0}),"zero mips");
 RGB white{1,1,1};const auto front=lambert({0,1,0},{0,1,0},white,white,.1);
 ck(eq(front.r,1.1),"front lambert");ck(eq(front.g,1.1),"front green");
 const auto back=lambert({0,1,0},{0,-1,0},white,white,.1);
 ck(eq(back.r,.1),"back ambient only");
 const auto side=lambert({0,1,0},{1,0,0},white,white,.1);
 ck(eq(side.b,.1),"side ambient");
 RGB tinted{.2,.4,.8},sun{2,1,.5};
 const auto mixed=lambert({0,1,0},{0,1,0},tinted,sun,.25);
 ck(eq(mixed.r,.45),"red tint");ck(eq(mixed.g,.5),"green tint");ck(eq(mixed.b,.6),"blue tint");
 const auto moved=skyboxPosition({1,2,3},{10,20,30});
 ck(eq(moved.x,11),"skybox translate x");ck(eq(moved.y,22),"skybox translate y");
 ck(eq(moved.z,33),"skybox translate z");
 const auto shifted=skyboxPosition({1,2,3},{-10,-20,-30});ck(eq(shifted.x,-9),"camera centered");
 bool caught=false;try{lambert({0,0,0},{0,1,0},white,white,0);}catch(const std::invalid_argument&){caught=true;}ck(caught,"reject zero normal");
 caught=false;try{lambert({0,1,0},{0,0,0},white,white,0);}catch(const std::invalid_argument&){caught=true;}ck(caught,"reject zero light");
 caught=false;try{lambert({0,1,0},{0,1,0},white,white,-.1);}catch(const std::invalid_argument&){caught=true;}ck(caught,"reject ambient");
 caught=false;try{depthAccept(std::numeric_limits<double>::quiet_NaN(),1,s);}catch(const std::invalid_argument&){caught=true;}ck(caught,"reject nan depth");
 std::cout<<"PASS "<<n<<" skybox/lighting CPU contract checks\n";
}

```
## skybox_lambert.hlsl
```plain text
// HLSL illustrative independent snippets. GPU and FXC/DXC compile NOT RUN.
cbuffer PerFrame : register(b0) { row_major float4x4 worldViewProj; float4 sunDirection; float4 sunColor; };
TextureCube skyTexture : register(t0);
SamplerState skySampler : register(s0);
struct VSIn { float3 pos : POSITION; float3 normal : NORMAL; };
struct VSOut { float4 clip : SV_POSITION; float3 local : TEXCOORD0; float3 n : TEXCOORD1; };
VSOut VSMain(VSIn input) { VSOut o; o.clip=mul(float4(input.pos,1),worldViewProj); o.local=input.pos; o.n=input.normal; return o; }
float4 PSSky(VSOut input) : SV_Target { return skyTexture.Sample(skySampler, normalize(input.local)); }
float4 PSLambert(VSOut input) : SV_Target {
    float3 N=normalize(input.n);
    float3 towardSun=normalize(-sunDirection.xyz); // convention: sunDirection points where sunlight travels
    return float4(saturate(dot(N,towardSun))*sunColor.rgb + 0.12f, 1);
}

```
## Root CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_study_05_08 LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 default, 11 compatibility")
enable_testing()
add_subdirectory(05_textured_rect)
add_subdirectory(06_transform_camera)
add_subdirectory(07_skybox_lighting)
add_subdirectory(08_terrain)

```
## Full validation logs (all configurations rerun same tests)
### gcc20
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch05_08/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_batch05_08/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_05_cpu

1: Test command: /mnt/data/dx11_batch05_08/build-gcc20/05_textured_rect/dx11_05_cpu
1: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/05_textured_rect
1: Test timeout computed to be: 10000000
1: PASS 29 textured rect CPU contract checks
1/4 Test #1: dx11_05_cpu ......................   Passed    0.01 sec
test 2
    Start 2: dx11_06_cpu

2: Test command: /mnt/data/dx11_batch05_08/build-gcc20/06_transform_camera/dx11_06_cpu
2: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/06_transform_camera
2: Test timeout computed to be: 10000000
2: PASS 30 transform/camera CPU contract checks
2/4 Test #2: dx11_06_cpu ......................   Passed    0.01 sec
test 3
    Start 3: dx11_07_cpu

3: Test command: /mnt/data/dx11_batch05_08/build-gcc20/07_skybox_lighting/dx11_07_cpu
3: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/07_skybox_lighting
3: Test timeout computed to be: 10000000
3: PASS 28 skybox/lighting CPU contract checks
3/4 Test #3: dx11_07_cpu ......................   Passed    0.02 sec
test 4
    Start 4: dx11_08_cpu

4: Test command: /mnt/data/dx11_batch05_08/build-gcc20/08_terrain/dx11_08_cpu
4: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/08_terrain
4: Test timeout computed to be: 10000000
4: PASS 33 terrain CPU contract checks
4/4 Test #4: dx11_08_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.05 sec

```
### gcc11
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch05_08/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_batch05_08/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_05_cpu

1: Test command: /mnt/data/dx11_batch05_08/build-gcc11/05_textured_rect/dx11_05_cpu
1: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/05_textured_rect
1: Test timeout computed to be: 10000000
1: PASS 29 textured rect CPU contract checks
1/4 Test #1: dx11_05_cpu ......................   Passed    0.01 sec
test 2
    Start 2: dx11_06_cpu

2: Test command: /mnt/data/dx11_batch05_08/build-gcc11/06_transform_camera/dx11_06_cpu
2: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/06_transform_camera
2: Test timeout computed to be: 10000000
2: PASS 30 transform/camera CPU contract checks
2/4 Test #2: dx11_06_cpu ......................   Passed    0.00 sec
test 3
    Start 3: dx11_07_cpu

3: Test command: /mnt/data/dx11_batch05_08/build-gcc11/07_skybox_lighting/dx11_07_cpu
3: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/07_skybox_lighting
3: Test timeout computed to be: 10000000
3: PASS 28 skybox/lighting CPU contract checks
3/4 Test #3: dx11_07_cpu ......................   Passed    0.02 sec
test 4
    Start 4: dx11_08_cpu

4: Test command: /mnt/data/dx11_batch05_08/build-gcc11/08_terrain/dx11_08_cpu
4: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/08_terrain
4: Test timeout computed to be: 10000000
4: PASS 33 terrain CPU contract checks
4/4 Test #4: dx11_08_cpu ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.05 sec

```
### clangsan
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch05_08/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_batch05_08/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_05_cpu

1: Test command: /mnt/data/dx11_batch05_08/build-clangsan/05_textured_rect/dx11_05_cpu
1: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/05_textured_rect
1: Test timeout computed to be: 10000000
1: PASS 29 textured rect CPU contract checks
1/4 Test #1: dx11_05_cpu ......................   Passed    0.02 sec
test 2
    Start 2: dx11_06_cpu

2: Test command: /mnt/data/dx11_batch05_08/build-clangsan/06_transform_camera/dx11_06_cpu
2: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/06_transform_camera
2: Test timeout computed to be: 10000000
2: PASS 30 transform/camera CPU contract checks
2/4 Test #2: dx11_06_cpu ......................   Passed    0.03 sec
test 3
    Start 3: dx11_07_cpu

3: Test command: /mnt/data/dx11_batch05_08/build-clangsan/07_skybox_lighting/dx11_07_cpu
3: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/07_skybox_lighting
3: Test timeout computed to be: 10000000
3: PASS 28 skybox/lighting CPU contract checks
3/4 Test #3: dx11_07_cpu ......................   Passed    0.02 sec
test 4
    Start 4: dx11_08_cpu

4: Test command: /mnt/data/dx11_batch05_08/build-clangsan/08_terrain/dx11_08_cpu
4: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/08_terrain
4: Test timeout computed to be: 10000000
4: PASS 33 terrain CPU contract checks
4/4 Test #4: dx11_08_cpu ......................   Passed    0.04 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.11 sec

```
