#include "math.hpp"
#include "check.hpp"
using namespace study;
int main() {
    const Mat4 s=Scale(2,2,1),r=RotationZ(std::numbers::pi/2),t=Translation(10,0,0);
    const Mat4 world=s*r*t; const auto p=Transform({1,0,0,1},world);
    Check(Near(p[0],10)&&Near(p[1],2)&&Near(p[3],1),"row SRT");
    const auto reverse=Transform({1,0,0,1},t*r*s);
    Check(Near(reverse[0],0)&&Near(reverse[1],22),"noncommutative transform");
    const auto direction=Transform({1,0,0,0},world);
    Check(Near(direction[0],0)&&Near(direction[1],2),"direction ignores translation");
    const auto inverse=Inverse(world); Check(inverse.has_value(),"inverse exists");
    const auto restored=Transform(p,*inverse);
    Check(Near(restored[0],1)&&Near(restored[1],0)&&Near(restored[3],1),"point roundtrip");
    const auto identity=world*(*inverse);
    for (std::size_t i=0;i<4;++i) for (std::size_t j=0;j<4;++j)
        Check(Near(identity.m[i][j],i==j?1.0:0.0),"matrix identity");
    Check(!Inverse(Scale(1,0,1)),"singular rejected");
    Mat4 bad=Identity(); bad.m[1][1]=std::numeric_limits<double>::quiet_NaN();
    Check(!Inverse(bad),"NaN inverse rejected");
    const auto hierarchy=Transform({0,0,0,1},Translation(2,0,0)*RotationZ(std::numbers::pi/2)*Translation(10,0,0));
    Check(Near(hierarchy[0],10)&&Near(hierarchy[1],2),"local then parent");
    const Mat4 nonuniform=Scale(2,1,1); const auto normalMatrix=Transpose(*Inverse(nonuniform));
    const auto tangent=Transform({1,1,0,0},nonuniform);
    const auto normal=Transform({-1,1,0,0},normalMatrix);
    Check(Near(tangent[0]*normal[0]+tangent[1]*normal[1],0),"inverse transpose normal");
    const auto wrong=Transform({-1,1,0,0},nonuniform);
    Check(!Near(tangent[0]*wrong[0]+tangent[1]*wrong[1],0),"ordinary normal transform fails");
    Camera2D camera{{10,20},{400,300},2};
    const auto screen=camera.ToScreen({13,24});
    Check(Near(screen.x,406)&&Near(screen.y,292),"world up to screen down");
    const auto worldAgain=camera.ToWorld(screen);
    Check(Near(worldAgain.x,13)&&Near(worldAgain.y,24),"camera inverse");
    Throws([]{ Camera2D badCamera; badCamera.zoom=0; (void)badCamera.ToWorld({}); });
    std::cout<<"SRT(1,0,0,1) = ("<<p[0]<<','<<p[1]<<','<<p[2]<<','<<p[3]<<")\n";
    return Done("transforms");
}
