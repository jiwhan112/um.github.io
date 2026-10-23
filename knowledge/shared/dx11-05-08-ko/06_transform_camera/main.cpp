// DX11 coordinate contract: portable C++11 CPU model, not DirectXMath or GPU.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
struct V {double x,y,z;};
V operator+(V a,V b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
V operator-(V a,V b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
V operator*(V a,double s){return {a.x*s,a.y*s,a.z*s};}
double dot(V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;}
V cross(V a,V b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
double norm(V v){return std::sqrt(dot(v,v));}
V unit(V v){double n=norm(v);if(!(n>1e-12)||!std::isfinite(n))throw std::invalid_argument("bad axis");return v*(1/n);}
struct M {double v[4][4]={};};
M identity(){M x;for(int i=0;i<4;++i)x.v[i][i]=1;return x;}
M mul(M a,M b){M o;for(int i=0;i<4;++i)for(int j=0;j<4;++j)for(int k=0;k<4;++k)o.v[i][j]+=a.v[i][k]*b.v[k][j];return o;}
M translate(V t){M x=identity();x.v[3][0]=t.x;x.v[3][1]=t.y;x.v[3][2]=t.z;return x;}
M scale(V s){M x=identity();x.v[0][0]=s.x;x.v[1][1]=s.y;x.v[2][2]=s.z;return x;}
M yaw(double t){M x=identity();double c=std::cos(t),s=std::sin(t);x.v[0][0]=c;x.v[0][2]=-s;x.v[2][0]=s;x.v[2][2]=c;return x;}
struct P4 {double x,y,z,w;};
P4 transform(P4 p,M m){const double a[4]={p.x,p.y,p.z,p.w};double b[4]={};for(int j=0;j<4;++j)for(int i=0;i<4;++i)b[j]+=a[i]*m.v[i][j];return {b[0],b[1],b[2],b[3]};}
M lookAtLH(V eye,V at,V up){ // [한국어 해설] 앞 방향과 up의 외적으로 오른쪽 축을 만든다. eye=at 또는 평행 up처럼 축이 무너지면 거부한다.
 const V f=unit(at-eye),r=unit(cross(up,f)),u=cross(f,r);M v=identity();
 v.v[0][0]=r.x;v.v[1][0]=r.y;v.v[2][0]=r.z;
 v.v[0][1]=u.x;v.v[1][1]=u.y;v.v[2][1]=u.z;
 v.v[0][2]=f.x;v.v[1][2]=f.y;v.v[2][2]=f.z;
 v.v[3][0]=-dot(eye,r);v.v[3][1]=-dot(eye,u);v.v[3][2]=-dot(eye,f);return v;
}
M perspectiveLH(double fovy,double aspect,double zn,double zf){ // [한국어 해설] 왼손 +Z 전방, near→0/far→1 규약이다. 양수 near와 far>near 조건을 검사한다.
 if(!std::isfinite(fovy)||!std::isfinite(aspect)||!std::isfinite(zn)||!std::isfinite(zf)||
   !(fovy>0&&fovy<3.141592653589793&&aspect>0&&zn>0&&zf>zn))throw std::invalid_argument("bad perspective");
 const double h=1/std::tan(fovy/2);M p{};p.v[0][0]=h/aspect;p.v[1][1]=h;p.v[2][2]=zf/(zf-zn);
 p.v[2][3]=1;p.v[3][2]=-zn*zf/(zf-zn);return p;
}
P4 ndc(P4 clip){if(!std::isfinite(clip.w)||std::abs(clip.w)<1e-12)throw std::invalid_argument("bad w");return {clip.x/clip.w,clip.y/clip.w,clip.z/clip.w,1};} // [한국어 해설] w를 검사한 뒤 나눈다. 이 함수는 여섯 클립 평면 판정의 대체물이 아니다.
struct Viewport {double x,y,w,h;};
V screen(P4 n,Viewport vp){if(!(vp.w>0&&vp.h>0))throw std::invalid_argument("bad viewport");return {vp.x+(n.x+1)*vp.w/2,vp.y+(1-n.y)*vp.h/2,n.z};} // [한국어 해설] NDC y 위쪽을 화면 y 아래쪽으로 뒤집고 뷰포트 오프셋을 더한다.
bool close(double a,double b){return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<1e-8;}
int main(){unsigned n=0;const auto ck=[&](bool b,const char* m){++n;if(!b)throw std::runtime_error(m);}; // [한국어 해설] 행 벡터 합성·카메라·투영·화면 변환과 실패 입력을 CPU 수치로 검사한다.
 const M i=identity();P4 p{2,3,4,1};auto q=transform(p,i);
 ck(close(q.x,2),"identity x");ck(close(q.y,3),"identity y");ck(close(q.z,4),"identity z");
 q=transform(p,translate({10,20,30}));ck(close(q.x,12),"translated x");ck(close(q.y,23),"translated y");ck(close(q.z,34),"translated z");
 auto d=transform({1,0,0,0},translate({10,0,0}));ck(close(d.x,1),"direction unaffected");
 const auto child=mul(scale({2,2,2}),translate({1,2,3}));q=transform({1,0,0,1},mul(child,translate({10,0,0})));
 ck(close(q.x,13),"child parent");ck(close(q.y,2),"child y");
 q=transform({1,0,0,1},mul(yaw(3.141592653589793/2),translate({1,2,3})));
 ck(close(q.x,1),"yaw x");ck(close(q.z,2),"yaw z");
 const M v=lookAtLH({0,0,-5},{0,0,0},{0,1,0});q=transform({0,0,0,1},v);
 ck(close(q.z,5),"view forward");ck(close(q.x,0),"view center");
 const M proj=perspectiveLH(3.141592653589793/2,1,1,101);
 auto c=transform({0,0,1,1},proj);ck(close(c.z/c.w,0),"near ndc depth");
 c=transform({0,0,101,1},proj);ck(close(c.z/c.w,1),"far ndc depth");
 c=transform({1,0,1,1},proj);ck(close(c.x/c.w,1),"right clip");
 c=transform({-1,0,1,1},proj);ck(close(c.x/c.w,-1),"left clip");
 c=transform({0,1,1,1},proj);ck(close(c.y/c.w,1),"top clip");
 const auto center=screen(ndc(transform({0,0,5,1},proj)),{100,50,800,600});
 ck(close(center.x,500),"viewport x offset");ck(close(center.y,350),"viewport flipped y");
 auto top=screen({0,1,.5,1},{100,50,800,600});ck(close(top.y,50),"screen top");
 auto bottom=screen({0,-1,.5,1},{100,50,800,600});ck(close(bottom.y,650),"screen bottom");
 auto bad=[&](bool b){ck(b,"must reject invalid input");};bool caught=false;
 try{perspectiveLH(0,1,1,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{perspectiveLH(1,0,1,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{perspectiveLH(1,1,0,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{perspectiveLH(1,1,100,100);}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{lookAtLH({0,0,0},{0,0,0},{0,1,0});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{lookAtLH({0,0,0},{0,0,1},{0,0,1});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{ndc({1,2,3,0});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 caught=false;try{screen({0,0,0,1},{0,0,0,1});}catch(const std::invalid_argument&){caught=true;}bad(caught);
 std::cout<<"PASS "<<n<<" transform/camera CPU contract checks\n";
}
