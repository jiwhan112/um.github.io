// DX9 09C: CPU-only teaching model, not a Direct3D9 runtime test.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
struct Color { double r,g,b; };
struct Vec3 { double x,y,z; };
Color operator+(Color a, Color b){return {a.r+b.r,a.g+b.g,a.b+b.b};}
Color operator*(Color a,double f){return {a.r*f,a.g*f,a.b*f};}
Vec3 operator+(Vec3 a,Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec3 operator-(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec3 operator*(Vec3 a,double f){return {a.x*f,a.y*f,a.z*f};}
double dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
double mag(Vec3 a){return std::sqrt(dot(a,a));}
Vec3 unit(Vec3 a){double len=mag(a);if(!std::isfinite(len)||len<1e-9)throw std::invalid_argument("zero vector");return a*(1/len);}
bool near(double a,double b){return std::fabs(a-b)<1e-9;}
bool near(Color a,Color b){return near(a.r,b.r)&&near(a.g,b.g)&&near(a.b,b.b);}
bool near(Vec3 a,Vec3 b){return near(a.x,b.x)&&near(a.y,b.y)&&near(a.z,b.z);}
void valid_alpha(double a){if(!std::isfinite(a)||a<0||a>1)throw std::out_of_range("alpha");}
Color straight(Color src,double a,Color dst){valid_alpha(a);return src*a+dst*(1-a);}
Color premult(Color src_times_alpha,double a,Color dst){valid_alpha(a);return src_times_alpha+dst*(1-a);}
Color additive(Color src,double a,Color dst){valid_alpha(a);return src*a+dst;}
bool alpha_ge(unsigned a,unsigned ref){if(a>255||ref>255)throw std::out_of_range("alpha8");return a>=ref;}
struct Pixel {Color c;double z;};
struct State {bool depthTest,depthWrite,alphaTest,blend;unsigned alphaRef;};
bool draw_cutout(Pixel& p,Color src,double z,unsigned alpha8,State s){
    if(s.blend)throw std::invalid_argument("cutout state");
    if(s.alphaTest&&!alpha_ge(alpha8,s.alphaRef))return false;
    if(s.depthTest&&z>p.z)return false; // sample ZFUNC=LESSEQUAL
    p.c=src;if(s.depthWrite)p.z=z;return true;
}
bool draw_alpha(Pixel& p,Color src,double z,double a,State s){
    if(!s.blend)throw std::invalid_argument("blend state");
    valid_alpha(a);
    if(s.depthTest&&z>p.z)return false;
    p.c=straight(src,a,p.c);if(s.depthWrite)p.z=z;return true;
}
struct Axes {Vec3 right,up,face;};
Axes spherical(Vec3 center,Vec3 camera){
    Vec3 face=unit(camera-center),right=unit(cross({0,1,0},face));
    return {right,unit(cross(face,right)),face};
}
Axes cylindrical(Vec3 center,Vec3 camera){
    Vec3 d=camera-center,face=unit({d.x,0,d.z});
    return {unit(cross({0,1,0},face)),{0,1,0},face};
}
struct Sprite {int id;Vec3 center;};
void far_first(std::vector<Sprite>& sprites,Vec3 camera,Vec3 forward){
    Vec3 f=unit(forward);
    std::stable_sort(sprites.begin(),sprites.end(),[&](const Sprite& a,const Sprite& b){
        return dot(a.center-camera,f)>dot(b.center-camera,f);
    });
}
struct Particle {Vec3 pos,velocity;double age,lifetime;};
bool alive(const Particle& p){return p.age>=0&&p.age<p.lifetime;}
void tick(Particle& p,double dt){
    if(!std::isfinite(dt)||dt<0||!std::isfinite(p.lifetime)||p.lifetime<=0)
        throw std::invalid_argument("time");
    if(!alive(p))return;
    double delta=std::min(dt,p.lifetime-p.age);p.pos=p.pos+p.velocity*delta;p.age+=delta;
}
int main(){
    unsigned n=0;
#define CHECK(c) do{++n;if(!(c)){std::cerr<<"FAILED line "<<__LINE__<<": "<<#c<<"\n";return 1;}}while(false)
    const Color bg{.2,.4,.6},red{1,0,0},blue{0,0,1};
    CHECK(near(straight(red,0,bg),bg));
    CHECK(near(straight(red,1,bg),red));
    CHECK(near(straight(red,.25,bg),Color{.4,.3,.45}));
    CHECK(near(premult(red*.25,.25,bg),straight(red,.25,bg)));
    CHECK(near(additive(red,.5,bg),Color{.7,.4,.6}));
    CHECK(near(additive(red,.5,additive(blue,.5,bg)),additive(blue,.5,additive(red,.5,bg))));
    CHECK(!near(straight(red,.5,straight(blue,.5,bg)),straight(blue,.5,straight(red,.5,bg))));
    bool rejected=false;try{(void)straight(red,-.1,bg);}catch(const std::out_of_range&){rejected=true;}CHECK(rejected);
    rejected=false;try{(void)alpha_ge(256,1);}catch(const std::out_of_range&){rejected=true;}CHECK(rejected);
    CHECK(!alpha_ge(0,1));CHECK(alpha_ge(1,1));CHECK(alpha_ge(2,1));
    CHECK(alpha_ge(255,255));CHECK(!alpha_ge(254,255));
    const State cut{true,true,true,false,1};
    Pixel p{bg,.6};
    CHECK(!draw_cutout(p,red,.2,0,cut));CHECK(near(p.c,bg));CHECK(near(p.z,.6));
    CHECK(draw_cutout(p,red,.2,1,cut));CHECK(near(p.c,red));CHECK(near(p.z,.2));
    CHECK(!draw_cutout(p,blue,.5,255,cut));CHECK(near(p.c,red));
    const State alpha{true,false,false,true,0};
    Pixel q{bg,.6};
    CHECK(draw_alpha(q,red,.3,.5,alpha));CHECK(near(q.c,Color{.6,.2,.3}));CHECK(near(q.z,.6));
    CHECK(!draw_alpha(q,blue,.9,.5,alpha));CHECK(near(q.c,Color{.6,.2,.3}));
    const State broken{true,true,false,true,0};
    Pixel wrong{bg,.9};
    CHECK(draw_alpha(wrong,red,.2,0,broken));CHECK(near(wrong.c,bg));CHECK(near(wrong.z,.2));
    CHECK(!draw_cutout(wrong,blue,.4,255,cut));
    const Axes s=spherical({0,0,0},{0,0,10});
    CHECK(near(s.face,Vec3{0,0,1}));CHECK(near(s.right,Vec3{1,0,0}));CHECK(near(s.up,Vec3{0,1,0}));
    CHECK(near(dot(s.face,s.right),0));CHECK(near(dot(s.right,s.up),0));
    const Axes c=cylindrical({0,0,0},{10,3,0});
    CHECK(near(c.face,Vec3{1,0,0}));CHECK(near(c.up,Vec3{0,1,0}));CHECK(near(c.right,Vec3{0,0,-1}));
    rejected=false;try{(void)spherical({0,0,0},{0,10,0});}catch(const std::invalid_argument&){rejected=true;}CHECK(rejected);
    rejected=false;try{(void)cylindrical({0,0,0},{0,10,0});}catch(const std::invalid_argument&){rejected=true;}CHECK(rejected);
    std::vector<Sprite> queue{{1,{0,0,5}},{2,{0,0,20}},{3,{0,0,5}},{4,{0,0,10}}};
    far_first(queue,{0,0,0},{0,0,1});
    CHECK(queue[0].id==2);CHECK(queue[1].id==4);CHECK(queue[2].id==1);CHECK(queue[3].id==3);
    Particle particle{{0,0,0},{2,0,0},0,1};
    CHECK(alive(particle));tick(particle,.25);CHECK(near(particle.pos,Vec3{.5,0,0}));CHECK(near(particle.age,.25));
    tick(particle,2);CHECK(near(particle.pos,Vec3{2,0,0}));CHECK(near(particle.age,1));CHECK(!alive(particle));
    tick(particle,.1);CHECK(near(particle.pos,Vec3{2,0,0}));
    rejected=false;try{tick(particle,-1);}catch(const std::invalid_argument&){rejected=true;}CHECK(rejected);
    std::cout<<"PASS: "<<n<<" explicit assertions (single CPU run)\n";
    return 0;
}

