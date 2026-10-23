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
void traverse(const Scene& scene,const Node& node,V parent,std::vector<Draw>& out,std::size_t depth){ // [한국어 해설] 노드 참조마다 누적 이동을 적용한다. 공유 mesh 정점에 변환을 영구적으로 굽지 않는다.
 if(depth>256)throw std::length_error("node depth limit");
 const V world=parent+node.translation; // [한국어 해설] 이 축약 모델은 이동만 지원한다. 실제 Assimp의 전체 행렬·회전·스케일을 재현하지 않는다.
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
std::vector<Draw> collect(const Scene& s){std::vector<Draw> out;traverse(s,s.root,{0,0,0},out,0);return out;} // [한국어 해설] 지역 결과에 모으므로 실패 결과를 반환하지 않는다. 외부 vector를 받는 traverse의 부분 추가는 자동 롤백되지 않는다.
bool near(double x,double y){return std::isfinite(x)&&std::isfinite(y)&&std::fabs(x-y)<1e-9;}
int main(){unsigned n=0;auto ck=[&](bool b,const char* label){++n;if(!b)throw std::runtime_error(label);}; // [한국어 해설] mesh 인덱스와 material 인덱스의 독립성, 공유 참조·누락 채널·깊이 제한을 검사한다.
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
