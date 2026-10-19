#include "pipeline.hpp"
#include "check.hpp"
int main(){return Run("depth_blend",[](Checks& c){
    using namespace pipeline;
    const Color red{1,0,0,1},blue{0,0,1,1},half_red{1,0,0,.5},half_blue{0,0,1,.5};
    Pixel p{blue,1};c.That(Fragment(p,red,.4,{}),"near fragment accepted");
    c.That(!Fragment(p,blue,.8,{}),"occluded fragment rejected");c.Near(p.depth,.4,"depth stored");
    Pixel a{{0,0,0,1},1};OutputState translucent{true,false,true,false,-1};
    Fragment(a,half_blue,.8,translucent);Fragment(a,half_red,.2,translucent);
    c.Near(a.color.r,.5,"back to front red");c.Near(a.color.b,.25,"back to front blue");c.Near(a.depth,1,"transparent no z writes");
    Pixel b{{0,0,0,1},1};Fragment(b,half_red,.2,translucent);Fragment(b,half_blue,.8,translucent);
    c.Near(b.color.r,.25,"reversed red differs");c.Near(b.color.b,.5,"reversed blue differs");
    Pixel hole{blue,1};auto bad=translucent;bad.depth_write=true;
    Fragment(hole,{1,0,0,0},.2,bad);
    c.Near(hole.color.b,1,"zero alpha preserves color");c.Near(hole.depth,.2,"zero alpha can still write z");
    c.That(!Fragment(hole,red,.5,{}),"invisible foreground blocks later opaque");
    Pixel cut{blue,1};bad.alpha_cutoff=.5;
    c.That(!Fragment(cut,{1,0,0,0},.2,bad),"alpha test rejects");c.Near(cut.depth,1,"cutout did not write z");
    Pixel s{blue,1},m{blue,1};Fragment(s,half_red,.2,translucent);
    auto pm=translucent;pm.premultiplied=true;Fragment(m,{.5,0,0,.5},.2,pm);
    c.Near(s.color.r,m.color.r,"straight and premultiplied match with matching factors");
    c.Near(s.color.b,m.color.b,"destination factor matches");
    c.Near(s.color.a,1,"explicit separate alpha source-over");
    Pixel no_z{blue,.1};OutputState disabled{false,true,false,false,-1};
    c.That(Fragment(no_z,red,.8,disabled),"depth disabled bypasses test");
    c.Near(no_z.depth,.1,"depth disabled also suppresses writes");
    c.Rejects([&]{Fragment(no_z,red,-.1,{});},"invalid depth rejected");
    std::cout<<"transparent back-to-front RGB=(0.5,0,0.25), reverse=(0.25,0,0.5)\n";
});}

