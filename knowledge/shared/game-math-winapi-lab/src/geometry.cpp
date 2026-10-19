#include "math.hpp"
#include "check.hpp"
using namespace study;
int main() {
    Check(Overlap({{0,0},{2,2}},{{2,1},{3,3}}),"AABB touching is contact");
    Check(!Overlap({{0,0},{2,2}},{{2.01,1},{3,3}}),"AABB separated");
    Check(Overlap({{0,0},{2,2}},{{.5,.5},{1,1}}),"AABB contained");
    Throws([]{ (void)Overlap({{2,0},{1,1}},{{0,0},{1,1}}); });
    Check(CircleOverlap({0,0},1,{2,0},1),"circle touching");
    Check(!CircleOverlap({0,0},1,{2.1,0},1),"circle separated");
    Throws([]{ (void)CircleOverlap({},-1,{},1); });
    const auto vertical=ClosestOnSegment({2,3},{0,0},{0,5});
    Check(Near(vertical.x,0)&&Near(vertical.y,3),"vertical segment");
    const auto endpoint=ClosestOnSegment({2,9},{0,0},{0,5});
    Check(Near(endpoint.y,5),"clamp segment");
    const auto degenerate=ClosestOnSegment({5,6},{1,2},{1,2});
    Check(Near(degenerate.x,1)&&Near(degenerate.y,2),"degenerate segment");
    for (int y=0;y<8;++y) for (int x=0;x<8;++x) {
        const auto picked=PickIso(IsoCenter(x,y,32,16),8,8,32,16);
        Check(picked&&picked->first==x&&picked->second==y,"isometric center roundtrip");
    }
    Check(!PickIso(IsoCenter(-.6,0,32,16),8,8,32,16),"negative outside not truncated to zero");
    const auto edge=PickIso(IsoCenter(.5,0,32,16),8,8,32,16);
    Check(edge&&edge->first==1,"half-open shared boundary");
    Check(!PickIso(IsoCenter(7.5,0,32,16),8,8,32,16),"right outer boundary excluded");
    Throws([]{ (void)PickIso({},2,2,0,16); });
    return Done("geometry");
}
