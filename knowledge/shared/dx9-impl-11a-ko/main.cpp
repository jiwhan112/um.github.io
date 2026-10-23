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
Plane unit(V4 v){ // [한국어 해설] 법선뿐 아니라 d도 같은 길이로 나눠 평면의 위치와 거리 해석을 보존한다.
  double len=std::hypot(v.x,std::hypot(v.y,v.z));
  if(!ok(len)||len<1.e-12||!ok(v.w))throw std::invalid_argument("plane");
  const Plane p{{v.x/len,v.y/len,v.z/len},v.w/len};
  if(!ok(p.n)||!ok(p.d))throw std::invalid_argument("normalized plane overflow");
  return p;
}
Frustum extract(const Mat&m){ // [한국어 해설] 행 벡터 클립 부등식으로 열을 조합한다. D3D near 평면은 z열이며 OpenGL식 w+z가 아니다.
  for(const auto&row:m.a)for(double n:row)if(!ok(n))throw std::invalid_argument("matrix");
  const V4 x=col(m,0),y=col(m,1),z=col(m,2),w=col(m,3);
  // x,y in [-w,w], z in [0,w]. Inward normals.
  return {{unit(plus(w,x)),unit(minus(w,x)),unit(plus(w,y)),
           unit(minus(w,y)),unit(z),unit(minus(w,z))}};
}
// Public classify functions require six inward, unit, finite planes.
// Validate all six before early-out so malformed data is not silently accepted.
void validate(const Frustum& f){ // [한국어 해설] 조기 탈락 전에 여섯 평면을 모두 검증해 뒤쪽의 잘못된 입력을 숨기지 않는다.
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
Result classify(const Frustum&f,Sphere s,double epsilon=1.e-9){ // [한국어 해설] 구 중심의 부호 거리와 반지름으로 보수적으로 분류한다. 접촉은 경계 교차로 남긴다.
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
Result classify(const Frustum&f,Aabb b,double epsilon=1.e-9){ // [한국어 해설] 월드 축 정렬 상자의 반폭을 법선에 투영한다. 회전한 OBB에 이 식을 그대로 적용하면 안 된다.
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
int runTests(){ // [한국어 해설] 이름 있는 검사와 15000개 속성 비교 표본은 별도 수치다. 표본 개수를 독립 검사 수로 부풀리지 않는다.
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
