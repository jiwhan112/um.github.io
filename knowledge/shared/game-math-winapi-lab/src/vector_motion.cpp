#include "math.hpp"
#include "check.hpp"
using namespace study;
int main() {
    Check(Near(Dot(Vec3{1,2,3},Vec3{4,5,6}),32),"dot formula");
    const auto cross=Cross({1,0,0},{0,1,0});
    Check(Near(cross.z,1)&&Near(cross.x,0)&&Near(cross.y,0),"basis cross");
    const Vec3 a{2,-1,4},b{3,5,-2}; const Vec3 c=Cross(a,b);
    Check(Near(Dot(a,c),0)&&Near(Dot(b,c),0),"cross orthogonal");
    Check(Near(Cross(b,a).x,-c.x)&&Near(Cross(b,a).y,-c.y)&&Near(Cross(b,a).z,-c.z),"anti commutative");
    Check(!Normalize(Vec3{}),"zero rejected");
    Check(!Normalize(Vec3{std::numeric_limits<double>::infinity(),0,0}),"infinity rejected");
    Check(!Normalize(Vec2{std::numeric_limits<double>::quiet_NaN(),0}),"NaN rejected");
    const auto n=Normalize(Vec2{3,4});
    Check(n&&Near(n->x,.6)&&Near(n->y,.8),"3 4 normalization");
    const auto diagonal=Normalize(Vec2{1,1});
    Check(diagonal&&Near(std::hypot(diagonal->x,diagonal->y),1),"diagonal speed");
    const auto p=MoveTowards({0,0},{3,4},2,1);
    Check(Near(p.x,1.2)&&Near(p.y,1.6),"two units per second");
    const auto q=MoveTowards({0,0},{3,4},10,1);
    Check(Near(q.x,3)&&Near(q.y,4),"no overshoot");
    Check(Near(MoveTowards({1,2},{1,2},0,1).y,2),"same target");
    Throws([]{ (void)MoveTowards({},{1,1},-1,1); });
    Throws([]{ (void)MoveTowards({},{1,1},1,-1); });
    constexpr double pi=std::numbers::pi;
    Check(Near(WrapAngle(-pi),pi),"opposite convention");
    Check(Near(TurnTowards(170*pi/180,-170*pi/180,5*pi/180,1),175*pi/180),"shortest turn");
    Check(Near(JumpHeight(0,10,-10,1),5),"jump apex");
    Check(Near(JumpHeight(0,10,-10,2),0),"jump return");
    Check(Near(std::atan2(1.0,-1.0),3*pi/4),"atan2 quadrant");
    return Done("vector_motion");
}
