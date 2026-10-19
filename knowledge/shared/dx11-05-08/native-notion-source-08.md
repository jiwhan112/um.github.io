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
add_executable(dx11_08_cpu main.cpp)
set_target_properties(dx11_08_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_08_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_08_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME dx11_08_cpu COMMAND dx11_08_cpu)

```
## main.cpp
```c++
// C++11 terrain mesh and triangular height sampling CPU model; no Direct3D call.
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct V {double x,y,z;};
struct Terrain {
 std::size_t width,depth;std::vector<double> height;
 Terrain(std::size_t w,std::size_t d,std::vector<double> h):width(w),depth(d),height(std::move(h)){
  if(w<2||d<2||w>std::numeric_limits<std::size_t>::max()/d||height.size()!=w*d)
   throw std::invalid_argument("terrain shape");
  for(double x:height)if(!std::isfinite(x))throw std::invalid_argument("height finite");
 }
 std::size_t cells()const{return (width-1)*(depth-1);}
 std::size_t tris()const{return cells()*2;}
 std::vector<std::uint32_t> indices()const{
  if(width*depth>std::numeric_limits<std::uint32_t>::max()||cells()>std::numeric_limits<std::size_t>::max()/6)
   throw std::length_error("index capacity");
  std::vector<std::uint32_t> out;out.reserve(cells()*6);
  for(std::size_t z=0;z+1<depth;++z)for(std::size_t x=0;x+1<width;++x){
   const auto a=static_cast<std::uint32_t>(z*width+x);
   const auto b=a+1,c=a+static_cast<std::uint32_t>(width),d=c+1;
   out.insert(out.end(),{a,b,c,b,d,c});
  }
  return out;
 }
 double sample(double x,double z)const{
  if(!std::isfinite(x)||!std::isfinite(z)||x<0||z<0||x>width-1||z>depth-1)
   throw std::out_of_range("terrain sample");
  const std::size_t cx=x==width-1?width-2:static_cast<std::size_t>(x);
  const std::size_t cz=z==depth-1?depth-2:static_cast<std::size_t>(z);
  const double fx=x-cx,fz=z-cz;
  const double a=height[cz*width+cx],b=height[cz*width+cx+1];
  const double c=height[(cz+1)*width+cx],d=height[(cz+1)*width+cx+1];
  // Divide quad across b--c diagonal; triangles a,b,c and b,d,c.
  return fx+fz<=1? a*(1-fx-fz)+b*fx+c*fz
                  : b*(1-fz)+c*(1-fx)+d*(fx+fz-1);
 }
};
static bool validateRGBA(const std::vector<std::uint8_t>& bytes,std::size_t width,std::size_t height,std::size_t pitch){
 if(!width||!height||width>std::numeric_limits<std::size_t>::max()/4)return false;
 const auto row=width*4;
 return pitch>=row && height<=std::numeric_limits<std::size_t>::max()/pitch && bytes.size()>=pitch*height;
}
static std::size_t texOffset(std::size_t x,std::size_t y,std::size_t pitch){return y*pitch+x*4;}
static bool near(double a,double b){return std::isfinite(a)&&std::isfinite(b)&&std::fabs(a-b)<1e-9;}
int main(){unsigned n=0;auto ck=[&](bool b,const char* m){++n;if(!b)throw std::runtime_error(m);};
 Terrain t(10,10,std::vector<double>(100,0));
 ck(t.width*t.depth==100,"vertex count");ck(t.cells()==81,"cell count");ck(t.tris()==162,"triangle count");
 auto ib=t.indices();ck(ib.size()==486,"index count");ck(ib[0]==0,"first a");ck(ib[1]==1,"first b");ck(ib[2]==10,"first c");
 ck(ib[3]==1&&ib[4]==11&&ib[5]==10,"other triangle");
 ck(ib.back()==98,"last index");
 ck(near(t.sample(0,0),0),"origin");ck(near(t.sample(9,9),0),"last vertex");
 Terrain small(2,2,{0,0,0,10});ck(near(small.sample(.25,.25),0),"first triangle");
 ck(near(small.sample(.75,.75),5),"second triangle differs bilinear");
 ck(near(small.sample(1,1),10),"corner d");ck(near(small.sample(1,0),0),"corner b");
 ck(near(small.sample(0,1),0),"corner c");ck(near(small.sample(.5,.5),0),"diagonal");
 Terrain ramp(2,2,{0,10,20,30});ck(near(ramp.sample(.25,.25),7.5),"ramp 1");
 ck(near(ramp.sample(.75,.75),22.5),"ramp 2");
 bool caught=false;try{small.sample(-.01,0);}catch(const std::out_of_range&){caught=true;}ck(caught,"negative x");
 caught=false;try{small.sample(0,1.01);}catch(const std::out_of_range&){caught=true;}ck(caught,"z overflow");
 caught=false;try{small.sample(std::numeric_limits<double>::quiet_NaN(),0);}catch(const std::out_of_range&){caught=true;}ck(caught,"nan coords");
 caught=false;try{Terrain bad(1,2,{0,0});}catch(const std::invalid_argument&){caught=true;}ck(caught,"bad width");
 caught=false;try{Terrain bad(2,2,{0,1,2});}catch(const std::invalid_argument&){caught=true;}ck(caught,"missing height");
 caught=false;try{Terrain bad(2,2,{0,1,std::numeric_limits<double>::infinity(),2});}catch(const std::invalid_argument&){caught=true;}ck(caught,"infinite height");
 std::vector<std::uint8_t> rgba(32,0);
 ck(validateRGBA(rgba,2,2,16),"pitch padded");ck(texOffset(0,1,16)==16,"second row start");
 ck(texOffset(1,1,16)==20,"second pixel second row");ck(!validateRGBA(rgba,2,2,7),"short pitch");
 ck(!validateRGBA(std::vector<std::uint8_t>(20),2,2,16),"short storage");
 ck(!validateRGBA(rgba,0,2,16),"zero width");ck(!validateRGBA(rgba,2,0,16),"zero height");
 ck(!validateRGBA(rgba,std::numeric_limits<std::size_t>::max(),2,16),"overflow width");
 std::cout<<"PASS "<<n<<" terrain CPU contract checks\n";
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
