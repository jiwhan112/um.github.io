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
