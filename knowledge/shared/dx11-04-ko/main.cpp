// DX11 shader-resource/texture contract - portable CPU model, NOT Direct3D or GPU execution.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

struct RGBA {std::uint8_t r,g,b,a;};
struct TextureDesc {std::size_t width,height,mips;bool srv,rtv;TextureDesc(std::size_t w,std::size_t h,std::size_t m,bool s,bool r):width(w),height(h),mips(m),srv(s),rtv(r){}};
bool Valid(const TextureDesc& d){return d.width>0&&d.height>0&&d.mips>0&&d.width<=16384&&d.height<=16384;}
bool ByteSizeFits(std::size_t w,std::size_t h,std::size_t bpp){
    if(!w||!h||!bpp||w>std::numeric_limits<std::size_t>::max()/bpp) return false;
    return h<=std::numeric_limits<std::size_t>::max()/(w*bpp);
}
bool CopyPitched(const std::vector<std::uint8_t>& src,std::size_t w,std::size_t h, // [한국어 해설] 소스/목적지 Pitch를 별도로 적용하며 패딩은 복사하지 않는다. 같은 버퍼 별칭은 거부한다.
                 std::size_t srcPitch,std::size_t dstPitch,std::vector<std::uint8_t>& dst){
    if(&src==&dst||!ByteSizeFits(w,h,4))return false;
    const std::size_t row=w*4;
    if(srcPitch<row||dstPitch<row||h>std::numeric_limits<std::size_t>::max()/srcPitch||
       h>std::numeric_limits<std::size_t>::max()/dstPitch)return false;
    // Strict buffer capacity model: rows include trailing padding, unlike D3D11 caller-owned mapped lifetimes.
    if(src.size()<srcPitch*h||dst.size()<dstPitch*h)return false;
    for(std::size_t y=0;y<h;++y)
        std::copy(src.begin()+y*srcPitch,src.begin()+y*srcPitch+row,dst.begin()+y*dstPitch);
    return true;
}
std::size_t Clamped(std::int64_t c,std::size_t size){
    if(c<0)return 0;
    if(static_cast<std::uint64_t>(c)>=size)return size-1;
    return static_cast<std::size_t>(c);
}
std::size_t Wrapped(std::int64_t c,std::size_t size){ // [한국어 해설] 음수 나머지를 보정하여 음수 좌표도 양의 텍셀 인덱스로 감싼다.
    const std::int64_t n=static_cast<std::int64_t>(size);
    return static_cast<std::size_t>(((c%n)+n)%n);
}
struct Float4 {float r,g,b,a;};
Float4 Multiply(Float4 a,Float4 b){return {a.r*b.r,a.g*b.g,a.b*b.b,a.a*b.a};}
RGBA Texel(const std::vector<RGBA>& px,std::size_t w,std::size_t h, // [한국어 해설] 정수 텍셀 주소 모델이다. GPU의 선형·밉·이방성 필터링을 재현하지 않는다.
           std::int64_t x,std::int64_t y,bool wrap){
    if(w==0||h==0||w>static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())||
       h>static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())||
       !ByteSizeFits(w,h,sizeof(RGBA))||px.size()<w*h)throw std::invalid_argument("bad sample input");
    std::size_t ix=wrap?Wrapped(x,w):Clamped(x,w);
    std::size_t iy=wrap?Wrapped(y,h):Clamped(y,h);
    return px[iy*w+ix];
}
// Granularity: resource ID + specific mip slice, not all possible D3D11 overlapping view cases.
struct View {int resource;unsigned mip;bool shaderRead;}; // [한국어 해설] 같은 resource ID와 특정 mip 한 장만으로 겹침을 판단하는 제한 모델이다.
bool Overlaps(const View& a,const View& b){return a.resource==b.resource&&a.mip==b.mip;}
bool Hazard(const View& a,const View& b){return Overlaps(a,b)&&a.shaderRead!=b.shaderRead;}
struct Matrix {float m[4][4];};
struct alignas(16) FrameCB {Matrix m; Float4 tint;};
static_assert(sizeof(FrameCB)==80,"constant buffer 80 bytes");
static_assert(sizeof(FrameCB)%16==0,"constant buffer width multiple of 16");
int main(){ // [한국어 해설] Pitch·범위·wrap/clamp·읽기쓰기 뷰 충돌과 상수 배치를 CPU에서 검사한다.
    unsigned checks=0;const auto ck=[&](bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);};
    try{
        ck(Valid({4,4,1,true,false}),"basic SRV texture");
        ck(Valid({4,4,2,true,true}),"multi mip SRV+RTV declaration");
        ck(!Valid({0,4,1,true,false}),"zero width");
        ck(!Valid({4,0,1,true,false}),"zero height");
        ck(!Valid({4,4,0,true,false}),"zero mips");
        ck(!Valid({16385,4,1,true,false}),"declared teaching cap");
        ck(ByteSizeFits(2,2,4),"two by two rgba");
        ck(!ByteSizeFits(std::numeric_limits<std::size_t>::max(),2,4),"width product overflow");
        ck(!ByteSizeFits(1,0,4),"empty image");
        std::vector<std::uint8_t> src={1,2,3,4,5,6,7,8,0xee,0xee,0xee,0xee}; // pitch=4 row content, but third row only padding? 
        std::vector<std::uint8_t> compact={1,2,3,4,5,6,7,8};
        std::vector<std::uint8_t> dst(16,0xcc);
        ck(CopyPitched(compact,1,2,4,8,dst),"copy narrow image into larger pitch");
        ck(dst[0]==1&&dst[3]==4&&dst[8]==5&&dst[11]==8,"two rows positioned correctly");
        ck(dst[4]==0xcc&&dst[15]==0xcc,"destination row padding untouched");
        ck(!CopyPitched(compact,1,2,3,8,dst),"source pitch too small");
        ck(!CopyPitched(compact,1,2,4,3,dst),"dest pitch too small");
        ck(!CopyPitched(compact,1,3,4,8,dst),"source insufficient");
        ck(!CopyPitched(compact,1,2,4,8,compact),"same buffer rejected");
        ck(!CopyPitched(compact,std::numeric_limits<std::size_t>::max(),2,4,8,dst),"overflow rejected");
        ck(!CopyPitched(src,1,3,4,8,dst),"dest insufficient for 3 rows");
        const std::vector<RGBA> tex={{255,0,0,255},{0,255,0,255},{0,0,255,255},{255,255,255,255}};
        ck(Texel(tex,2,2,0,0,false).r==255,"top-left red");
        ck(Texel(tex,2,2,1,0,false).g==255,"top-right green");
        ck(Texel(tex,2,2,0,1,false).b==255,"bottom-left blue");
        ck(Texel(tex,2,2,1,1,false).r==255,"bottom-right white");
        ck(Texel(tex,2,2,-1,0,false).r==255,"clamp negative x");
        ck(Texel(tex,2,2,3,0,false).g==255,"clamp >max x");
        ck(Texel(tex,2,2,-1,0,true).g==255,"wrap negative x");
        ck(Texel(tex,2,2,2,1,true).b==255,"wrap x=width");
        bool threw=false;try{(void)Texel({},0,2,0,0,false);}catch(const std::invalid_argument&){threw=true;}
        ck(threw,"empty texel rejection");
        const Float4 color=Multiply({.5f,1,1,.25f},{1,.25f,.5f,.5f});
        ck(std::abs(color.r-.5f)<1e-6f&&std::abs(color.g-.25f)<1e-6f,"sample*vertex color");
        ck(std::abs(color.b-.5f)<1e-6f&&std::abs(color.a-.125f)<1e-6f,"alpha multiplication");
        ck(Hazard({1,0,true},{1,0,false}),"same mip read/write hazard");
        ck(!Hazard({1,0,true},{1,1,false}),"different mips disjoint in this model");
        ck(!Hazard({1,0,true},{2,0,false}),"different resources");
        ck(!Hazard({1,0,true},{1,0,true}),"two reads no write hazard");
        ck(Overlaps({1,0,true},{1,0,false}),"same subresource overlap");
        ck(sizeof(FrameCB)==80,"CB payload size");
        ck(alignof(FrameCB)>=16,"CB struct alignment");
        std::cout<<"PASS "<<checks<<" texture/pitch/view checks (CPU model only)\n";
    }catch(const std::exception& e){std::cerr<<"FAIL "<<checks<<" "<<e.what()<<'\n';return 1;}
}
