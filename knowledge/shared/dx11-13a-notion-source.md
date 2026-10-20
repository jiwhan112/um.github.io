<callout icon="📦" color="blue_bg">
	**13A · DX11 Instancing 실습 보관·검증 색인**
	상위 <mention-page url="https://app.notion.com/p/3fba99703f4081c6b435d946bcb40dc2">13 · Instancing — IA Slot·StepRate·DrawIndexedInstanced·Dynamic VB</mention-page>. DX11 InputLayout 슬롯 0/1, StepRate=1, DrawIndexedInstanced 범위 검사와 Dynamic buffer NO_OVERWRITE 갱신 계약을 대상으로 합니다.
</callout>
## 이전 실행에서 실제 검증한 자료
- GCC 14.2 C++20: 37 CPU 조건, CTest 1/1.
- GCC 14.2 C++11: 같은 37 CPU 조건, CTest 1/1.
- Clang 17 C++20 ASan/UBSan: 같은 37 CPU 조건, CTest 1/1.
- main.cpp SHA-256: `fd3839f7c350d1e573cab36a8c3f942f55a8e6c9d4e94b45033e7d3b3b4472ae`.
- CMakeLists.txt SHA-256: `1ba7395b7b9201cf982fa1059a41a70edef99fbd4aa93743088b4dc43a3c3ed2`.
## 개정 범위
`D3D11_INPUT_PER_INSTANCE_DATA`는 GPU의 별도 vertex input stream입니다. CPU 모델은 `Layout`, `Draw`, `DynamicInstances`의 부분집합에 대한 규칙만 확인합니다. `WRITE_NO_OVERWRITE`가 이미 GPU가 읽고 있는 데이터에 덮어써도 된다는 의미는 아닙니다.
## 보관 범위 주의
**2026-10-18 14시 재검수:** 이 페이지에는 현재 `main.cpp` 코드 조각 4개, 전체 CMakeLists.txt 1개, GCC C++20·C++11·Clang ASan/UBSan의 원시 `CTest -V` 3개가 실제 존재합니다. CMake를 네이티브 블록에서 재조립한 SHA-256은 기록값 `1ba7395b...3c3ed2`와 일치합니다. 그러나 C++ 소스 네 블록을 순서대로 합쳐 각 경계에 0\~2개의 줄바꿈, 마지막에 0\~2개의 줄바꿈을 시험한 81개 조합 모두 기록된 SHA-256 `fd3839f7...4472ae`와 불일치했습니다. 따라서 **전체 원본 소스의 무결성은 미검증**이며, configure/build 원시 로그의 Notion 보관도 확인되지 않아 13A를 완결로 집계하지 않습니다. CTest 로그의 과거 성공 기록을 이번 실행의 재컴파일·재실행으로 오인하지 않습니다. Windows/MSVC·D3D11/GPU 실기 검증은 미완료입니다.
## main.cpp · 전문 1/3 (연결 시 위·아래 코드 블록을 순서대로 합침)
```c++
// CPU-only D3D11 instancing contract, not a D3D11 runtime implementation.
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct Element{unsigned slot,offset,bytes,stepRate;bool perInstance;};
struct Layout{unsigned vertexStride,instanceStride;std::vector<Element> elements;};
void checkLayout(const Layout& x){
 if(!x.vertexStride||!x.instanceStride||x.elements.empty())throw std::invalid_argument("empty layout");
 for(std::size_t i=0;i<x.elements.size();++i){const Element a=x.elements[i];
  const unsigned stride=a.slot==0?x.vertexStride:a.slot==1?x.instanceStride:0;
  if(!stride||!a.bytes||a.offset>stride||a.bytes>stride-a.offset||a.perInstance!=(a.slot==1)||a.stepRate!=(a.perInstance?1u:0u))throw std::invalid_argument("invalid element");
  for(std::size_t j=i+1;j<x.elements.size();++j){const Element b=x.elements[j];
   if(a.slot==b.slot && a.offset < b.offset+b.bytes && b.offset < a.offset+a.bytes)throw std::invalid_argument("overlap");
  }
 }
}
struct Draw{std::vector<unsigned> indices;std::uint64_t vertexCount,instanceCapacity;};
void checkDraw(const Draw& d,std::uint64_t count,std::uint64_t instanceCount,std::uint64_t startIndex,std::int64_t baseVertex,std::uint64_t startInstance){
 if(startIndex>d.indices.size()||count>d.indices.size()-startIndex||startInstance>d.instanceCapacity||instanceCount>d.instanceCapacity-startInstance)throw std::out_of_range("draw span");
 if(count==0||instanceCount==0)return;
 for(std::uint64_t i=startIndex;i<startIndex+count;++i){const std::uint64_t index=d.indices[static_cast<std::size_t>(i)];
  if(baseVertex<0){const std::uint64_t magnitude=static_cast<std::uint64_t>(-(baseVertex+1))+1;
    if(index<magnitude || index-magnitude>=d.vertexCount)throw std::out_of_range("baseVertex underflow");
  }else{const std::uint64_t base=static_cast<std::uint64_t>(baseVertex);
    if(base>=d.vertexCount||index>=d.vertexCount-base)throw std::out_of_range("baseVertex overflow");
  }
 }
}
struct Span{unsigned first,count;};
struct DynamicInstances {
 unsigned capacity,cursor=0;std::vector<Span> inFlight;
 explicit DynamicInstances(unsigned n):capacity(n){if(!n)throw std::invalid_argument("capacity zero");}
 void discard(){cursor=0;inFlight.clear();}
 unsigned reserveNoOverwrite(unsigned n){if(n==0||n>capacity||cursor>capacity-n)throw std::out_of_range("reserve size");
  for(const Span r:inFlight)if(cursor<r.first+r.count && r.first<cursor+n)throw std::logic_error("GPU overlap");
  const unsigned start=cursor;cursor+=n;return start;
 }
 void submit(unsigned start,unsigned n){if(!n||start>capacity||n>capacity-start||start+n>cursor)throw std::out_of_range("submit invalid");
   inFlight.push_back({start,n});
 }
};
```
## main.cpp · 전문 2/3
```c++
int main(){unsigned checks=0;auto ck=[&](bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);};
 auto reject=[&](bool ok){ck(ok,"must reject");};
 const Layout standard{20,64,{{0,0,12,0,false},{0,12,8,0,false},{1,0,16,1,true},{1,16,16,1,true},{1,32,16,1,true},{1,48,16,1,true}}};
 checkLayout(standard);ck(true,"valid layout");
 bool bad=false;try{checkLayout({20,64,{{0,0,12,1,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{1,0,16,0,true}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{2,0,16,1,true}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{0,8,16,0,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{0,0,12,0,false},{0,8,8,0,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{0,0,0,0,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 Draw d{{0,1,2,2,3,0},4,100};
 checkDraw(d,6,12,0,0,0);ck(true,"draw valid");
 checkDraw(d,3,10,3,0,90);ck(true,"draw subrange");
 checkDraw(d,0,5,0,0,0);ck(true,"zero index count is no-op");
 checkDraw(d,6,0,0,0,0);ck(true,"zero instance count is no-op");
 auto drawReject=[&](std::uint64_t c,std::uint64_t ins,std::uint64_t begin,std::int64_t base,std::uint64_t first){
  bool caught=false;try{checkDraw(d,c,ins,begin,base,first);}catch(const std::out_of_range&){caught=true;}reject(caught);
 };
 drawReject(7,1,0,0,0);drawReject(1,1,6,0,0);drawReject(1,1,7,0,0);
 drawReject(1,1,0,0,100);drawReject(1,2,0,0,99);
 drawReject(6,1,0,1,0);drawReject(6,1,0,-1,0);
 drawReject(1,1,0,std::numeric_limits<std::int64_t>::min(),0);
 drawReject(1,1,0,std::numeric_limits<std::int64_t>::max(),0);
 drawReject(6,std::numeric_limits<std::uint64_t>::max(),0,0,1);
 const Draw offset{{1,2,3},3,1};checkDraw(offset,3,1,0,-1,0);ck(true,"negative base valid with safe offset");
```
## main.cpp · 전문 3a/3c
```c++
DynamicInstances ring(8);ck(ring.reserveNoOverwrite(3)==0,"first reserve");ring.submit(0,3);ck(ring.inFlight.size()==1,"submit GPU range");
ck(ring.reserveNoOverwrite(2)==3,"second reserve");ring.submit(3,2);ck(ring.inFlight.size()==2,"second submit");
ck(ring.reserveNoOverwrite(3)==5,"last reserve");
bool ex=false;try{ring.reserveNoOverwrite(1);}catch(const std::out_of_range&){ex=true;}reject(ex);
ex=false;try{ring.reserveNoOverwrite(0);}catch(const std::out_of_range&){ex=true;}reject(ex);
ex=false;try{ring.submit(8,1);}catch(const std::out_of_range&){ex=true;}reject(ex);
ex=false;try{ring.submit(7,0);}catch(const std::out_of_range&){ex=true;}reject(ex);
```
## main.cpp · 전문 3b/3c
```c++
 ex=false;try{ring.submit(10,1);}catch(const std::out_of_range&){ex=true;}reject(ex);
 ring.discard();ck(ring.cursor==0&&ring.inFlight.empty(),"discard models rename");
 ck(ring.reserveNoOverwrite(8)==0,"full range after discard");ring.submit(0,8);
 ex=false;try{ring.reserveNoOverwrite(1);}catch(const std::out_of_range&){ex=true;}reject(ex);
 ex=false;try{DynamicInstances empty(0);}catch(const std::invalid_argument&){ex=true;}reject(ex);
 std::cout<<"PASS "<<checks<<" DX11 instancing CPU contract checks\n";
}
```
## CMakeLists.txt (전문)
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_13_instancing_cpu LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 or 11")
enable_testing()
add_executable(dx11_13_cpu main.cpp)
set_target_properties(dx11_13_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_13_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_13_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME dx11_13_cpu COMMAND dx11_13_cpu)
```
## 13A · GCC C++20 원시 CTest -V
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch13_instancing/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_batch13_instancing/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch13_instancing/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_batch13_instancing/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13_cpu

1: Test command: /mnt/data/dx11_batch13_instancing/build-gcc20/dx11_13_cpu
1: Working Directory: /mnt/data/dx11_batch13_instancing/build-gcc20
1: Test timeout computed to be: 10000000
1: PASS 37 DX11 instancing CPU contract checks
1/1 Test #1: dx11_13_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```
## 13A · GCC C++11 원시 CTest -V
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch13_instancing/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_batch13_instancing/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch13_instancing/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_batch13_instancing/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13_cpu

1: Test command: /mnt/data/dx11_batch13_instancing/build-gcc11/dx11_13_cpu
1: Working Directory: /mnt/data/dx11_batch13_instancing/build-gcc11
1: Test timeout computed to be: 10000000
1: PASS 37 DX11 instancing CPU contract checks
1/1 Test #1: dx11_13_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```
## 13A · Clang C++20 ASan/UBSan 원시 CTest -V
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch13_instancing/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_batch13_instancing/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch13_instancing/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_batch13_instancing/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13_cpu

1: Test command: /mnt/data/dx11_batch13_instancing/build-clangsan/dx11_13_cpu
1: Working Directory: /mnt/data/dx11_batch13_instancing/build-clangsan
1: Test timeout computed to be: 10000000
1: PASS 37 DX11 instancing CPU contract checks
1/1 Test #1: dx11_13_cpu ......................   Passed    0.04 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.04 sec
```
<page url="https://app.notion.com/p/3fda99703f4081d5ae1ac37e1b558bd0">13A 재현 가능 CPU 검증 · 현재 6043B 소스·GCC11/20·ClangSan 원시 로그 9개 · 2026-10-18</page>
<page url="https://app.notion.com/p/3fda99703f408150bcb6ceb1602850a9">13B1 · InputLayout 정수 경계·2-pass 검증 CPU 전체 main.cpp · 5681B</page>
<page url="https://app.notion.com/p/3fda99703f4081cf9e53d91efafce3b1">13B2 · InputLayout CMakeLists 전체 · C++11/20 빌드 지침</page>
<page url="https://app.notion.com/p/3fda99703f4081fa822bf7fa2923c702">13B3 · gcc20 실제 Configure·Build·CTest 원시 로그 · CTest 1/1</page>
<page url="https://app.notion.com/p/3fda99703f408135b96ac0a0bae7334b">13B3 · gcc11 실제 Configure·Build·CTest 원시 로그 · CTest 1/1</page>
<page url="https://app.notion.com/p/3fda99703f408182b510f65f37811759">13B3 · clangsan 실제 Configure·Build·CTest 원시 로그 · CTest 1/1</page>
<page url="https://app.notion.com/p/3fda99703f4081acaea2cdd358c5e39e">13B · Instancing InputLayout 요소 경계 검증 순서 · 2-pass CPU 실습</page>
