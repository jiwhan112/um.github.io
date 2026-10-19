<callout icon="📦" color="gray_bg">
	**DX11 10A · 전체 C++11 호환 소스/CTest 기록**
	<mention-date start="2026-10-16"/>. CPU 학습 모델. 실제 Assimp/Windows/D3D11/GPU/HLSL 미실행을 구분합니다.
</callout>
Independent CPU-only teaching model for key interpolation, quaternion slerp, hierarchy, inverse bind and weights. Real Assimp, Windows/MSVC, D3D11, HLSL, GPU: NOT RUN.
## main.cpp
```c++
// CPU-only skeletal animation: quaternion interpolation, hierarchy, skin weights.
// Assimp ReadFile, Windows Direct3D11, HLSL skinning NOT executed.
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct V {double x,y,z;};
V operator+(V a,V b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V operator*(V a,double s){return {a.x*s,a.y*s,a.z*s};}
bool finite(V v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
double dot(V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;}
struct Q {double w,x,y,z;};
double dot(Q a,Q b){return a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z;}
Q normalized(Q q){double l=std::sqrt(dot(q,q));if(!(l>1e-12)||!std::isfinite(l))throw std::invalid_argument("bad quaternion");return {q.w/l,q.x/l,q.y/l,q.z/l};}
Q conj(Q q){return {q.w,-q.x,-q.y,-q.z};}
Q multiply(Q a,Q b){return {a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z,a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w};}
V rotate(Q q,V v){q=normalized(q);const Q p=multiply(multiply(q,{0,v.x,v.y,v.z}),conj(q));return {p.x,p.y,p.z};}
Q slerp(Q a,Q b,double t){
 if(!std::isfinite(t)||t<0||t>1)throw std::invalid_argument("time range");
 a=normalized(a);b=normalized(b);double d=dot(a,b);
 if(d<0){b={-b.w,-b.x,-b.y,-b.z};d=-d;}
 Q q{};
 if(d>.9995)q={a.w+(b.w-a.w)*t,a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.z+(b.z-a.z)*t};
 else{d=std::max(-1.0,std::min(1.0,d));double ang=std::acos(d),sinAng=std::sin(ang);
  const double s0=std::sin((1-t)*ang)/sinAng,s1=std::sin(t*ang)/sinAng;
  q={a.w*s0+b.w*s1,a.x*s0+b.x*s1,a.y*s0+b.y*s1,a.z*s0+b.z*s1};}
 return normalized(q);
}
struct Pose{V p;Q r;};
Pose compose(Pose parent,Pose child){return {parent.p+rotate(parent.r,child.p),normalized(multiply(parent.r,child.r))};}
V transform(Pose p,V x){return p.p+rotate(p.r,x);}
struct Key {double tick;Pose pose;};
Pose sample(const std::vector<Key>& keys,double time){
 if(keys.empty()||!std::isfinite(time))throw std::invalid_argument("keys/time");
 for(std::size_t i=0;i<keys.size();++i){if(!std::isfinite(keys[i].tick)||keys[i].tick<0||!finite(keys[i].pose.p))throw std::invalid_argument("invalid key");if(i&&!(keys[i].tick>keys[i-1].tick))throw std::invalid_argument("unsorted keys");(void)normalized(keys[i].pose.r);}
 if(time<=keys.front().tick)return keys.front().pose;
 if(time>=keys.back().tick)return keys.back().pose;
 for(std::size_t i=1;i<keys.size();++i)if(time<=keys[i].tick){
  const double t=(time-keys[i-1].tick)/(keys[i].tick-keys[i-1].tick);
  const V a=keys[i-1].pose.p,b=keys[i].pose.p;
  return {a*(1-t)+b*t,slerp(keys[i-1].pose.r,keys[i].pose.r,t)};
 }
 throw std::logic_error("unreachable");
}
struct Clip{double durationTicks,ticksPerSecond;std::vector<Key> keys;};
Pose atSeconds(const Clip& clip,double seconds){
 if(!(clip.durationTicks>0)||!(clip.ticksPerSecond>0)||!std::isfinite(clip.durationTicks)||!std::isfinite(clip.ticksPerSecond)||!std::isfinite(seconds)||seconds<0)throw std::invalid_argument("invalid clip");
 double tick=std::fmod(seconds*clip.ticksPerSecond,clip.durationTicks);
 if(!std::isfinite(tick))throw std::invalid_argument("tick overflow");
 return sample(clip.keys,tick);
}
struct Influence {std::size_t bone;double weight;};
V skin(V vertex,const std::vector<Pose>& boneWorld,const std::vector<Pose>& inverseBind,const std::vector<Influence>& weights){
 if(!finite(vertex)||boneWorld.size()!=inverseBind.size()||weights.empty()||weights.size()>4)throw std::invalid_argument("skin layout");
 V out{0,0,0};double total=0;
 for(auto i:weights){if(i.bone>=boneWorld.size()||!std::isfinite(i.weight)||i.weight<0)throw std::invalid_argument("bad influence");
  out=out+transform(boneWorld[i.bone],transform(inverseBind[i.bone],vertex))*i.weight; total+=i.weight;}
 if(!std::isfinite(total)||std::abs(total-1)>1e-6)throw std::invalid_argument("weight sum");
 return out;
}
bool near(double a,double b){return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<1e-8;}
int main(){unsigned n=0;auto ck=[&](bool ok,const char* why){++n;if(!ok)throw std::runtime_error(why);};
 const Q I{1,0,0,0};const double pi=std::acos(-1.0);
 const Q half{std::cos(pi/4),0,0,std::sin(pi/4)},turn{0,0,0,1};
 auto v=rotate(I,{1,0,0});ck(near(v.x,1),"identity rot");ck(near(v.y,0),"identity y");
 v=rotate(half,{1,0,0});ck(near(v.x,0),"quarter x");ck(near(v.y,1),"quarter y");
 const auto q=slerp(I,turn,.5);v=rotate(q,{1,0,0});ck(near(v.y,1),"slerp half");
 ck(near(dot(q,q),1),"slerp normalized");ck(near(slerp(I,turn,0).w,1),"slerp start");
 ck(near(slerp(I,turn,1).z,1),"slerp end");
 const Q antipodal{-1,0,0,0};ck(near(slerp(I,antipodal,.5).w,1),"shortest arc sign");
 const Pose a{{10,0,0},I},b{{0,2,0},I};Pose composed=compose(a,b);
 ck(near(composed.p.x,10),"parent x");ck(near(composed.p.y,2),"child y");
 composed=compose({{10,0,0},half},{{1,0,0},I});ck(near(composed.p.x,10),"rotated child x");ck(near(composed.p.y,1),"rotated child y");
 std::vector<Key> keys{{0,{{0,0,0},I}},{10,{{10,0,0},turn}}};
 auto p=sample(keys,5);ck(near(p.p.x,5),"linear translation");v=rotate(p.r,{1,0,0});ck(near(v.y,1),"quaternion mid");
 ck(near(sample(keys,-1).p.x,0),"clamp before");ck(near(sample(keys,11).p.x,10),"clamp after");
 Clip clip{10,10,keys};ck(near(atSeconds(clip,.5).p.x,5),"seconds to ticks");
 ck(near(atSeconds(clip,1.5).p.x,5),"wrap animation");
 ck(near(atSeconds(clip,2.0).p.x,0),"wrap zero");
 const std::vector<Pose> world={{{0,0,0},half}},bind={{{0,0,0},I}};
 v=skin({1,0,0},world,bind,{{0,1}});ck(near(v.x,0),"skin x");ck(near(v.y,1),"skin y");
 std::vector<Pose> twoWorld={{{0,0,0},I},{{2,0,0},I}},twoBind={{{0,0,0},I},{{0,0,0},I}};
 v=skin({0,0,0},twoWorld,twoBind,{{0,.5},{1,.5}});ck(near(v.x,1),"weighted skin");
 auto bad=[&](bool caught){ck(caught,"invalid must reject");};bool e=false;
 try{slerp(I,turn,-.1);}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{slerp({0,0,0,0},turn,.5);}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{sample({},0);}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{sample({keys[1],keys[0]},5);}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{atSeconds({10,0,keys},.5);}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{atSeconds(clip,-.5);}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{skin({0,0,0},world,bind,{{2,1}});}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{skin({0,0,0},world,bind,{{0,.8}});}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{skin({0,0,0},world,bind,{{0,-.1},{0,1.1}});}catch(const std::invalid_argument&){e=true;}bad(e);
 e=false;try{skin({0,0,0},world,bind,{{0,.25},{0,.25},{0,.25},{0,.15},{0,.1}});}catch(const std::invalid_argument&){e=true;}bad(e);
 std::cout<<"PASS "<<n<<" CPU skeletal animation checks\n";
}

```
## CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_animation_cpu LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 default, 11 compatible")
add_executable(dx11_10_cpu main.cpp)
set_target_properties(dx11_10_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_10_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_10_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME dx11_10_cpu COMMAND dx11_10_cpu)

```
## Commands
```bash
cmake -S . -B build-gcc20 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DLAB_CXX_STANDARD=20
cmake --build build-gcc20 -j 2
ctest --test-dir build-gcc20 --output-on-failure -V
cmake -S . -B build-gcc11 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DLAB_CXX_STANDARD=11
cmake --build build-gcc11 -j 2
ctest --test-dir build-gcc11 --output-on-failure -V
cmake -S . -B build-clangsan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DLAB_CXX_STANDARD=20 -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=undefined" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-clangsan -j 2
ctest --test-dir build-clangsan --output-on-failure -V
```
GCC 14.2 C++20 / GCC 14.2 C++11 / Clang17 ASan+UBSan: the SAME 33 CPU checks; each CTest 1/1.
## gcc20 raw CTest output
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch10_animation/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_batch10_animation/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch10_animation/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_batch10_animation/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_10_cpu

1: Test command: /mnt/data/dx11_batch10_animation/build-gcc20/dx11_10_cpu
1: Working Directory: /mnt/data/dx11_batch10_animation/build-gcc20
1: Test timeout computed to be: 10000000
1: PASS 33 CPU skeletal animation checks
1/1 Test #1: dx11_10_cpu ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec

```
## gcc11 raw CTest output
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch10_animation/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_batch10_animation/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch10_animation/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_batch10_animation/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_10_cpu

1: Test command: /mnt/data/dx11_batch10_animation/build-gcc11/dx11_10_cpu
1: Working Directory: /mnt/data/dx11_batch10_animation/build-gcc11
1: Test timeout computed to be: 10000000
1: PASS 33 CPU skeletal animation checks
1/1 Test #1: dx11_10_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec

```
## clangsan raw CTest output
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch10_animation/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_batch10_animation/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch10_animation/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_batch10_animation/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_10_cpu

1: Test command: /mnt/data/dx11_batch10_animation/build-clangsan/dx11_10_cpu
1: Working Directory: /mnt/data/dx11_batch10_animation/build-clangsan
1: Test timeout computed to be: 10000000
1: PASS 33 CPU skeletal animation checks
1/1 Test #1: dx11_10_cpu ......................   Passed    0.03 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.04 sec

```
## SHA-256 source bytes
```plain text
main.cpp 3c4df314bb9cb5ebf4d48e988ba9e8b78a8313c0e688159caf2e1a9d1c100ee9
CMakeLists.txt d7d02ed803e9a796c640983c542c13d6ef9a9886ee1ef5a1827f0f5f3b107a1a
```
