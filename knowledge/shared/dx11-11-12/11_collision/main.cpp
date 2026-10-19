// CPU-only collider reference: AABB, sphere, 2D XZ OBB footprint SAT, 3D ray vs AABB.
// The planar OBB test is NOT a full 3D 15-axis OBB SAT.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
struct V{double x,y,z;};
V operator-(V a,V b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V operator+(V a,V b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V operator*(V a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;}
bool finite(V a){return std::isfinite(a.x)&&std::isfinite(a.y)&&std::isfinite(a.z);}
struct AABB{V lo,hi;};
void valid(AABB a){if(!finite(a.lo)||!finite(a.hi)||a.lo.x>a.hi.x||a.lo.y>a.hi.y||a.lo.z>a.hi.z)throw std::invalid_argument("invalid AABB");}
bool overlap(AABB a,AABB b){valid(a);valid(b);return a.lo.x<=b.hi.x&&a.hi.x>=b.lo.x && a.lo.y<=b.hi.y&&a.hi.y>=b.lo.y && a.lo.z<=b.hi.z&&a.hi.z>=b.lo.z;}
struct Sphere{V p;double r;};
bool sphereAabb(Sphere s,AABB a){valid(a);if(!finite(s.p)||!std::isfinite(s.r)||s.r<0)throw std::invalid_argument("invalid sphere");
 const V q{std::max(a.lo.x,std::min(s.p.x,a.hi.x)),std::max(a.lo.y,std::min(s.p.y,a.hi.y)),std::max(a.lo.z,std::min(s.p.z,a.hi.z))};
 const V d=s.p-q;return dot(d,d)<=s.r*s.r;
}
struct ObbXZ{V center;double ex,ez,angle;};
void valid(ObbXZ b){if(!finite(b.center)||!std::isfinite(b.ex)||!std::isfinite(b.ez)||!std::isfinite(b.angle)||b.ex<0||b.ez<0)throw std::invalid_argument("invalid planar OBB");}
bool obbXZ(ObbXZ a,ObbXZ b){
 valid(a);valid(b);const double ca=std::cos(a.angle),sa=std::sin(a.angle),cb=std::cos(b.angle),sb=std::sin(b.angle);
 const V ax{ca,0,sa},az{-sa,0,ca},bx{cb,0,sb},bz{-sb,0,cb},d=b.center-a.center;
 const V axis[]={ax,az,bx,bz};
 for(const V t:axis){const double ra=a.ex*std::fabs(dot(t,ax))+a.ez*std::fabs(dot(t,az));
   const double rb=b.ex*std::fabs(dot(t,bx))+b.ez*std::fabs(dot(t,bz));
   if(std::fabs(dot(t,d))>ra+rb+1e-10)return false;
 }
 return true;
}
struct Ray{V origin,dir;double maxT;};
bool rayAabb(Ray ray,AABB a,double& outT){
 valid(a);if(!finite(ray.origin)||!finite(ray.dir)||!std::isfinite(ray.maxT)||ray.maxT<0)throw std::invalid_argument("invalid ray");
 if(dot(ray.dir,ray.dir)<1e-24)throw std::invalid_argument("zero ray direction");
 double tmin=0,tmax=ray.maxT;
 const double o[]={ray.origin.x,ray.origin.y,ray.origin.z},d[]={ray.dir.x,ray.dir.y,ray.dir.z},lo[]={a.lo.x,a.lo.y,a.lo.z},hi[]={a.hi.x,a.hi.y,a.hi.z};
 for(int i=0;i<3;++i){if(std::fabs(d[i])<1e-12){if(o[i]<lo[i]||o[i]>hi[i])return false;continue;}
  double t0=(lo[i]-o[i])/d[i],t1=(hi[i]-o[i])/d[i];if(t0>t1)std::swap(t0,t1);
  tmin=std::max(tmin,t0);tmax=std::min(tmax,t1);if(tmax<tmin)return false;
 }
 outT=tmin;return true;
}
bool near(double a,double b){return std::isfinite(a)&&std::isfinite(b)&&std::fabs(a-b)<1e-9;}
int main(){unsigned n=0;auto ck=[&](bool ok,const char* label){++n;if(!ok)throw std::runtime_error(label);};
 const AABB a{{-1,-1,-1},{1,1,1}};ck(overlap(a,a),"same AABB");ck(overlap(a,{{1,0,0},{2,1,1}}),"touch X");
 ck(!overlap(a,{{1.01,0,0},{2,1,1}}),"separated X");ck(!overlap(a,{{0,2,0},{1,3,1}}),"separated Y");
 ck(!overlap(a,{{0,0,2},{1,1,3}}),"separated Z");ck(overlap(a,{{-2,-2,-2},{2,2,2}}),"contained");
 ck(sphereAabb({{0,0,0},0},a),"inside sphere point");ck(sphereAabb({{2,0,0},1},a),"sphere touch");
 ck(!sphereAabb({{2.01,0,0},1},a),"sphere miss");ck(!sphereAabb({{2,2,0},1},a),"corner miss");
 ck(sphereAabb({{2,2,0},std::sqrt(2.0)},a),"corner touch");
 const ObbXZ o{{0,0,0},1,1,0};ck(obbXZ(o,o),"same OBB");
 ck(obbXZ(o,{{2,0,0},1,1,0}),"OBB touching");ck(!obbXZ(o,{{2.1,0,0},1,1,0}),"OBB gap");
 ck(obbXZ(o,{{1,0,0},1,1,.7853981633974483}),"OBB rotated overlap");
 ck(!obbXZ(o,{{5,0,5},1,1,.7853981633974483}),"OBB rotated far");
 ck(obbXZ(o,{{0,20,0},1,1,0}),"planar footprint ignores Y");
 double t=-1;ck(rayAabb({{-3,0,0},{1,0,0},10},a,t),"ray enters");ck(near(t,2),"ray t");
 ck(!rayAabb({{-3,5,0},{1,0,0},10},a,t),"parallel Y miss");
 ck(rayAabb({{0,0,0},{1,0,0},10},a,t),"ray inside");ck(near(t,0),"inside t0");
 ck(!rayAabb({{-3,0,0},{1,0,0},1},a,t),"limited segment");
 auto bad=[&](bool x){ck(x,"invalid input rejected");};bool ex=false;
 try{overlap({{2,0,0},{1,1,1}},a);}catch(const std::invalid_argument&){ex=true;}bad(ex);
 ex=false;try{sphereAabb({{0,0,0},-1},a);}catch(const std::invalid_argument&){ex=true;}bad(ex);
 ex=false;try{obbXZ({{0,0,0},-1,1,0},o);}catch(const std::invalid_argument&){ex=true;}bad(ex);
 ex=false;try{obbXZ({{0,0,0},1,1,std::numeric_limits<double>::quiet_NaN()},o);}catch(const std::invalid_argument&){ex=true;}bad(ex);
 ex=false;try{rayAabb({{0,0,0},{0,0,0},1},a,t);}catch(const std::invalid_argument&){ex=true;}bad(ex);
 ex=false;try{rayAabb({{0,0,0},{1,0,0},-1},a,t);}catch(const std::invalid_argument&){ex=true;}bad(ex);
 std::cout<<"PASS "<<n<<" collider CPU contract checks\n";
}