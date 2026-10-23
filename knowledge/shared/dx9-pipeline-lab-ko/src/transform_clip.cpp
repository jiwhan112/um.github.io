#include "pipeline.hpp"
#include "check.hpp"
// [한국어 해설] 월드→뷰→클립→화면 수치, 여섯 평면 절단, 원시 UV 교점 보간과 실패 입력을 검사한다.
int main(){return Run("transform_clip",[](Checks& c){
    using namespace pipeline;
    constexpr double pi=3.14159265358979323846;
    auto p=PerspectiveLH(pi/2,4.0/3.0,1,11);
    auto w=Translation(1,0,0), v=Translation(0,0,5);
    auto clip=Transform({1,1,0,1},Multiply(Multiply(w,v),p));
    const Vec4 expected{1.5,1,4.4,5};
    for(std::size_t k=0;k<4;++k)c.Near(clip[k],expected[k],"clip result");
    c.That(Inside(clip),"inside frustum");
    auto screen=Project({clip,{}},{0,0,800,600,0,1});
    c.Near(screen.p.x,520,"screen x"); c.Near(screen.p.y,240,"screen y");
    c.Near(screen.depth,.88,"screen depth");
    for (double z:{1.0,2.0,6.0,11.0}) {
        auto q=Transform({0,0,z,1},p);
        c.Near(q[3],z,"clip w equals view z in this projection");
        c.Near(q[2]/q[3],1.1-1.1/z,"depth mapping");
    }
    auto near=Project({Transform({0,0,1,1},p),{}},{100,50,400,300,.2,.8});
    c.Near(near.p.x,300,"viewport x offset"); c.Near(near.p.y,200,"viewport y offset");
    c.Near(near.depth,.2,"min z remap");
    c.Rejects([]{PerspectiveLH(1,1,0,10);},"near zero");
    c.Rejects([]{PerspectiveLH(1,1,5,5);},"near equals far");
    c.Rejects([]{PerspectiveLH(1,0,1,10);},"aspect zero");
    c.Rejects([]{Project({{0,0,0,0},{}},{0,0,800,600,0,1});},"zero w");
    c.Rejects([]{Project({{0,0,0,-1},{}},{0,0,800,600,0,1});},"negative w");
    c.Rejects([]{Project({{0,0,0,1},{}},{0,0,0,600,0,1});},"zero viewport width");
// [한국어 해설] near 평면을 가로지르는 삼각형은 교점 두 개가 생겨 사각형이 될 수 있다.
    const std::array<Vertex,3> crossing{{{{-.5,-.5,-.5,1},{0,0}},{{.5,-.5,.5,1},{1,0}},{{0,.5,.5,1},{.5,1}}}};
    const auto polygon=Clip(crossing); c.That(polygon.size()==4,"near clip creates quad");
    int on_plane=0;
    for(auto q:polygon){c.That(Inside(q.clip,1e-12),"clipped vertex inside");if(std::abs(q.clip[2])<1e-12)++on_plane;}
    c.That(on_plane==2,"two new boundary vertices");
    c.Near(polygon[0].uv.x,.25,"new vertex raw attribute interpolation");
    std::array<Vertex,3> outside=crossing;
    for(auto& q:outside)q.clip[2]=-1;
    c.That(Clip(outside).empty(),"fully outside rejected");
    for(std::size_t plane=0;plane<6;++plane){
        auto q=Vec4{0,0,.5,1};
        if(plane<2)q[0]=plane==0?-2:2;
        else if(plane<4)q[1]=plane==2?-2:2;
        else q[2]=plane==4?-1:2;
        c.That(!Inside(q),"each clip plane rejects outside");
    }
// [한국어 해설] 모든 원래 정점이 바깥이어도 삼각형이 클립 부피를 둘러싸면 보이는 부분이 남는다.
    const std::array<Vertex,3> surround{{{{-3,-3,.5,1},{}},{{3,-3,.5,1},{}},{{0,3,.5,1},{}}}};
    for(auto q:surround)c.That(!Inside(q.clip),"source vertex is outside");
    auto enclosing=Clip(surround);
    c.That(enclosing.size()>=3,"all-outside vertices do not imply invisible triangle");
    for(auto q:enclosing)c.That(Inside(q.clip,1e-12),"frustum intersection survives");
    for(std::size_t plane=0;plane<6;++plane){
        std::array<Vertex,3> crossing_plane{{{{-.25,-.25,.5,1},{0,0}},{{.25,-.25,.5,1},{1,0}},{{0,.25,.5,1},{.5,1}}}};
        auto& q=crossing_plane[0].clip;
        if(plane<2)q[0]=plane==0?-2:2;
        else if(plane<4)q[1]=plane==2?-2:2;
        else q[2]=plane==4?-1:2;
        auto result=Clip(crossing_plane);
        c.That(result.size()>=3,"partial intersection retained for every plane");
        for(auto r:result)c.That(Inside(r.clip,1e-12),"all generated vertices obey planes");
    }
    std::cout<<"local=(1,1,0,1) world=(2,1,0,1) view=(2,1,5,1)\n"
             <<"clip=(1.5,1,4.4,5) ndc=(0.3,0.2,0.88) screen=(520,240,0.88)\n";
});}

