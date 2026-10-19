
이 페이지는 <mention-page url="https://app.notion.com/p/3fba99703f40816eb333f270aad46a46">11 · Frustum 컬링 — 6평면·Sphere·AABB</mention-page>의 실행 가능한 전체 실습입니다. 첨부 다운로드 도구가 빈 결과를 반환하여, 다음 실행에서도 읽을 수 있는 네이티브 코드 블록으로 전체 파일을 함께 보존합니다. Linux CPU 검증이며 D3D9/GPU 실행 코드가 아닙니다.
## 1. 사용법과 파일 계약
아래 첫 코드 블록을 main.cpp로, 두 번째를 CMakeLists.txt로 저장합니다. 표준 C++ 라이브러리만 사용하며 외부 입력·에셋은 없습니다. CMake 기본은 C++20입니다. C++11 호환성은 GCC로 별도 검증했고 회사 프로젝트 표준은 바꾸지 않습니다. 성공 출력은 PASS 39 named checks이고 종료 코드 0, 오류는 stderr와 종료 코드 1입니다. 검사 한 개는 15,000표본의 속성 검사이며 CTest 등록 테스트는 실행파일 하나입니다.
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 2
ctest --test-dir build --output-on-failure
```
Windows/MSVC 수동 확인 절차는 cmake 구성 후 `cmake --build build --config Release`, `ctest --test-dir build -C Release --output-on-failure`입니다. 이번 실행에서는 수행하지 않았습니다. 사용한 std::array, 람다, 범위 for, enum class, using 별칭, 목록 초기화는 C++11 기능입니다.
## 2. main.cpp — 전체 파일
```c++
// DX9 chapter 11 v1.1. CPU-only, row-vector LH; NOT a D3D9 renderer.
// C++20 teaching build; syntax and standard-library use also tested as C++11.
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
struct V3 { double x,y,z; };
struct V4 { double x,y,z,w; };
struct Mat { double a[4][4]; };
struct Plane { V3 n; double d; }; // inside if dot(n,p)+d >= 0
struct Sphere { V3 center; double radius; };
struct Aabb { V3 center,half; }; // WORLD ALIGNED, not OBB
using Frustum=std::array<Plane,6>;
enum class Result { Out, Intersect, In };
V4 col(const Mat&m,int j){return {m.a[0][j],m.a[1][j],m.a[2][j],m.a[3][j]};}
V4 plus(V4 a,V4 b){return {a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w};}
V4 minus(V4 a,V4 b){return {a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w};}
bool ok(double v){return std::isfinite(v);}
bool ok(V3 v){return ok(v.x)&&ok(v.y)&&ok(v.z);}
Plane unit(V4 v){
  double len=std::hypot(v.x,std::hypot(v.y,v.z));
  if(!ok(len)||len<1.e-12||!ok(v.w))throw std::invalid_argument("plane");
  const Plane p{{v.x/len,v.y/len,v.z/len},v.w/len};
  if(!ok(p.n)||!ok(p.d))throw std::invalid_argument("normalized plane overflow");
  return p;
}
Frustum extract(const Mat&m){
  for(const auto&row:m.a)for(double n:row)if(!ok(n))throw std::invalid_argument("matrix");
  const V4 x=col(m,0),y=col(m,1),z=col(m,2),w=col(m,3);
  // x,y in [-w,w], z in [0,w]. Inward normals.
  return {{unit(plus(w,x)),unit(minus(w,x)),unit(plus(w,y)),
           unit(minus(w,y)),unit(z),unit(minus(w,z))}};
}
// Public classify functions require six inward, unit, finite planes.
// Validate all six before early-out so malformed data is not silently accepted.
void validate(const Frustum& f){
  for(const Plane& p:f){
    const double n=std::hypot(p.n.x,std::hypot(p.n.y,p.n.z));
    if(!ok(p.n)||!ok(p.d)||!ok(n)||std::abs(n-1.)>1.e-8)
      throw std::invalid_argument("frustum requires finite unit planes");
  }
}
Mat mul(const Mat&a,const Mat&b){
  Mat c{};
  for(int i=0;i<4;++i){
    for(int j=0;j<4;++j){
      for(int k=0;k<4;++k)c.a[i][j]+=a.a[i][k]*b.a[k][j];
    }
  }
  return c;
}
Mat projLH(double fov,double aspect,double nearZ,double farZ){
  if(!ok(fov)||!ok(aspect)||!ok(nearZ)||!ok(farZ)||
     fov<=0||fov>=std::acos(-1.)||aspect<=0||nearZ<=0||farZ<=nearZ)
    throw std::invalid_argument("projection");
  double ys=1./std::tan(fov/2),zs=farZ/(farZ-nearZ);
  return {{{ys/aspect,0,0,0},{0,ys,0,0},{0,0,zs,1},
           {0,0,-nearZ*zs,0}}};
}
Mat cameraView(V3 eye){
  if(!ok(eye))throw std::invalid_argument("camera");
  return {{{1,0,0,0},{0,1,0,0},{0,0,1,0},
           {-eye.x,-eye.y,-eye.z,1}}};
}
V4 clip(V3 p,const Mat&m){
  double a[4]={p.x,p.y,p.z,1},r[4]={};
  for(int j=0;j<4;++j)for(int k=0;k<4;++k)r[j]+=a[k]*m.a[k][j];
  return {r[0],r[1],r[2],r[3]};
}
bool insideClip(V3 p,const Mat&m){
  V4 v=clip(p,m);
  return v.w>0 && -v.w<=v.x&&v.x<=v.w&&-v.w<=v.y&&v.y<=v.w&&
         0<=v.z&&v.z<=v.w;
}
Result classify(const Frustum&f,Sphere s,double epsilon=1.e-9){
  if(!ok(s.center)||!ok(s.radius)||s.radius<0||!ok(epsilon)||epsilon<0)
    throw std::invalid_argument("sphere");
  validate(f);
  bool part=false;
  for(auto p:f){
    double d=p.n.x*s.center.x+p.n.y*s.center.y+p.n.z*s.center.z+p.d;
    if(!ok(d))throw std::invalid_argument("overflow");
    if(d < -s.radius-epsilon)return Result::Out;
    if(d <= s.radius+epsilon)part=true;
  }
  return part?Result::Intersect:Result::In;
}
Result classify(const Frustum&f,Aabb b,double epsilon=1.e-9){
  if(!ok(b.center)||!ok(b.half)||b.half.x<0||b.half.y<0||b.half.z<0||
     !ok(epsilon)||epsilon<0)throw std::invalid_argument("box");
  validate(f);
  bool part=false;
  for(auto p:f){
    double d=p.n.x*b.center.x+p.n.y*b.center.y+p.n.z*b.center.z+p.d;
    double r=std::abs(p.n.x)*b.half.x+std::abs(p.n.y)*b.half.y+
             std::abs(p.n.z)*b.half.z;
    if(!ok(d)||!ok(r))throw std::invalid_argument("overflow");
    if(d < -r-epsilon)return Result::Out;
    if(d <= r+epsilon)part=true;
  }
  return part?Result::Intersect:Result::In;
}
template<typename F>bool invalid(F fn){
  try{fn();}catch(const std::invalid_argument&){return true;}return false;
}
int runTests(){
  int checks=0;
  auto test=[&](bool condition,const char*label){
    ++checks;if(!condition)throw std::runtime_error(std::string("FAIL: ")+label);
  };
  const Mat projection=projLH(std::acos(-1.)/2,1,1,10);
  const Frustum f=extract(projection);
  test(classify(f,Sphere{{0,0,5},0.1})==Result::In,"sphere inside");
  test(classify(f,Sphere{{0,0,0.2},0.1})==Result::Out,"sphere near outside");
  test(classify(f,Sphere{{0,0,11},0.2})==Result::Out,"sphere far outside");
  test(classify(f,Sphere{{-10,0,2},0.1})==Result::Out,"sphere left outside");
  test(classify(f,Sphere{{10,0,2},0.1})==Result::Out,"sphere right outside");
  test(classify(f,Sphere{{0,10,2},0.1})==Result::Out,"sphere top outside");
  test(classify(f,Sphere{{0,-10,2},0.1})==Result::Out,"sphere bottom outside");
  test(classify(f,Sphere{{0,0,0.5},0.5})==Result::Intersect,"sphere near tangent");
  test(classify(f,Sphere{{0,0,10.5},0.5})==Result::Intersect,"sphere far tangent");
  test(classify(f,Sphere{{0,0,5},0})==Result::In,"zero-radius sphere");
  test(classify(f,Aabb{{0,0,5},{0.5,0.5,0.5}})==Result::In,"box inside");
  test(classify(f,Aabb{{0,0,0.5},{0.1,0.1,0.1}})==Result::Out,"box near outside");
  test(classify(f,Aabb{{0,0,11},{0.1,0.1,0.1}})==Result::Out,"box far outside");
  test(classify(f,Aabb{{0,0,0.8},{0.1,0.1,0.2}})==Result::Intersect,"box near tangent");
  test(classify(f,Aabb{{3,0,2},{1.5,1.5,0.5}})==Result::Intersect,"box crosses right");
  test(classify(f,Aabb{{0,0,5},{0,0,0}})==Result::In,"zero-half box");
  const Frustum translated=extract(mul(cameraView({0,0,5}),projection));
  test(classify(translated,Sphere{{0,0,8},0.1})==Result::In,"translated camera inside");
  test(classify(translated,Sphere{{0,0,5},0.1})==Result::Out,"translated camera near reject");
  test(classify(translated,Aabb{{0,0,16},{0.1,0.1,0.1}})==Result::Out,"translated camera far reject");
  test(invalid([&]{projLH(1,1,1,1);}),"near==far rejected");
  test(invalid([&]{projLH(1,0,1,10);}),"aspect zero rejected");
  test(invalid([&]{extract(Mat{});}),"degenerate plane rejected");
  test(invalid([&]{Mat bad=projection;bad.a[0][0]=std::numeric_limits<double>::quiet_NaN();extract(bad);}),"NaN matrix rejected");
  test(invalid([&]{classify(f,Sphere{{0,0,2},-1});}),"negative radius rejected");
  test(invalid([&]{classify(f,Aabb{{0,0,2},{1,-1,1}});}),"negative extent rejected");
  test(invalid([&]{classify(f,Sphere{{0,0,2},std::numeric_limits<double>::infinity()});}),"infinite radius rejected");
  test(invalid([&]{classify(f,Aabb{{0,0,2},{1,1,1}},-0.1);}),"negative tolerance rejected");
  // v1.1: 11 additional named regression checks, not more random samples.
  const Plane scaled=unit({0,0,2,-2});
  test(scaled.n.z==1 && scaled.d==-1,"normalize both normal and D");
  test(invalid([&]{unit({1.e-11,0,0,std::numeric_limits<double>::max()});}),
       "normalized D overflow rejected");
  test(invalid([&]{classify(f,Sphere{{0,0,2},0.1},std::numeric_limits<double>::quiet_NaN());}),
       "NaN tolerance rejected");
  test(invalid([&]{classify(f,Aabb{{0,0,2},{1,std::numeric_limits<double>::infinity(),1}});}),
       "infinite extent rejected");
  const Sphere closeToNear{{0,0,1.-5.e-10},0};
  test(classify(f,closeToNear,1.e-9)!=Result::Out && classify(f,closeToNear,0)==Result::Out,
       "epsilon keeps near-boundary uncertainty");
  // Camera at origin, oriented toward world +X. World x becomes view z.
  const Mat viewPlusX={{{0,0,1,0},{0,1,0,0},{-1,0,0,0},{0,0,0,1}}};
  const Frustum rotated=extract(mul(viewPlusX,projection));
  test(classify(rotated,Sphere{{5,0,0},0.1})==Result::In,"rotated camera inside");
  test(classify(rotated,Sphere{{-5,0,0},0.1})==Result::Out,"rotated camera behind");
  // Orthographic: width=height=4; near=1; far=10. Same clip inequalities.
  const Mat ortho={{{0.5,0,0,0},{0,0.5,0,0},{0,0,1./9,0},{0,0,-1./9,1}}};
  const Frustum of=extract(ortho);
  test(classify(of,Aabb{{0,0,5},{0.5,0.5,0.5}})==Result::In,"orthographic box inside");
  test(classify(of,Sphere{{2.2,0,5},0.1})==Result::Out,"orthographic right reject");
  Frustum wrongNear=f;
  wrongNear[4]=unit(plus(col(projection,3),col(projection,2)));
  const Sphere beforeNear{{0,0,0.75},0};
  test(classify(f,beforeNear)==Result::Out && classify(wrongNear,beforeNear)!=Result::Out,
       "OpenGL near extraction is wrong for this D3D9 projection");
  test(invalid([&]{Frustum bad=f;bad[0].n={0,0,0};classify(bad,Sphere{{0,0,5},0.1});}),
       "nonunit frustum rejected before classification");
  // A single property-based test, not 15,000 independent named tests.
  std::size_t sampleCount=0;
  for(int iz=0;iz<24;++iz)for(int iy=-12;iy<=12;++iy)for(int ix=-12;ix<=12;++ix){
    V3 p{ix*0.31+0.017,iy*0.27+0.013,0.3+iz*0.49};
    if((classify(f,Sphere{p,0},0)!=Result::Out)!=insideClip(p,projection))
      throw std::runtime_error("clip-vs-plane mismatch");
    ++sampleCount;
  }
  test(sampleCount==15000,"15,000 sampled clip comparisons");
  std::cout<<"PASS "<<checks<<" named checks; property sample points="<<sampleCount
           <<". CPU-only (NOT D3D9 GPU).\n";
  return 0;
}
int main(){
  try{return runTests();}
  catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
```
## 3. CMakeLists.txt — 전체 파일
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx9_frustum11_cpu LANGUAGES CXX)
set(FRUSTUM_CXX_STANDARD 20 CACHE STRING "Teaching standard: 20, compatibility: 11")
add_executable(frustum_cpu main.cpp)
set_target_properties(frustum_cpu PROPERTIES
  CXX_STANDARD ${FRUSTUM_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
  target_compile_options(frustum_cpu PRIVATE /W4 /permissive- /EHsc)
else()
  target_compile_options(frustum_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME frustum_contract COMMAND frustum_cpu)
```
## 4. VALIDATION.txt — 실제 실행 로그
```plain text
StartedUTC: 2026-10-16T04:54:51.948361+00:00
Platform: Linux-6.18.44-x86_64-with-glibc2.41
SourceSHA256: dcbb2900c718e7c3fb19dcfb8158a54cb784237ba267a708e462e52628a62925

$ g++ --version
g++ (Debian 14.2.0-19) 14.2.0
Copyright (C) 2024 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.



exit=0

$ clang++ --version
clang version 17.0.0 (https://github.com/swiftlang/llvm-project.git 10999b6d034fe318f3d56c83bddb6572593a8bb0)
Target: x86_64-unknown-linux-gnu
Thread model: posix
InstalledDir: /usr/local/swift/usr/bin


exit=0

$ cmake --version
cmake version 3.31.6

CMake suite maintained and supported by Kitware (kitware.com/cmake).


exit=0

$ g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o frustum_gcc20


exit=0

$ ./frustum_gcc20
PASS 39 named checks; property sample points=15000. CPU-only (NOT D3D9 GPU).


exit=0

$ clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o frustum_clang20


exit=0

$ ./frustum_clang20
PASS 39 named checks; property sample points=15000. CPU-only (NOT D3D9 GPU).


exit=0

$ clang++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer main.cpp -o frustum_san


exit=0

$ ./frustum_san
PASS 39 named checks; property sample points=15000. CPU-only (NOT D3D9 GPU).


exit=0

$ g++ -std=c++11 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o frustum_gcc11


exit=0

$ ./frustum_gcc11
PASS 39 named checks; property sample points=15000. CPU-only (NOT D3D9 GPU).


exit=0

$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx9_frustum11_resume/build


exit=0

$ cmake --build build -j 2
[ 50%] Building CXX object CMakeFiles/frustum_cpu.dir/main.cpp.o
[100%] Linking CXX executable frustum_cpu
[100%] Built target frustum_cpu


exit=0

$ ctest --test-dir build --output-on-failure
Internal ctest changing into directory: /mnt/data/dx9_frustum11_resume/build
Test project /mnt/data/dx9_frustum11_resume/build
    Start 1: frustum_contract
1/1 Test #1: frustum_contract .................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec


exit=0

EndedUTC: 2026-10-16T04:54:57.372147+00:00
Count: 39 named checks. One is a property check over 15000 deterministic points.
Same suite rerun in four toolchain configurations; do not add these as distinct cases.
CTest: one registered executable test, not 39 registered CTest cases.
Windows / MSVC / D3D9 / GPU / renderer performance: NOT RUN.
```
## 5. 변경 이력과 제한
<mention-date start="2026-10-16"/> v1.1. 중단 당시 main.cpp의 SHA-256을 확인한 뒤 수정 전 28검사를 재실행했습니다. n과 D 정규화, 정규화 결과 overflow 거부, 평면 입력 검증, main 예외 처리 및 회전·직교·잘못된 Near·경계 허용 오차 등 11검사를 보강했습니다. 현재 main.cpp의 예상 UTF-8 LF 바이트 수는 9369이고 SHA-256은 위 로그에 있습니다. 코드 블록 복사 시 파일 마지막 개행을 포함합니다.
cameraView는 이동 전용, 회전 테스트는 고정 행렬입니다. 실습의 6평면 검사는 보수적 가시성 검사이며 OBB/SAT·실제 D3D9 Draw·GPU·드라이버·전체 입력 범위 수치 안정성 검증은 아닙니다. 수학적 유도, 원본 이미지 미확인 범위, 공식 기술 출처는 상위 교재에 있습니다. 첨부 저장 및 연결은 성공했으나 download-attachment는 1회 재시도 후에도 빈 객체를 반환했습니다. 따라서 이 네이티브 전체 소스 페이지를 재시작 가능한 보존 경로로 사용합니다.
