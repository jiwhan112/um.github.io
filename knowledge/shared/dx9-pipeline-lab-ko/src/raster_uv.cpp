#include "pipeline.hpp"
#include "check.hpp"
// [한국어 해설] 공유 변 소유권, 원근 UV와 화면 선형 깊이, 텍셀 중심 및 잘못된 입력을 검사한다.
int main(){return Run("raster_uv",[](Checks& c){
    using namespace pipeline;
    const std::array<Vec2,3> a{{{0,0},{4,0},{4,4}}},b{{{0,0},{4,4},{0,4}}};
    int total=0;
// [한국어 해설] 정수 픽셀 중심 샘플에서 사각형 내부는 정확히 한 삼각형만 소유해야 한다.
    for(int y=-1;y<=5;++y)for(int x=-1;x<=5;++x){
        const Vec2 sample{static_cast<double>(x),static_cast<double>(y)};
        const int covered=static_cast<int>(Covers(a,sample))+static_cast<int>(Covers(b,sample));
        const int wanted=(x>=0 && x<4 && y>=0 && y<4)?1:0;
        c.That(covered==wanted,"shared edge has exactly one owner");total+=covered;
    }
    c.That(total==16,"quad covers 16 integer pixel centers");
    auto reversed=a;std::swap(reversed[1],reversed[2]);
    c.That(Covers(reversed,{2,1})==Covers(a,{2,1}),"coverage invariant without culling");
    c.That(Edge(a[0],a[1],a[2])>0 && Edge(reversed[0],reversed[1],reversed[2])<0,"winding changes sign");
// [한국어 해설] 서로 다른 1/w를 사용하므로 UV의 단순 보간과 원근 보정 값이 달라진다.
    std::array<ScreenVertex,3> t{{{{0,0},.2,1,{0,0}},{{4,0},.6,.5,{1,0}},{{0,4},.9,.25,{0,1}}}};
    auto l=Barycentric({t[0].p,t[1].p,t[2].p},{1,1});c.That(l.has_value(),"nondegenerate triangle");
    c.Near((*l)[0],.5,"weight0");c.Near((*l)[1],.25,"weight1");c.Near((*l)[2],.25,"weight2");
    auto uv=PerspectiveUV(t,*l);c.Near(uv.x,2.0/11,"perspective u");c.Near(uv.y,1.0/11,"perspective v");
    c.Near(AffineDepth(t,*l),.475,"depth uses screen-linear z/w");
    c.That(!Barycentric({Vec2{0,0},Vec2{1,1},Vec2{2,2}},{1,1}),"degenerate rejected");
// [한국어 해설] 레벨0 1차원 샘플러 모델로 중심 정렬을 설명한다. GPU 텍스처 출력 캡처는 아니다.
    const std::array<double,4> texels{0,1,0,1};
    for(int i=0;i<4;++i)c.Near(LinearClamp(texels,(i+.5)/4),texels[static_cast<std::size_t>(i)],"texel center");
    c.Near(LinearClamp(texels,.25),.5,"uncorrected pixel mixes texels");
    c.Near(LinearClamp(texels,(1+.5)/4),1,"minus half-pixel vertex alignment");
    c.Near(LinearClamp(texels,-1),0,"left clamp");c.Near(LinearClamp(texels,2),1,"right clamp");
    c.Rejects([]{LinearClamp({},0);},"empty texture");
    const double nan=std::numeric_limits<double>::quiet_NaN();
    c.That(!Covers(a,{nan,0}),"nonfinite sample is not covered");
    c.That(!Barycentric(a,{0,nan}),"nonfinite barycentric input rejected");
    std::cout<<"weights=(0.5,0.25,0.25) affine_uv=(0.25,0.25) perspective_uv=(2/11,1/11) depth=0.475\n";
});}

