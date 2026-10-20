Windows/MSVC/D3D11/GPU 미실행. D3D11 자동 View 해제와 달리 애플리케이션 측 안전 정책을 엄격하게 모델링한 참조 구현입니다.
```c++
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// Portable CPU model of explicit application-side pass ownership.
// D3D11 may auto-unbind conflicting views; this test deliberately rejects conflicts.
namespace deferred {
enum Bind { RT=1, SR=2, DS=4 };
enum Format { RGBA8, RGBA16F, Depth24 };
struct Texture { unsigned id,w,h,samples; unsigned flags; Format format; };
class Graph {
  std::map<unsigned,Texture> textures_;
  std::vector<unsigned> rt_; std::vector<unsigned> sr_; unsigned depth_;
  unsigned next_;
  const Texture& find(unsigned id) const {
    std::map<unsigned,Texture>::const_iterator it=textures_.find(id);
    if(it==textures_.end()) throw std::invalid_argument("unknown resource");
    return it->second;
  }
  static bool has(const std::vector<unsigned>& ids,unsigned id) {
    return std::find(ids.begin(),ids.end(),id)!=ids.end();
  }
public:
  Graph():depth_(0),next_(1){}
  unsigned create(unsigned w,unsigned h,unsigned samples,unsigned flags,Format f) {
    if(!w||!h||!samples||!flags||(flags&~(RT|SR|DS))!=0||
       (f==Depth24&&(flags&RT))||(f!=Depth24&&(flags&DS)))
       throw std::invalid_argument("invalid texture descriptor");
    const unsigned id=next_++;textures_[id]={id,w,h,samples,flags,f};return id;
  }
  void bindOM(const std::vector<unsigned>& renderTargets,unsigned depth=0) {
    if(renderTargets.size()>8) throw std::invalid_argument("MRT exceeds D3D11 limit");
    unsigned w=0,h=0,samples=0;
    for(size_t i=0;i<renderTargets.size();++i) {
      const Texture& t=find(renderTargets[i]);
      if(!(t.flags&RT)||has(sr_,t.id)) throw std::invalid_argument("RTV invalid or reading");
      if(std::find(renderTargets.begin(),renderTargets.begin()+i,t.id)!=renderTargets.begin()+i)
        throw std::invalid_argument("duplicate MRT resource");
      if(!i){w=t.w;h=t.h;samples=t.samples;}
      else if(t.w!=w||t.h!=h||t.samples!=samples)
        throw std::invalid_argument("MRT dimensions or multisample mismatch");
    }
    if(depth) {
      const Texture& d=find(depth);
      if(!(d.flags&DS)||has(sr_,d.id)) throw std::invalid_argument("invalid depth");
      if(!renderTargets.empty()&&(d.w!=w||d.h!=h||d.samples!=samples))
        throw std::invalid_argument("depth not compatible");
    }
    rt_=renderTargets;depth_=depth;
  }
  void bindPS(unsigned id) {
    const Texture& t=find(id);
    if(!(t.flags&SR)||has(rt_,id)||depth_==id) throw std::invalid_argument("SRV hazard");
    if(!has(sr_,id)) sr_.push_back(id);
  }
  void unbindPS(){sr_.clear();}
  void unbindOM(){rt_.clear();depth_=0;}
  void resize() const {
    if(!rt_.empty()||!sr_.empty()||depth_) throw std::logic_error("unbinding required before resize");
  }
  bool reads(unsigned id) const {return has(sr_,id);}
  bool writes(unsigned id) const {return has(rt_,id)||depth_==id;}
  size_t targetCount() const {return rt_.size();}
  size_t shaderReadCount() const {return sr_.size();}
};
} // namespace deferred

int main(){
  using namespace deferred;
  unsigned n=0;
  const auto ck=[&](bool yes,const char* why){if(!yes)throw std::runtime_error(why);++n;};
  const auto invalid=[&](const std::function<void()>& fn){try {fn();return false;}catch(const std::invalid_argument&){return true;}};
  const auto logic=[&](const std::function<void()>& fn){try {fn();return false;}catch(const std::logic_error&){return true;}};
  Graph g;
  ck(invalid([&]{g.create(0,100,1,RT,RGBA8);}),"width must be positive");
  ck(invalid([&]{g.create(100,0,1,RT,RGBA8);}),"height must be positive");
  ck(invalid([&]{g.create(100,100,0,RT,RGBA8);}),"samples must be positive");
  ck(invalid([&]{g.create(100,100,1,0,RGBA8);}),"bind flags missing");
  ck(invalid([&]{g.create(100,100,1,8,RGBA8);}),"unknown flag");
  ck(invalid([&]{g.create(100,100,1,RT,Depth24);}),"depth cannot RTV");
  ck(invalid([&]{g.create(100,100,1,DS,RGBA8);}),"color cannot DSV");
  unsigned a=g.create(1024,768,1,RT|SR,RGBA8);
  unsigned b=g.create(1024,768,1,RT|SR,RGBA16F);
  unsigned c=g.create(1024,768,1,RT|SR,RGBA8);
  unsigned d=g.create(1024,768,1,DS,Depth24);
  unsigned small=g.create(512,768,1,RT|SR,RGBA8);
  unsigned msaa=g.create(1024,768,4,RT|SR,RGBA8);
  unsigned noSR=g.create(1024,768,1,RT,RGBA8);
  unsigned noRT=g.create(1024,768,1,SR,RGBA8);
  unsigned depthWrong=g.create(1024,768,4,DS,Depth24);
  ck(a!=b&&b!=c&&c!=d,"IDs unique");
  ck(g.targetCount()==0&&g.shaderReadCount()==0,"empty state");
  ck(invalid([&]{g.bindOM({999});}),"unknown RTV");
  ck(invalid([&]{g.bindOM({noRT});}),"non RTV");
  ck(invalid([&]{g.bindOM({a,a});}),"duplicate MRT");
  ck(invalid([&]{g.bindOM({a,small});}),"different dimensions");
  ck(invalid([&]{g.bindOM({a,msaa});}),"different samples");
  ck(invalid([&]{g.bindOM({a,b},depthWrong);}),"depth samples");
  ck(invalid([&]{g.bindOM({a,b},999);}),"unknown DSV");
  ck(invalid([&]{g.bindOM({a},noRT);}),"not depth resource");
  ck(invalid([&]{g.bindOM({a,b,c,a,b,c,a,b,c});}),"over 8 RTV");
  g.bindOM({a,b,c},d);
  ck(g.targetCount()==3,"geometry pass RTV count");
  ck(g.writes(a)&&g.writes(b)&&g.writes(c)&&g.writes(d),"writes Gbuffer");
  ck(!g.reads(a),"Gbuffer not yet sampled");
  ck(invalid([&]{g.bindPS(a);}),"RTV SRV hazard");
  ck(invalid([&]{g.bindPS(d);}),"depth missing SR flag");
  ck(logic([&]{g.resize();}),"resize while RTV bound");
  g.unbindOM();ck(g.targetCount()==0,"geometry RT unbind");
  g.bindPS(a);g.bindPS(b);g.bindPS(c);
  ck(g.shaderReadCount()==3,"lighting reads Gbuffer");
  ck(g.reads(a)&&g.reads(b)&&g.reads(c),"reading state");
  ck(invalid([&]{g.bindOM({a});}),"cannot write still-read texture");
  ck(invalid([&]{g.bindOM({noSR,a});}),"all writes check hazard");
  ck(logic([&]{g.resize();}),"resize with SRV bound");
  g.bindPS(a);ck(g.shaderReadCount()==3,"idempotent SRV bind");
  ck(invalid([&]{g.bindPS(noSR);}),"no SRV flag");
  ck(invalid([&]{g.bindPS(999);}),"unknown SRV");
  g.unbindPS();ck(g.shaderReadCount()==0,"lighting unbind");
  g.bindOM({a,b,c},d);ck(g.targetCount()==3,"next frame rebind");
  g.unbindOM();g.resize();ck(g.targetCount()==0,"safe resize after release");
  g.bindPS(noRT);ck(g.reads(noRT),"SRV-only resource");
  ck(invalid([&]{g.bindOM({noRT});}),"SRV-only cannot RTV");
  g.unbindPS();
  g.bindOM({noSR});ck(g.writes(noSR),"RTV-only resource");
  ck(invalid([&]{g.bindPS(noSR);}),"RTV-only cannot SRV");
  g.unbindOM();g.resize();ck(g.targetCount()==0,"final reset");
  std::cout<<"PASS "<<n<<" Deferred MRT SRV hazard CPU contract checks\n";
}
```
**실제 테스트:** GCC14.2 C++20·C++11 및 Clang17 C++20 ASan/UBSan 각 41조건, CTest 1/1. 코드는 별도 검증 패키지로도 보관되며 자세한 원시 로그는 18B 후속 첨부로 보관합니다.
공식 계약: [https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetrendertargets](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetrendertargets)
