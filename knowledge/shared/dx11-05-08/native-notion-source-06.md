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
add_executable(dx11_06_cpu main.cpp)
set_target_properties(dx11_06_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_06_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_06_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME dx11_06_cpu COMMAND dx11_06_cpu)

```
## main.cpp
```c++
// DX11 coordinate contract: portable C++11 CPU model, not DirectXMath or GPU.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
struct V {double x,y,z;};
V operator+(V a,V b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V operator-(V a,V b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V operator*(V a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V cross(V a,V b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
double norm(V v){return std::sqrt(dot(v,v));}
V unit(V v){double n=norm(v);if(!(n>1e-12)||!std::isfinite(n))throw std::invalid_argument("bad axis");return v*(1/n);}
struct M {double v[4][4]={};};
M identity(){M x;for(int i=0;i<4;++i)x.v[i][i]=1;return x;}
M mul(M a,M b){M o;for(int i=0;i<4;++i)for(int j=0;j<4;++j)for(int k=0;k<4;++k)o.v[i][j]+=a.v[i][k]*b.v[k][j];return o;}
M translate(V t){M x=identity();x.v[3][0]=t.x;x.v[3][1]=t.y;x.v[3][2]=t.z;return x;}
M scale(V s){M x=identity();x.v[0][0]=s.x;x.v[1][1]=s.y;x.v[2][2]=s.z;return x;}
M yaw(double t){M x=identity();double c=std::cos(t),s=std::sin(t);x.v[0][0]=c;x.v[0][2]=-s;x.v[2][0]=s;x.v[2][2]=c;return x;}
struct P4 {double x,y,z,w;};
P4 transform(P4 p,M m){const double a[4]={p.x,p.y,p.z,p.w};double b[4]={};for(int j=0;j<4;++j)for(int i=0;i<4;++i)b[j]+=a[i]*m.v[i][j];return {b[0],b[1],b[2],b[3]};}
M lookAtLH(V eye,V at,V up){
 const V f=unit(at-eye),r=unit(cross(up,f)),u=cross(f,r);M v=identity();
 v.v[0][0]=r.x;v.v[1][0]=r.y;v.v[2][0]=r.z;
 v.v[0][1]=u.x;v.v[1][1]=u.y;v.v[2][1]=u.z;
 v.v[0][2]=f.x;v.v[1][2]=f.y;v.v[2][2]=f.z;
 v.v[3][0]=-dot(eye,r);v.v[3][1]=-dot(eye,u);v.v[3][2]=-dot(eye,f);return v;
}
M perspectiveLH(double fovy,double aspect,double zn,double zf){
 if(!std::isfinite(fovy)||!std::isfinite(aspect)||!std::isfinite(zn)||!std::isfinite(zf)||
   !(fovy>0&&fovy<3.141592653589793&&aspect>0&&zn>0&&zf>zn))throw std::invalid_argument("bad perspective");
 const double h=1/std::tan(fovy/2);M p{};p.v[0][0]=h/aspect;p.v[1][1]=h;p.v[2][2]=zf/(zf-zn);
 p.v[2][3]=1;p.v[3][2]=-zn*zf/(zf-zn);return p;
}
P4 ndc(P4 clip){if(!std::isfinite(clip.w)||std::abs(clip.w)<1e-12)throw std::invalid_argument("bad w");return {clip.x/clip.w,clip.y/clip.w,clip.z/clip.w,1};}
struct Viewport {double x,y,w,h;};
V screen(P4 n,Viewport vp){if(!(vp.w>0&&vp.h>0))throw std::invalid_argument("bad viewport");return {vp.x+(n.x+1)*vp.w/2,vp.y+(1-n.y)*vp.h/2,n.z};}
bool close(double a,double b){return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<1e-8;}
int main(){unsigned n=0;const auto ck=[&](bool b,const char* m){++n;if(!b)throw std::runtime_error(m);};
 const M i=identity();P4 p{2,3,4,1};auto q=transform(p,i);
 ck(close(q.x,2),"identity x");ck(close(q.y,3),"identity y");ck(close(q.z,4),"identity z");
 q=transform(p,translate({10,20,30}));ck(close(q.x,12),"translated x");ck(close(q.y,23),"translated y");ck(close(q.z,34),"translated z");
 auto d=transform({1,0,0,0},translate({10,0,0}));ck(close(d.x,1),"direction unaffected");
 const auto child=mul(scale({2,2,2}),translate({1,2,3}));q=transform({1,0,0,1},mul(child,translate({10,0,0})));
 ck(close(q.x,13),"child parent");ck(close(q.y,2),"child y");
 q=transform({1,0,0,1},mul(yaw(3.141592653589793/2),translate({1,2,3})));
 ck(close(q.x,1),"yaw x");ck(close(q.z,2),"yaw z");
 const M v=lookAtLH({0,0,-5},{0,0,0},{0,1,0});q=transform({0,0,0,1},v);
 ck(close(q.z,5),"view forward");ck(close(q.x,0),"view center");
 const M proj=perspectiveLH(3.141592653589793/2,1,1,101);
 auto c=transform({0,0,1,1},proj);ck(close(c.z/c.w,0),"near ndc depth");
 c=transform({0,0,101,1},proj);ck(close(c.z/c.w,1),"far ndc depth");
 c=transform({1,0,1,1},proj);ck(close(c.x/c.w,1),"right clip");
 c=transform({-1,0,1,1},proj);ck(close(c.x/c.w,-1),"left clip");
 c=transform({0,1,1,1},proj);ck(close(c.y/c.w,1),"top clip");
 const auto center=screen(ndc(transform({0,0,5,1},proj)),{100,50,800,600});
 ck(close(center.x,500),"viewport x offset");ck(close(center.y,350),"viewport flipped y");
 auto top=screen({0,1,.5,1},{100,50,800,600});ck(close(top.y,50),"screen top");
 auto bottom=screen({0,-1,.5,1},{100,50,800,600});ck(close(bottom.y,650),"screen bottom");
 auto bad=[&](bool b){ck(b,"must reject invalid input");};bool caught=false;
 try{perspectiveLH(0,1,1,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{perspectiveLH(1,0,1,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{perspectiveLH(1,1,0,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{perspectiveLH(1,1,100,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{lookAtLH({0,0,0},{0,0,0},{0,1,0});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{lookAtLH({0,0,0},{0,0,1},{0,0,1});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{ndc({1,2,3,0});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{screen({0,0,0,1},{0,0,0,1});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 std::cout<<"PASS "<<n<<" transform/camera CPU contract checks\n";
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
