// DX9 09C의 한국어 주석 학습판입니다. 원본의 실행 코드와 53개 검사 조건을 유지했습니다.
// 이 프로그램은 CPU에서 수식과 상태 계약을 확인하며 Direct3D9 장치나 GPU를 호출하지 않습니다.
// 원본 파일은 original/main.cpp에 있으며, 주석·들여쓰기 외의 코드 토큰은 변경하지 않았습니다.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

// RGB는 선형 색 공간의 double 값입니다. sRGB 변환·8비트 양자화·알파 채널 저장은 다루지 않습니다.
struct Color {
    double r,g,b;
};

// 좌표·속도·깊이에는 유한한 값을 전달한다고 가정합니다. 모든 입력을 검사하는 엔진 API는 아닙니다.
struct Vec3 {
    double x,y,z;
};

Color operator+(Color a, Color b) {
    return {a.r+b.r,a.g+b.g,a.b+b.b};
}

Color operator*(Color a,double f) {
    return {a.r*f,a.g*f,a.b*f};
}

Vec3 operator+(Vec3 a,Vec3 b) {
    return {a.x+b.x,a.y+b.y,a.z+b.z};
}

Vec3 operator-(Vec3 a,Vec3 b) {
    return {a.x-b.x,a.y-b.y,a.z-b.z};
}

Vec3 operator*(Vec3 a,double f) {
    return {a.x*f,a.y*f,a.z*f};
}

// 내적은 두 방향의 관계를 나타냅니다. 정렬에서는 카메라 전방에 투영한 깊이를 구하는 데 씁니다.
double dot(Vec3 a,Vec3 b) {
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

// 외적은 두 축에 수직인 축을 만듭니다. 빌보드의 오른쪽·위쪽 방향을 구성할 때 사용합니다.
Vec3 cross(Vec3 a,Vec3 b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}

double mag(Vec3 a) {
    return std::sqrt(dot(a,a));
}

// 길이가 거의 0인 벡터에는 방향이 없습니다. 나누기 전에 거부해 잘못된 축의 전파를 막습니다.
Vec3 unit(Vec3 a) {
    double len=mag(a);
    if(!std::isfinite(len)||len<1e-9)
        throw std::invalid_argument("zero vector");
    return a*(1/len);
}

// CPU 검사의 허용 오차입니다. 실제 GPU의 오차·정밀도를 이 값으로 보장하지 않습니다.
bool near(double a,double b) {
    return std::fabs(a-b)<1e-9;
}

bool near(Color a,Color b) {
    return near(a.r,b.r)&&near(a.g,b.g)&&near(a.b,b.b);
}

bool near(Vec3 a,Vec3 b) {
    return near(a.x,b.x)&&near(a.y,b.y)&&near(a.z,b.z);
}

void valid_alpha(double a) {
    if(!std::isfinite(a)||a<0||a>1)
        throw std::out_of_range("alpha");
}

// 일반 알파: 아직 알파를 곱하지 않은 원색 src를 받습니다. 전경과 배경의 비중을 나눠 더합니다.
Color straight(Color src,double a,Color dst) {
    valid_alpha(a);
    return src*a+dst*(1-a);
}

// 사전 곱 알파: 입력 RGB가 이미 알파와 곱해져 있습니다. 여기서 다시 a를 곱하면 이중 적용입니다.
Color premult(Color src_times_alpha,double a,Color dst) {
    valid_alpha(a);
    return src_times_alpha+dst*(1-a);
}

// 가산형: 기존 배경을 줄이지 않고 전경의 기여분을 더합니다. 이 CPU 모델은 결과를 1로 제한하지 않습니다.
Color additive(Color src,double a,Color dst) {
    valid_alpha(a);
    return src*a+dst;
}

// 알파 테스트는 색을 섞는 작업이 아니라 통과 여부를 고르는 비교입니다. 이 모델은 0~255만 받습니다.
bool alpha_ge(unsigned a,unsigned ref) {
    if(a>255||ref>255)
        throw std::out_of_range("alpha8");
    return a>=ref;
}

// 화면 전체 대신 픽셀 하나의 현재 색과 깊이만 보관합니다.
struct Pixel {
    Color c;
    double z;
};

// 깊이 검사·깊이 기록·알파 테스트·알파 혼합은 서로 다른 스위치입니다.
struct State {
    bool depthTest,depthWrite,alphaTest,blend;
    unsigned alphaRef;
};

// 컷아웃 경로: 알파·깊이 검사를 통과한 조각만 색을 쓰며, 깊이 기록은 별도 상태를 따릅니다.
bool draw_cutout(Pixel& p,Color src,double z,unsigned alpha8,State s) {
    if(s.blend)
        throw std::invalid_argument("cutout state");
    if(s.alphaTest&&!alpha_ge(alpha8,s.alphaRef))
        return false;

    // ZFUNC=LESSEQUAL을 단순화했습니다. 들어오는 깊이가 더 멀면 탈락하고, 같으면 통과합니다.
    if(s.depthTest&&z>p.z)
        return false;
    p.c=src;
    if(s.depthWrite)
        p.z=z;
    return true;
}

// 일반 투명 경로: 깊이 비교 후 RGB를 혼합합니다. 이 함수는 알파 테스트까지 모사하지 않습니다.
bool draw_alpha(Pixel& p,Color src,double z,double a,State s) {
    if(!s.blend)
        throw std::invalid_argument("blend state");
    valid_alpha(a);
    if(s.depthTest&&z>p.z)
        return false;
    p.c=straight(src,a,p.c);

    // 알파가 0이어도 이 분기는 실행됩니다. 투명 패스에서 ZWRITE를 잘못 켰을 때의 실패도 재현합니다.
    if(s.depthWrite)
        p.z=z;
    return true;
}

struct Axes {
    Vec3 right,up,face;
};

// 구면 빌보드: 중심에서 카메라를 향하는 법선과 그에 수직인 두 축을 만듭니다.
Axes spherical(Vec3 center,Vec3 camera) {
    // 카메라 방향이 월드 Y축과 평행하면 외적이 0이 됩니다. 원본 정책대로 unit이 예외로 거부합니다.
    Vec3 face=unit(camera-center),right=unit(cross({0,1,0},face));
    return {right,unit(cross(face,right)),face};
}

// 원통 빌보드: 높이 방향의 기울기를 제거하고 월드 Y축을 유지합니다. 나무처럼 서 있는 면의 축입니다.
Axes cylindrical(Vec3 center,Vec3 camera) {
    Vec3 d=camera-center,face=unit({d.x,0,d.z});
    return {unit(cross({0,1,0},face)),{0,1,0},face};
}

struct Sprite {
    int id;
    Vec3 center;
};

// 중심점의 전방 깊이가 큰 물체를 먼저 그립니다. 유클리드 거리 정렬이나 교차 메시의 완전한 해법은 아닙니다.
void far_first(std::vector<Sprite>& sprites,Vec3 camera,Vec3 forward) {
    Vec3 f=unit(forward);

    // 같은 깊이는 입력 순서를 유지합니다. 모든 정렬 좌표가 유한해야 비교 관계도 안정적으로 성립합니다.
    std::stable_sort(sprites.begin(),sprites.end(),[&](const Sprite& a,const Sprite& b) {
        return dot(a.center-camera,f)>dot(b.center-camera,f);
    });
}

struct Particle {
    Vec3 pos,velocity;
    double age,lifetime;
};

// 수명과 정확히 같은 나이가 된 순간부터 죽은 입자입니다.
bool alive(const Particle& p) {
    return p.age>=0&&p.age<p.lifetime;
}

void tick(Particle& p,double dt) {
    if(!std::isfinite(dt)||dt<0||!std::isfinite(p.lifetime)||p.lifetime<=0)
        throw std::invalid_argument("time");
    if(!alive(p))
        return;

    // 이번 프레임이 남은 수명보다 길어도 만료 시점까지만 이동합니다. 만료 뒤 위치가 더 진행되지 않습니다.
    double delta=std::min(dt,p.lifetime-p.age);
    p.pos=p.pos+p.velocity*delta;
    p.age+=delta;
}

int main() {
    unsigned n=0;

    // 조건 하나마다 n을 증가시킵니다. __LINE__은 이 학습판에서 실패한 실제 줄을 표시합니다.
    // 원본과 같은 매크로 토큰·조건·출력 문자열을 유지하며, 주석을 늘린 만큼 진단 줄번호만 달라집니다.
#define CHECK(c) do{++n;if(!(c)){std::cerr<<"FAILED line "<<__LINE__<<": "<<#c<<"\n";return 1;}}while(false)
    const Color bg{.2,.4,.6},red{1,0,0},blue{0,0,1};

    // 1. 알파 0과 1의 끝값, 중간 혼합값, 두 알파 표현의 대응을 확인합니다.
    CHECK(near(straight(red,0,bg),bg));
    CHECK(near(straight(red,1,bg),red));
    CHECK(near(straight(red,.25,bg),Color{.4,.3,.45}));
    CHECK(near(premult(red*.25,.25,bg),straight(red,.25,bg)));
    CHECK(near(additive(red,.5,bg),Color{.7,.4,.6}));

    // 이 단순 가산식은 순서를 바꿔도 같은 결과지만, 일반 알파 혼합은 그리기 순서에 따라 달라집니다.
    CHECK(near(additive(red,.5,additive(blue,.5,bg)),additive(blue,.5,additive(red,.5,bg))));
    CHECK(!near(straight(red,.5,straight(blue,.5,bg)),straight(blue,.5,straight(red,.5,bg))));
    bool rejected=false;
    try {
        (void)straight(red,-.1,bg);
    } catch(const std::out_of_range&) {
        rejected=true;
    }
    CHECK(rejected);
    rejected=false;
    try {
        (void)alpha_ge(256,1);
    } catch(const std::out_of_range&) {
        rejected=true;
    }
    CHECK(rejected);

    // 2. 임계값과 같은 알파는 GREATER_EQUAL 비교를 통과합니다.
    CHECK(!alpha_ge(0,1));
    CHECK(alpha_ge(1,1));
    CHECK(alpha_ge(2,1));
    CHECK(alpha_ge(255,255));
    CHECK(!alpha_ge(254,255));
    const State cut{true,true,true,false,1};
    Pixel p{bg,.6};

    // 컷아웃에서 폐기한 조각은 색뿐 아니라 기존 깊이도 남겨 두어야 합니다.
    CHECK(!draw_cutout(p,red,.2,0,cut));
    CHECK(near(p.c,bg));
    CHECK(near(p.z,.6));
    CHECK(draw_cutout(p,red,.2,1,cut));
    CHECK(near(p.c,red));
    CHECK(near(p.z,.2));
    CHECK(!draw_cutout(p,blue,.5,255,cut));
    CHECK(near(p.c,red));

    // 3. 일반 투명 패스는 깊이 검사를 켜고 깊이 기록을 끕니다. 통과 후에도 기존 깊이가 유지됩니다.
    const State alpha{true,false,false,true,0};
    Pixel q{bg,.6};
    CHECK(draw_alpha(q,red,.3,.5,alpha));
    CHECK(near(q.c,Color{.6,.2,.3}));
    CHECK(near(q.z,.6));
    CHECK(!draw_alpha(q,blue,.9,.5,alpha));
    CHECK(near(q.c,Color{.6,.2,.3}));

    // 반례: 알파가 0이어도 깊이를 기록하면 뒤쪽의 파란 조각이 가려집니다.
    const State broken{true,true,false,true,0};
    Pixel wrong{bg,.9};
    CHECK(draw_alpha(wrong,red,.2,0,broken));
    CHECK(near(wrong.c,bg));
    CHECK(near(wrong.z,.2));
    CHECK(!draw_cutout(wrong,blue,.4,255,cut));

    // 4. 구면·원통 빌보드 축의 방향과 직교 관계를 확인합니다. GPU 정점이나 삼각형은 만들지 않습니다.
    const Axes s=spherical({0,0,0},{0,0,10});
    CHECK(near(s.face,Vec3{0,0,1}));
    CHECK(near(s.right,Vec3{1,0,0}));
    CHECK(near(s.up,Vec3{0,1,0}));
    CHECK(near(dot(s.face,s.right),0));
    CHECK(near(dot(s.right,s.up),0));
    const Axes c=cylindrical({0,0,0},{10,3,0});
    CHECK(near(c.face,Vec3{1,0,0}));
    CHECK(near(c.up,Vec3{0,1,0}));
    CHECK(near(c.right,Vec3{0,0,-1}));

    // 카메라가 바로 위에 있으면 축을 정할 수 없습니다. 임의 보정 대신 예외를 내는 원본 정책을 검증합니다.
    rejected=false;
    try {
        (void)spherical({0,0,0},{0,10,0});
    } catch(const std::invalid_argument&) {
        rejected=true;
    }
    CHECK(rejected);
    rejected=false;
    try {
        (void)cylindrical({0,0,0},{0,10,0});
    } catch(const std::invalid_argument&) {
        rejected=true;
    }
    CHECK(rejected);

    // 5. 전방 깊이 20, 10, 5, 5 순서이며 같은 깊이의 1번·3번은 원래 순서를 유지해야 합니다.
    std::vector<Sprite> queue{{1,{0,0,5}},{2,{0,0,20}},{3,{0,0,5}},{4,{0,0,10}}};
    far_first(queue,{0,0,0},{0,0,1});
    CHECK(queue[0].id==2);
    CHECK(queue[1].id==4);
    CHECK(queue[2].id==1);
    CHECK(queue[3].id==3);

    // 6. 초당 2만큼 움직이는 입자가 1초 수명을 넘겨 이동하지 않는지 확인합니다.
    Particle particle{{0,0,0},{2,0,0},0,1};
    CHECK(alive(particle));
    tick(particle,.25);
    CHECK(near(particle.pos,Vec3{.5,0,0}));
    CHECK(near(particle.age,.25));
    tick(particle,2);
    CHECK(near(particle.pos,Vec3{2,0,0}));
    CHECK(near(particle.age,1));
    CHECK(!alive(particle));
    tick(particle,.1);
    CHECK(near(particle.pos,Vec3{2,0,0}));
    rejected=false;
    try {
        tick(particle,-1);
    } catch(const std::invalid_argument&) {
        rejected=true;
    }
    CHECK(rejected);

    // 출력 문자열은 원본 그대로입니다. 원본·주석판의 성공 결과는 모두 같은 53개 조건입니다.
    std::cout<<"PASS: "<<n<<" explicit assertions (single CPU run)\n";
    return 0;
}
