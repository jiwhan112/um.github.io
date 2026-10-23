#include <functional>
#include <algorithm>
#include <cstddef>
#include <utility>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

struct V3 { double x, y, z; };
bool finite(V3 v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
V3 operator+(V3 a, V3 b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V3 operator-(V3 a, V3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
V3 operator*(V3 a, double s) { return {a.x*s,a.y*s,a.z*s}; }
double dot(V3 a,V3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
V3 cross(V3 a,V3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
double length(V3 a) { return std::sqrt(dot(a,a)); }
V3 normalized(V3 a) {
    const double n=length(a);
    if (!(n>1e-12) || !std::isfinite(n)) throw std::invalid_argument("zero/invalid direction");
    return a*(1.0/n);
}
bool close(double a,double b) { return std::abs(a-b)<1e-8; }
struct Ray { V3 origin, dir; double maxT; }; // dir world-unit; near -> far
struct Viewport { double x,y,width,height; };
Ray makeRay(double px,double py,Viewport vp,V3 eye,V3 at,V3 up,double fovy,double zn,double zf) { // [한국어 해설] 뷰포트 오프셋과 화면 y 반전을 반영해 near에서 far까지 유한 구간 광선을 만든다.
    if (!std::isfinite(px)||!std::isfinite(py)||!std::isfinite(vp.x)||!std::isfinite(vp.y)||
        !std::isfinite(vp.width)||!std::isfinite(vp.height)||!std::isfinite(fovy)||
        !std::isfinite(zn)||!std::isfinite(zf)||!finite(eye)||!finite(at)||!finite(up))
        throw std::invalid_argument("non-finite camera/viewport");
    if (!(vp.width>0 && vp.height>0 && zn>0 && zf>zn && fovy>0 && fovy<3.141592653589793))
        throw std::invalid_argument("invalid camera/viewport");
    if (!(px>=vp.x && px<vp.x+vp.width && py>=vp.y && py<vp.y+vp.height))
        throw std::out_of_range("mouse outside viewport");
    const V3 f=normalized(at-eye),r=normalized(cross(up,f)),u=cross(f,r);
    const double nx=2.0*(px-vp.x)/vp.width-1.0;
    const double ny=1.0-2.0*(py-vp.y)/vp.height;
    const double h=std::tan(fovy*0.5),aspect=vp.width/vp.height;
    const V3 nearVec=f+r*(nx*aspect*h)+u*(ny*h);
    const V3 start=eye+nearVec*zn;
    const V3 end=eye+nearVec*zf;
    const double maxT=length(end-start);
    if(!finite(start)||!finite(end)||!std::isfinite(maxT)||maxT<=0)
        throw std::invalid_argument("ray construction overflow");
    return {start, normalized(nearVec), maxT};
}
struct Hit { bool yes; double t,u,v; };
Hit intersect(Ray ray,V3 a,V3 b,V3 c) { // [한국어 해설] 양면 삼각형 교차다. 평행/퇴화·바깥 무게중심·광선 구간 밖 결과를 거부한다.
    const double eps=1e-10;
    if(!finite(ray.origin)||!finite(ray.dir)||!std::isfinite(ray.maxT)||ray.maxT<0||
       !finite(a)||!finite(b)||!finite(c)) throw std::invalid_argument("non-finite ray/triangle");
    const double dirLength=length(ray.dir);
    if(!std::isfinite(dirLength)||dirLength<=1e-12) throw std::invalid_argument("invalid ray direction");
    const V3 e1=b-a,e2=c-a,p=cross(ray.dir,e2);
    const double det=dot(e1,p);
    if (!std::isfinite(det)) throw std::invalid_argument("intersection overflow");
    if (std::abs(det)<eps) return {false,0,0,0};
    const double inv=1.0/det;
    const V3 s=ray.origin-a;
    const double u=dot(s,p)*inv;
    if (!std::isfinite(u)) throw std::invalid_argument("barycentric overflow");
    if (u<-eps || u>1+eps) return {false,0,0,0};
    const V3 q=cross(s,e1);
    const double v=dot(ray.dir,q)*inv;
    if (!std::isfinite(v)) throw std::invalid_argument("barycentric overflow");
    if (v<-eps || u+v>1+eps) return {false,0,0,0};
    const double t=dot(e2,q)*inv;
    if (!std::isfinite(t)) throw std::invalid_argument("distance overflow");
    if (t<-eps || t>ray.maxT+eps) return {false,0,0,0};
    return {true,t,u,v};
}
struct Mat { double m[4][4]; };
Mat identity() { Mat a={}; for(int i=0;i<4;++i) a.m[i][i]=1; return a; }
Mat multiply(Mat a,Mat b) {
    Mat r={};
    for(int i=0;i<4;++i) for(int j=0;j<4;++j) for(int k=0;k<4;++k)
        r.m[i][j]+=a.m[i][k]*b.m[k][j];
    return r;
}
Mat scale(double x,double y,double z) { Mat a=identity(); a.m[0][0]=x;a.m[1][1]=y;a.m[2][2]=z;return a; }
Mat translate(double x,double y,double z) { Mat a=identity();a.m[3][0]=x;a.m[3][1]=y;a.m[3][2]=z;return a; }
Mat inverse(Mat a) { // [한국어 해설] 부분 피벗 가우스-조던 역행렬이다. 특이행렬과 비유한 값을 거부한다.
    for(const auto& row:a.m)for(double v:row)if(!std::isfinite(v))throw std::invalid_argument("non-finite matrix");
    Mat r=identity();
    for(int j=0;j<4;++j) {
        int pivot=j;
        for(int i=j+1;i<4;++i) if(std::abs(a.m[i][j])>std::abs(a.m[pivot][j])) pivot=i;
        if(std::abs(a.m[pivot][j])<1e-12) throw std::invalid_argument("singular world matrix");
        for(int k=0;k<4;++k) { std::swap(a.m[j][k],a.m[pivot][k]); std::swap(r.m[j][k],r.m[pivot][k]); }
        const double d=a.m[j][j];
        for(int k=0;k<4;++k) { a.m[j][k]/=d;r.m[j][k]/=d; }
        for(int i=0;i<4;++i) if(i!=j) {
            const double f=a.m[i][j];
            for(int k=0;k<4;++k) { a.m[i][k]-=f*a.m[j][k]; r.m[i][k]-=f*r.m[j][k]; }
        }
    }
    for(const auto& row:r.m)for(double v:row)if(!std::isfinite(v))throw std::invalid_argument("inverse overflow");
    return r;
}
V3 transform(V3 v,Mat m,bool point) {
    const double w=point?1.0:0.0;
    return {v.x*m.m[0][0]+v.y*m.m[1][0]+v.z*m.m[2][0]+w*m.m[3][0],
            v.x*m.m[0][1]+v.y*m.m[1][1]+v.z*m.m[2][1]+w*m.m[3][1],
            v.x*m.m[0][2]+v.y*m.m[1][2]+v.z*m.m[2][2]+w*m.m[3][2]};
}
Ray toLocal(Ray r,Mat world) { // [한국어 해설] 방향은 벡터로 역변환하되 재정규화하지 않는다. 그래야 t가 원래 월드 거리 매개변수로 유지된다.
    if(world.m[0][3]!=0||world.m[1][3]!=0||world.m[2][3]!=0||world.m[3][3]!=1)
        throw std::invalid_argument("local ray needs affine world");
    Mat inv=inverse(world);
    // Do not normalize the transformed direction: world-distance t must be preserved.
    return {transform(r.origin,inv,true),transform(r.dir,inv,false),r.maxT};
}
struct Terrain { // [한국어 해설] b→c 대각선의 두 삼각형 높이를 보간한다. 사각 셀의 bilinear 보간과 다른 표면이다.
    int width, depth;
    std::vector<double> y; // row-major (z*width+x), +z increases with row
    double sample(double x,double z) const {
        if(width<2 || depth<2 || static_cast<std::size_t>(width)>std::numeric_limits<std::size_t>::max()/static_cast<std::size_t>(depth) ||
           y.size()!=static_cast<std::size_t>(width)*static_cast<std::size_t>(depth) ||
           !std::isfinite(x)||!std::isfinite(z)||x<0||z<0||x>width-1||z>depth-1)
            throw std::out_of_range("invalid terrain/coordinate");
        const int ix=std::min(static_cast<int>(x),width-2),iz=std::min(static_cast<int>(z),depth-2);
        const double fx=x-ix,fz=z-iz;
        const std::size_t i=static_cast<std::size_t>(iz)*static_cast<std::size_t>(width)+static_cast<std::size_t>(ix);
        const std::size_t w=static_cast<std::size_t>(width);
        const double a=y[i],b=y[i+1],c=y[i+w],d=y[i+w+1];
        if(!std::isfinite(a)||!std::isfinite(b)||!std::isfinite(c)||!std::isfinite(d))
            throw std::invalid_argument("non-finite height");
        // Explicit diagonal b -> c; never bilinear interpolation of a triangle mesh.
        return fx+fz<=1 ? a*(1-fx-fz)+b*fx+c*fz : d*(fx+fz-1)+b*(1-fz)+c*(1-fx);
    }
};
int main() { // [한국어 해설] 화면광선·교차·비균일 변환·지형 경계와 잘못된 입력을 CPU에서 검사한다.
    int checks=0;
    const auto check=[&](bool yes){ if(!yes) throw std::runtime_error("assertion failed"); ++checks; };
    const double pi=3.141592653589793;
    const Viewport vp{100,50,800,600};
    const Ray r=makeRay(500,350,vp,{0,0,0},{0,0,1},{0,1,0},pi/2,1,100);
    check(close(r.origin.x,0)&&close(r.origin.y,0)&&close(r.origin.z,1));
    check(close(r.dir.x,0)&&close(r.dir.y,0)&&close(r.dir.z,1));
    check(close(r.maxT,99));
    const Ray corner=makeRay(100,50,vp,{0,0,0},{0,0,1},{0,1,0},pi/2,1,100);
    check(corner.dir.x<0&&corner.dir.y>0&&corner.dir.z>0);
    bool rejected=false;
    try { (void)makeRay(99,350,vp,{0,0,0},{0,0,1},{0,1,0},pi/2,1,100); } catch(const std::out_of_range&) { rejected=true; }
    check(rejected);
    rejected=false;
    try { (void)makeRay(500,350,vp,{0,0,0},{0,0,0},{0,1,0},pi/2,1,100); } catch(const std::invalid_argument&) { rejected=true; }
    check(rejected);
    rejected=false;
    try { (void)makeRay(500,350,vp,{0,0,0},{0,0,1},{0,0,1},pi/2,1,100); } catch(const std::invalid_argument&) { rejected=true; }
    check(rejected);
    rejected=false;
    try { (void)makeRay(500,350,vp,{0,0,0},{0,0,1},{0,1,0},pi/2,0,100); } catch(const std::invalid_argument&) { rejected=true; }
    check(rejected);
    const V3 a{-1,-1,5},b{1,-1,5},c{0,1,5};
    Hit h=intersect(r,a,b,c);
    check(h.yes&&close(h.t,4));
    check(close(h.u,.25)&&close(h.v,.5)&&close(1-h.u-h.v,.25));
    check(!intersect(r,{10,10,5},{12,10,5},{11,12,5}).yes);
    check(!intersect(r,{0,0,5},{0,0,5},{0,0,5}).yes);
    check(!intersect(r,{-1,-1,.5},{1,-1,.5},{0,1,.5}).yes);
    check(!intersect(r,{-1,-1,101},{1,-1,101},{0,1,101}).yes);
    const Mat world=multiply(scale(2,1,2),translate(0,0,1));
    const Ray local=toLocal(r,world);
    check(close(local.origin.z,0)&&close(local.dir.z,.5));
    h=intersect(local,{-1,-1,2},{1,-1,2},{0,1,2});
    check(h.yes&&close(h.t,4));
    const V3 worldHit=transform(local.origin+local.dir*h.t,world,true);
    check(close(worldHit.x,0)&&close(worldHit.y,0)&&close(worldHit.z,5));
    rejected=false;
    try { (void)toLocal(r,scale(0,1,1)); } catch(const std::invalid_argument&) { rejected=true; }
    check(rejected);
    const Terrain plane{2,2,{0,10,20,30}};
    check(close(plane.sample(.25,.25),7.5));
    check(close(plane.sample(.75,.75),22.5));
    check(close(plane.sample(1,1),30));
    const Terrain seam{2,2,{0,0,0,10}};
    check(close(seam.sample(.25,.25),0));
    check(close(seam.sample(.75,.75),5));
    rejected=false;
    try { (void)seam.sample(-.01,0); } catch(const std::out_of_range&) { rejected=true; }
    check(rejected);
    rejected=false;
    try { (void)Terrain{2,2,{1,2,3}}.sample(.1,.1); } catch(const std::out_of_range&) { rejected=true; }
    check(rejected);
    const double nan=std::numeric_limits<double>::quiet_NaN();
    auto invalid=[&](const std::function<void()>& fn){bool threw=false;try{fn();}catch(const std::invalid_argument&){threw=true;}check(threw);};
    invalid([&]{intersect(r,{nan,-1,5},b,c);});
    invalid([&]{Ray bad=r;bad.dir.x=nan;intersect(bad,a,b,c);});
    invalid([&]{Ray bad=r;bad.dir={0,0,0};intersect(bad,a,b,c);});
    invalid([&]{makeRay(500,350,vp,{0,0,0},{0,0,1},{0,1,0},pi/2,1,std::numeric_limits<double>::infinity());});
    invalid([&]{Mat bad=identity();bad.m[0][0]=nan;toLocal(r,bad);});
    invalid([&]{Mat bad=identity();bad.m[0][3]=0.5;toLocal(r,bad);});
    invalid([&]{Terrain{2,2,{0,0,0,nan}}.sample(.75,.75);});
    check(intersect(r,a,c,b).yes);
    std::cout<<"PASS "<<checks<<" checks\n";
}
