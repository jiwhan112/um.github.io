// Portable DX11 textured quad *CPU contract model*. No graphics API calls.
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct Vtx {float x,y,z,u,v;};
static_assert(sizeof(Vtx)==20,"vertex layout expected");
struct MaterialSlots {bool vs,ps,layout,vb,ib,srv,sampler,rtv;};
struct RowImage {std::size_t width,height,pitch;std::vector<std::uint8_t> bytes;};
static bool ValidateRow(const RowImage& t) { // [한국어 해설] RGBA 행 바이트와 Pitch×높이의 저장 공간을 오버플로 검사 후 확인한다.
 if(!t.width || !t.height || t.width>std::numeric_limits<std::size_t>::max()/4) return false;
 const std::size_t row=t.width*4;
 if(t.pitch<row || t.height>std::numeric_limits<std::size_t>::max()/t.pitch) return false;
 return t.bytes.size()>=t.pitch*t.height;
}
static bool Sample(const RowImage& t,std::size_t x,std::size_t y,std::array<unsigned,4>& out){
 if(!ValidateRow(t)||x>=t.width||y>=t.height)return false;
 const std::size_t off=y*t.pitch+x*4;
 for(unsigned k=0;k<4;++k)out[k]=t.bytes[off+k];
 return true;
}
static bool ValidateDraw(const std::vector<Vtx>& vb,const std::vector<std::uint16_t>& ib, // [한국어 해설] VB/IB 구간의 CPU 계약이다. base는 실제 D3D11 INT 범위 안의 예제 입력을 전제로 읽는다.
 std::size_t start,std::size_t count,std::int64_t base){
 if(vb.empty()||count==0||count%3||start>ib.size()||count>ib.size()-start)return false;
 for(std::size_t i=start;i<start+count;++i){
  const std::int64_t v=base+static_cast<std::int64_t>(ib[i]);
  if(v<0||static_cast<std::uint64_t>(v)>=vb.size())return false;
 }
 return true;
}
static bool Ready(const MaterialSlots& m){return m.vs&&m.ps&&m.layout&&m.vb&&m.ib&&m.srv&&m.sampler&&m.rtv;} // [한국어 해설] IA 스트림·SRV·샘플러·출력 타깃은 서로 다른 바인딩이다. 하나가 있어도 다른 슬롯을 대신하지 못한다.
int main(){unsigned n=0; auto ck=[&](bool b,const char* s){++n;if(!b)throw std::runtime_error(s);}; // [한국어 해설] 사각형 데이터·Pitch·필수 슬롯의 CPU 검사다. 실제 체크무늬 화면은 Windows 예제에서 따로 확인해야 한다.
 const std::vector<Vtx> v={{-1,-1,.5f,0,1},{-1,1,.5f,0,0},{1,-1,.5f,1,1},{1,1,.5f,1,0}};
 const std::vector<std::uint16_t> idx={0,1,2,2,1,3};
 ck(ValidateDraw(v,idx,0,6,0),"quad"); ck(!ValidateDraw(v,idx,0,7,0),"indices overflow");
 ck(!ValidateDraw(v,idx,0,5,0),"not triangles");ck(!ValidateDraw(v,idx,4,3,0),"start out");
 ck(!ValidateDraw(v,idx,0,6,-1),"negative vertex");ck(!ValidateDraw(v,idx,0,6,1),"vertex overflow");
 ck(ValidateDraw(v,{1,2,3},0,3,-1),"relative base");ck(!ValidateDraw(v,idx,0,0,0),"empty draw");
 RowImage img{2,2,12,std::vector<std::uint8_t>(24,0xCC)};
 img.bytes[0]=10;img.bytes[1]=20;img.bytes[2]=30;img.bytes[3]=255;
 img.bytes[12]=40;img.bytes[13]=50;img.bytes[14]=60;img.bytes[15]=255;
 ck(ValidateRow(img),"pitched image");std::array<unsigned,4> px={};
 ck(Sample(img,0,1,px),"sample row");ck(px[0]==40&&px[1]==50&&px[2]==60&&px[3]==255,"row pitch offset");
 ck(Sample(img,0,0,px),"sample origin");ck(px[0]==10,"first row");
 ck(!Sample(img,2,0,px),"out x");ck(!Sample(img,0,2,px),"out y");
 RowImage invalid=img;invalid.pitch=7;ck(!ValidateRow(invalid),"short pitch");
 invalid=img;invalid.bytes.resize(23);ck(!ValidateRow(invalid),"short bytes");
 invalid=img;invalid.width=std::numeric_limits<std::size_t>::max();ck(!ValidateRow(invalid),"width overflow");
 invalid=img;invalid.height=std::numeric_limits<std::size_t>::max();ck(!ValidateRow(invalid),"height overflow");
 invalid=img;invalid.width=0;ck(!ValidateRow(invalid),"zero width");
 invalid=img;invalid.height=0;ck(!ValidateRow(invalid),"zero height");
 MaterialSlots m{true,true,true,true,true,true,true,true};ck(Ready(m),"all bound");
 m.srv=false;ck(!Ready(m),"missing texture SRV");m.srv=true;m.sampler=false;ck(!Ready(m),"missing sampler");
 m.sampler=true;m.vs=false;ck(!Ready(m),"missing VS");m.vs=true;m.rtv=false;ck(!Ready(m),"missing render target");
 // Shader t0/s0 bindings are independent of IA vertex stream and render target.
 m.rtv=true;m.layout=false;ck(!Ready(m),"missing input layout");
 ck(v.size()*sizeof(Vtx)==80,"VB size");ck(idx.size()*sizeof(std::uint16_t)==12,"IB size");
 std::cout<<"PASS "<<n<<" textured rect CPU contract checks\n";
}
