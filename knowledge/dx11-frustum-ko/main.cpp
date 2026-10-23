// CPU-only Direct3D-style frustum plane extraction for clip coordinates 0<=z<=w.
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
struct Vec3 { double x,y,z; };
struct Vec4 { double x,y,z,w; };
struct Matrix { std::array<Vec4,4> r; }; // column vector: clip = M * vec4(world,1) // [한국어 해설] 이 원문은 열 벡터 clip=M*world 규약이다. 다른 단원의 행 벡터 코드와 추출 행/열을 섞지 않는다.
struct Plane { Vec3 n; double d; };
struct AABB { Vec3 lo,hi; };
enum class Visibility { Outside, Intersect, Inside };
Vec4 add(Vec4 a,Vec4 b){return {a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w};}
Vec4 sub(Vec4 a,Vec4 b){return {a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w};}
bool isFinite(double v){return std::isfinite(v);}
std::array<Plane,6> extract(const Matrix& m){ // [한국어 해설] 행을 조합해 -w≤x,y≤w 및 0≤z≤w 평면을 얻고 법선과 d를 함께 정규화한다.
 const Vec4& x=m.r[0];const Vec4& y=m.r[1];const Vec4& z=m.r[2];const Vec4& w=m.r[3];
 const std::array<Vec4,6> rows{{add(w,x),sub(w,x),add(w,y),sub(w,y),z,sub(w,z)}};
 std::array<Plane,6> planes{};
 for(std::size_t i=0;i<6;++i){const Vec4 a=rows[i];
  if(!isFinite(a.x)||!isFinite(a.y)||!isFinite(a.z)||!isFinite(a.w))throw std::invalid_argument("nonfinite plane");
  const double norm=std::sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
  if(!(norm>0)||!isFinite(norm))throw std::invalid_argument("degenerate frustum plane");
  planes[i]={{a.x/norm,a.y/norm,a.z/norm},a.w/norm};
 }
 return planes;
}
Visibility classify(const std::array<Plane,6>& planes,const AABB& box){ // [한국어 해설] extract가 만든 유효한 평면을 전제로 한다. 임의 외부 평면의 유한성·단위길이까지 다시 검사하지 않는다.
 const Vec3 a=box.lo,b=box.hi;
 if(!isFinite(a.x)||!isFinite(a.y)||!isFinite(a.z)||!isFinite(b.x)||!isFinite(b.y)||!isFinite(b.z)||
    a.x>b.x||a.y>b.y||a.z>b.z)throw std::invalid_argument("invalid AABB");
 bool straddles=false;
 for(const Plane& p:planes){
  const Vec3 positive{p.n.x>=0?b.x:a.x,p.n.y>=0?b.y:a.y,p.n.z>=0?b.z:a.z}; // [한국어 해설] 법선 방향 최대 지지점도 밖이면 전체 AABB가 밖이다. 최소 지지점만 밖이면 경계와 교차한다.
  const Vec3 negative{p.n.x>=0?a.x:b.x,p.n.y>=0?a.y:b.y,p.n.z>=0?a.z:b.z};
  const double maxDist=p.n.x*positive.x+p.n.y*positive.y+p.n.z*positive.z+p.d;
  const double minDist=p.n.x*negative.x+p.n.y*negative.y+p.n.z*negative.z+p.d;
  if(maxDist<0)return Visibility::Outside;
  if(minDist<0)straddles=true;
 }
 return straddles?Visibility::Intersect:Visibility::Inside;
}
Matrix identity(){Matrix m{};m.r[0]={1,0,0,0};m.r[1]={0,1,0,0};m.r[2]={0,0,1,0};m.r[3]={0,0,0,1};return m;}
Matrix perspectiveLH(double nearZ,double farZ){ // [한국어 해설] 양의 near와 far>near를 요구하는 축약 투영이다. 입력 전체 유한성 검사를 갖춘 범용 행렬 API는 아니다.
 if(!(nearZ>0&&farZ>nearZ))throw std::invalid_argument("near/far");
 const double q=farZ/(farZ-nearZ);
 Matrix m{};m.r[0]={1,0,0,0};m.r[1]={0,1,0,0};m.r[2]={0,0,q,-q*nearZ};m.r[3]={0,0,1,0};return m;
}
unsigned count=0;
void ck(bool b,const char* msg){++count;if(!b)throw std::runtime_error(msg);}
template<class F> void reject(F f){bool failed=false;try{f();}catch(const std::invalid_argument&){failed=true;}ck(failed,"expected invalid_argument");}
int main(){ // [한국어 해설] 33개 평면·AABB CPU 검사다. 실제 GPU 컬링 이미지나 모든 수치 범위의 증명은 아니다.
 const auto unit=extract(identity());
 ck(classify(unit,{{0,0,0.5},{0,0,0.5}})==Visibility::Inside,"central point inside");
 ck(classify(unit,{{-0.1,-0.1,0.3},{0.1,0.1,0.7}})==Visibility::Inside,"box inside");
 ck(classify(unit,{{2,0,0.5},{3,1,0.9}})==Visibility::Outside,"x right outside");
 ck(classify(unit,{{-3,0,0.2},{-2,0.3,0.8}})==Visibility::Outside,"x left outside");
 ck(classify(unit,{{0,2,0.2},{1,3,0.9}})==Visibility::Outside,"y top outside");
 ck(classify(unit,{{0,-3,0.2},{1,-2,0.9}})==Visibility::Outside,"y bottom outside");
 ck(classify(unit,{{0,0,-2},{0.5,0.5,-1}})==Visibility::Outside,"z near outside");
 ck(classify(unit,{{0,0,2},{0.5,0.5,3}})==Visibility::Outside,"z far outside");
 ck(classify(unit,{{0.5,0,0.5},{1.5,0.5,0.8}})==Visibility::Intersect,"x straddles");
 ck(classify(unit,{{0,0,-0.2},{0.3,0.3,0.2}})==Visibility::Intersect,"near straddles");
 ck(classify(unit,{{0,0,0.8},{0.3,0.3,1.2}})==Visibility::Intersect,"far straddles");
 ck(classify(unit,{{1,0,0.5},{1,0,0.5}})==Visibility::Inside,"boundary is visible");
 ck(classify(unit,{{0,0,0},{0,0,0}})==Visibility::Inside,"near plane boundary");
 ck(classify(unit,{{0,0,1},{0,0,1}})==Visibility::Inside,"far plane boundary");
 ck(classify(unit,{{-1,-1,0},{1,1,1}})==Visibility::Inside,"full viewport inclusive");
 reject([&]{classify(unit,{{1,0,0},{-1,1,1}});});
 reject([&]{classify(unit,{{0,1,0},{1,0,1}});});
 reject([&]{classify(unit,{{0,0,1},{1,1,0}});});
 reject([&]{const double n=std::numeric_limits<double>::quiet_NaN();classify(unit,{{n,0,0},{1,1,1}});});
 reject([&]{const double n=std::numeric_limits<double>::infinity();classify(unit,{{0,0,0},{n,1,1}});});
 reject([&]{Matrix m{};extract(m);});
 reject([&]{auto m=identity();m.r[3].w=std::numeric_limits<double>::quiet_NaN();extract(m);});
 reject([&]{perspectiveLH(0,10);});reject([&]{perspectiveLH(10,10);});reject([&]{perspectiveLH(10,1);});
 const auto depth=extract(perspectiveLH(0.5,10.0));
 ck(classify(depth,{{0,0,1},{0,0,1}})==Visibility::Inside,"perspective inside");
 ck(classify(depth,{{0,0,0.1},{0,0,0.2}})==Visibility::Outside,"perspective near reject");
 ck(classify(depth,{{0,0,11},{0,0,12}})==Visibility::Outside,"perspective far reject");
 ck(classify(depth,{{3,0,1},{4,0,1}})==Visibility::Outside,"perspective right reject");
 ck(classify(depth,{{-0.5,-0.5,1},{0.5,0.5,1}})==Visibility::Inside,"perspective middle");
 ck(classify(depth,{{0,0,0.3},{0,0,0.7}})==Visibility::Intersect,"perspective near cross");
 ck(classify(depth,{{0,0,9.5},{0,0,10.5}})==Visibility::Intersect,"perspective far cross");
 ck(classify(depth,{{-0.2,-0.2,-2},{0.2,0.2,-1}})==Visibility::Outside,"behind camera");
 std::cout<<"PASS "<<count<<" frustum plane and AABB CPU checks\n";
}
