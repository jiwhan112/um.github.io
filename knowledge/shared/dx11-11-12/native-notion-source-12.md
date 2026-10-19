<callout icon="📦" color="gray_bg">
	**DX11 12A · NavMesh 검증 소스 보관** — 상위 교재 <mention-page url="https://app.notion.com/p/3fba99703f408107ba7cf3679dbe98d8">12 · Navigation Mesh — Triangle Cell·인접 그래프·A\*</mention-page>. 기존 12_navmesh/main.cpp의 전체 본문과 CMake, 검증 기록을 이 하위 페이지에 누적합니다. 2D XZ/공유 정점 인덱스 기반의 CPU 모델이며 실제 Recast/Detour API가 아닙니다.
</callout>
## CMakeLists.txt
```plain text
add_executable(dx11_12_cpu main.cpp)
set_target_properties(dx11_12_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_12_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_12_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME dx11_12_cpu COMMAND dx11_12_cpu)
```
## 상위 CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_collision_navmesh LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 default / 11 compatible")
enable_testing()
add_subdirectory(11_collision)
add_subdirectory(12_navmesh)
```
## 실제 검증 로그 요약
```plain text
GCC14.2 C++20: PASS 27 navigation mesh CPU checks, CTest 배치 2/2
GCC14.2 C++11: PASS 27 navigation mesh CPU checks, CTest 배치 2/2
Clang17 C++20 ASan/UBSan: PASS 27 navigation mesh CPU checks, CTest 배치 2/2
```
원본 main.cpp SHA-256: `249d93eedcc26993afcd28b58592fddf38ad974ae28334bc4628f0060f41666f`. 다음에 오는 코드 Part 1·Part 2는 **순서대로 이어 붙여** main.cpp로 사용할 수 있습니다. 원시 로그 및 완결 소스는 별도 대화 ZIP에서도 제공됩니다. Windows/MSVC·D3D11·Recast 미검증.
## main.cpp Part 1/2
이 코드 블록과 Part 2의 내용 사이에 빈 줄을 추가하지 않고 이어 붙여 하나의 `main.cpp` 파일로 사용합니다.
```c++
// CPU-only 2D XZ navigation mesh graph & A* triangle path. No funnel, agent radius or physics.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>
struct P{double x,z;};
struct Tri{std::size_t a,b,c;};
static double cross(P a,P b,P c){return (b.x-a.x)*(c.z-a.z)-(b.z-a.z)*(c.x-a.x);}
static bool finite(P p){return std::isfinite(p.x)&&std::isfinite(p.z);}
struct NavMesh {
 std::vector<P> points;std::vector<Tri> tris;std::vector<std::vector<std::size_t>> adj;
 NavMesh(std::vector<P> p,std::vector<Tri> t):points(std::move(p)),tris(std::move(t)){
  if(points.empty()||tris.empty())throw std::invalid_argument("empty mesh");
  for(P v:points)if(!finite(v))throw std::invalid_argument("nonfinite vertex");
  for(auto x:tris){if(x.a>=points.size()||x.b>=points.size()||x.c>=points.size()||x.a==x.b||x.b==x.c||x.c==x.a)throw std::invalid_argument("invalid indices");
   if(std::fabs(cross(points[x.a],points[x.b],points[x.c]))<1e-12)throw std::invalid_argument("degenerate triangle");}
  adj.resize(tris.size());
  for(std::size_t i=0;i<tris.size();++i)for(std::size_t j=i+1;j<tris.size();++j){
    const auto a=tris[i],b=tris[j];const std::size_t va[]={a.a,a.b,a.c},vb[]={b.a,b.b,b.c};unsigned shared=0;
    for(auto x:va)for(auto y:vb)if(x==y)++shared;
    if(shared==3)throw std::invalid_argument("duplicate triangle");
    if(shared==2){adj[i].push_back(j);adj[j].push_back(i);}
  }
 }
 bool contains(std::size_t ti,P p)const{
  if(ti>=tris.size()||!finite(p))return false;
  const auto x=tris[ti];const double area=cross(points[x.a],points[x.b],points[x.c]);
  const double u=cross(p,points[x.b],points[x.c])/area;
  const double v=cross(points[x.a],p,points[x.c])/area;
  const double w=1-u-v;
  return u>=-1e-10&&v>=-1e-10&&w>=-1e-10;
 }
 std::size_t locate(P p)const{if(!finite(p))throw std::invalid_argument("nonfinite query");
  for(std::size_t i=0;i<tris.size();++i)if(contains(i,p))return i;
  throw std::out_of_range("point outside navmesh");
 }
 P center(std::size_t i)const{const auto t=tris.at(i);return {(points[t.a].x+points[t.b].x+points[t.c].x)/3,(points[t.a].z+points[t.b].z+points[t.c].z)/3};}
 double distance(std::size_t i,std::size_t j)const{P a=center(i),b=center(j);return std::hypot(a.x-b.x,a.z-b.z);}
```
## main.cpp Part 2/2
```c++
 std::vector<std::size_t> trianglePath(P start,P goal)const{
  const std::size_t s=locate(start),g=locate(goal);
  if(s==g)return {s};
  const double inf=std::numeric_limits<double>::infinity();
  std::vector<double> dist(tris.size(),inf);
  std::vector<std::size_t> prev(tris.size(),tris.size());
  using Pair=std::pair<double,std::size_t>;
  std::priority_queue<Pair,std::vector<Pair>,std::greater<Pair>> open;
  dist[s]=0;open.push({distance(s,g),s});
  while(!open.empty()){
   const std::size_t i=open.top().second;const double f=open.top().first;open.pop();
   if(f>dist[i]+distance(i,g)+1e-10)continue;
   if(i==g)break;
   for(std::size_t j:adj[i]){
    const double candidate=dist[i]+distance(i,j);
    if(candidate+1e-10<dist[j]){dist[j]=candidate;prev[j]=i;open.push({candidate+distance(j,g),j});}
   }
  }
  if(prev[g]==tris.size())return {};
  std::vector<std::size_t> path;for(std::size_t v=g;;v=prev[v]){path.push_back(v);if(v==s)break;}
  std::reverse(path.begin(),path.end());return path;
 }
};
int main(){unsigned n=0;auto ck=[&](bool ok,const char* name){++n;if(!ok)throw std::runtime_error(name);};
 NavMesh mesh({{0,0},{1,0},{0,1},{1,1},{2,0},{2,1}},{{0,1,2},{1,3,2},{1,4,3},{4,5,3}});
 ck(mesh.tris.size()==4,"4 triangles");ck(mesh.adj[0].size()==1,"start neighbor");
 ck(mesh.adj[1].size()==2,"middle neighbors");ck(mesh.adj[2].size()==2,"other neighbors");
 ck(mesh.adj[3].size()==1,"end neighbor");
 ck(mesh.contains(0,{.1,.1}),"inside first");ck(mesh.contains(0,{0,0}),"touch vertex");
 ck(!mesh.contains(0,{2,2}),"outside");ck(!mesh.contains(99,{0,0}),"bad triangle id");
 ck(mesh.locate({.2,.2})==0,"point locate first");
 ck(mesh.locate({1.8,.8})==3,"point locate end");
 auto path=mesh.trianglePath({.2,.2},{1.8,.8});
 ck(path.size()==4,"path length");ck(path[0]==0,"path begins");ck(path[1]==1,"path via 1");
 ck(path[2]==2,"path via 2");ck(path[3]==3,"path ends");
 path=mesh.trianglePath({.2,.2},{.3,.2});ck(path.size()==1&&path[0]==0,"same triangle path");
 bool e=false;try{mesh.locate({-1,0});}catch(const std::out_of_range&){e=true;}ck(e,"outside locate rejected");
 e=false;try{mesh.trianglePath({-1,0},{1,1});}catch(const std::out_of_range&){e=true;}ck(e,"outside start rejected");
 e=false;try{mesh.locate({std::numeric_limits<double>::quiet_NaN(),0});}catch(const std::invalid_argument&){e=true;}ck(e,"nonfinite query");
 e=false;try{NavMesh bad({{0,0},{1,0},{0,1}},{{0,1,3}});}catch(const std::invalid_argument&){e=true;}ck(e,"bad index");
 e=false;try{NavMesh bad({{0,0},{1,0},{2,0}},{{0,1,2}});}catch(const std::invalid_argument&){e=true;}ck(e,"degenerate triangle");
 e=false;try{NavMesh bad({{0,0},{1,0},{0,1}},{{0,1,2},{0,1,2}});}catch(const std::invalid_argument&){e=true;}ck(e,"duplicate triangle");
 NavMesh disconnected({{0,0},{1,0},{0,1},{10,10},{11,10},{10,11}},{{0,1,2},{3,4,5}});
 ck(disconnected.trianglePath({.2,.2},{10.2,10.2}).empty(),"disconnected path");
 ck(disconnected.adj[0].empty()&&disconnected.adj[1].empty(),"no false adjacency");
 ck(mesh.center(0).x>0&&mesh.center(0).z>0,"centroid inside");
 ck(mesh.distance(0,1)>0,"positive graph edge");
 std::cout<<"PASS "<<n<<" navigation mesh CPU checks\n";
}
```
## 주의
코드는 두 파트를 연결하여 원본 main.cpp로 구성합니다. CPU 모형 통과를 DirectX11 또는 Recast 실제 API 실행 성공으로 해석하지 않습니다.
