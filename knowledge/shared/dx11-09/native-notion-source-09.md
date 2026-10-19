<callout icon="📦" color="gray_bg">
	**DX11 09A · 전체 CPU 소스와 실행 증거**
	<mention-date start="2026-10-16"/>. 독립 C++20 학습·C++11 호환 CPU 모델. 실제 Assimp SDK·GPU를 호출하지 않는다는 점을 분리합니다.
</callout>
C++20 learning default; same full source built GCC C++11. Does NOT link/load real Assimp or call D3D11/GPU.
## main.cpp
```c++
// Assimp aiScene/aiMesh/aiNode contracts modeled with plain C++11. NOT linked to Assimp or GPU.
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
struct V{double x,y,z;};
V operator+(V a,V b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
struct Material{std::string diffuseName;};
struct Mesh{std::vector<V> vertices;std::vector<std::uint32_t> indices;std::size_t material;bool hasNormals;};
struct Node{std::string name;V translation;std::vector<std::size_t> meshes;std::vector<Node> children;};
struct Scene{std::vector<Mesh> meshes;std::vector<Material> materials;Node root;};
struct Draw{std::size_t meshIndex,materialIndex;V origin;};
void traverse(const Scene& scene,const Node& node,V parent,std::vector<Draw>& out,std::size_t depth){
 if(depth>256)throw std::length_error("node depth limit");
 const V world=parent+node.translation;
 if(!std::isfinite(world.x)||!std::isfinite(world.y)||!std::isfinite(world.z))throw std::invalid_argument("nonfinite node");
 for(std::size_t i:node.meshes){
  if(i>=scene.meshes.size())throw std::out_of_range("node mesh ref");
  const Mesh& m=scene.meshes[i];
  if(m.material>=scene.materials.size())throw std::out_of_range("material index");
  if(m.vertices.empty()||m.indices.empty()||m.indices.size()%3)throw std::invalid_argument("triangle list");
  for(const V& p:m.vertices)if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z))throw std::invalid_argument("vertex finite");
  for(std::uint32_t x:m.indices)if(x>=m.vertices.size())throw std::out_of_range("vertex index");
  out.push_back({i,m.material,world});
 }
 for(const Node& c:node.children)traverse(scene,c,world,out,depth+1);
}
std::vector<Draw> collect(const Scene& s){std::vector<Draw> out;traverse(s,s.root,{0,0,0},out,0);return out;}
bool near(double x,double y){return std::isfinite(x)&&std::isfinite(y)&&std::fabs(x-y)<1e-9;}
int main(){unsigned n=0;auto ck=[&](bool b,const char* label){++n;if(!b)throw std::runtime_error(label);};
 Mesh a{{{0,0,0},{1,0,0},{0,1,0}},{0,1,2},2,true};
 Mesh b{{{0,0,0},{1,0,0},{0,0,1}},{0,1,2},0,false};
 Scene scene{{a,b},{{"steel"},{"unused"},{"wood"}}, {"root",{10,0,0},{0},{{"child",{0,5,0},{1,0},{}}}}};
 ck(scene.materials.size()==3,"three materials");ck(scene.meshes.size()==2,"two meshes");
 auto draws=collect(scene);ck(draws.size()==3,"shared mesh referenced twice");
 ck(draws[0].meshIndex==0,"root mesh");ck(draws[0].materialIndex==2,"material not mesh-index");
 ck(draws[1].meshIndex==1,"child mesh1");ck(draws[1].materialIndex==0,"child material0");
 ck(draws[2].meshIndex==0,"child references same mesh");ck(draws[2].materialIndex==2,"shared material still index2");
 ck(near(draws[0].origin.x,10),"root transform");ck(near(draws[1].origin.x,10),"child parent x");
 ck(near(draws[1].origin.y,5),"child relative y");ck(near(draws[2].origin.y,5),"second child draw origin");
 ck(scene.meshes[1].hasNormals==false,"optional normals absent");
 bool caught=false;try{auto bad=scene;bad.root.meshes={99};collect(bad);}catch(const std::out_of_range&){caught=true;}ck(caught,"bad mesh index");
 caught=false;try{auto bad=scene;bad.meshes[0].material=4;collect(bad);}catch(const std::out_of_range&){caught=true;}ck(caught,"bad material index");
 caught=false;try{auto bad=scene;bad.meshes[0].indices={0,1,99};collect(bad);}catch(const std::out_of_range&){caught=true;}ck(caught,"bad vertex index");
 caught=false;try{auto bad=scene;bad.meshes[0].indices={0,1};collect(bad);}catch(const std::invalid_argument&){caught=true;}ck(caught,"non-triangulated face rejected");
 caught=false;try{auto bad=scene;bad.meshes[0].vertices.clear();collect(bad);}catch(const std::invalid_argument&){caught=true;}ck(caught,"empty mesh");
 caught=false;try{auto bad=scene;bad.meshes[0].vertices[0].x=std::numeric_limits<double>::quiet_NaN();collect(bad);}catch(const std::invalid_argument&){caught=true;}ck(caught,"nan vertex");
 caught=false;try{auto bad=scene;bad.root.translation.x=std::numeric_limits<double>::infinity();collect(bad);}catch(const std::invalid_argument&){caught=true;}ck(caught,"infinite world transform");
 caught=false;try{auto bad=scene;Node* cursor=&bad.root;for(int i=0;i<258;++i){cursor->children.push_back({"nested",{0,0,0},{},{}});cursor=&cursor->children.back();}collect(bad);}catch(const std::length_error&){caught=true;}ck(caught,"max recursion depth");
 // Optional channels do not imply absent mesh. Fallback is an engine policy.
 ck(!scene.meshes[1].hasNormals && scene.meshes[1].indices.size()==3,"normal fallback needed");
 // Node transform is applied per reference, not baked permanently into shared mesh.
 ck(near(scene.meshes[0].vertices[0].x,0),"source mesh vertices unchanged");
 ck(draws[0].materialIndex!=draws[0].meshIndex,"materialIndex independent of meshIndex");
 ck(draws[0].origin.y!=draws[2].origin.y,"different node world for same mesh");
 ck(draws[0].meshIndex==draws[2].meshIndex,"reuse mesh storage");
 std::cout<<"PASS "<<n<<" Assimp-like scene CPU checks\n";
}

```
## CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_assimp_scene_cpu LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 default, 11 compatible")
add_executable(dx11_09_cpu main.cpp)
set_target_properties(dx11_09_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_09_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_09_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME dx11_09_cpu COMMAND dx11_09_cpu)

```
## Actual commands
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
## Verified results
GCC C++20 / GCC C++11 / Clang C++20 ASan+UBSan: same 27 CPU checks, each CTest 1/1; Windows/MSVC, real Assimp ReadFile, D3D11, GPU, HLSL: NOT EXECUTED.
## gcc20 full CTest stdout
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch09_mesh/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_batch09_mesh/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch09_mesh/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_batch09_mesh/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_09_cpu

1: Test command: /mnt/data/dx11_batch09_mesh/build-gcc20/dx11_09_cpu
1: Working Directory: /mnt/data/dx11_batch09_mesh/build-gcc20
1: Test timeout computed to be: 10000000
1: PASS 27 Assimp-like scene CPU checks
1/1 Test #1: dx11_09_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.02 sec

```
## gcc11 full CTest stdout
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch09_mesh/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_batch09_mesh/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch09_mesh/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_batch09_mesh/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_09_cpu

1: Test command: /mnt/data/dx11_batch09_mesh/build-gcc11/dx11_09_cpu
1: Working Directory: /mnt/data/dx11_batch09_mesh/build-gcc11
1: Test timeout computed to be: 10000000
1: PASS 27 Assimp-like scene CPU checks
1/1 Test #1: dx11_09_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec

```
## clangsan full CTest stdout
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch09_mesh/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_batch09_mesh/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch09_mesh/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_batch09_mesh/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_09_cpu

1: Test command: /mnt/data/dx11_batch09_mesh/build-clangsan/dx11_09_cpu
1: Working Directory: /mnt/data/dx11_batch09_mesh/build-clangsan
1: Test timeout computed to be: 10000000
1: PASS 27 Assimp-like scene CPU checks
1/1 Test #1: dx11_09_cpu ......................   Passed    0.09 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.10 sec

```
## Source file hashes
```plain text
main.cpp SHA256 ba5e091d4679f42de15aac3633c1997fdf165e5bda1338d03b33471105a36a09
CMakeLists.txt SHA256 6a9a5de86d3298adcac2679d058f474d7b020ad5da60d3ff692375428e5f1b56
```
