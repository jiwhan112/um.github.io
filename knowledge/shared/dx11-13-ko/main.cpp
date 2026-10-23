// CPU-only D3D11 instancing contract, not a D3D11 runtime implementation.
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct Element{unsigned slot,offset,bytes,stepRate;bool perInstance;};
struct Layout{unsigned vertexStride,instanceStride;std::vector<Element> elements;};
void checkLayout(const Layout& x){ // [한국어 해설] 원래 단일 순회 검증기의 계약이다. 뒤 원소를 먼저 완전히 검사하는 보완판은 별도13B에 보존되어 있다.
 if(!x.vertexStride||!x.instanceStride||x.elements.empty())throw std::invalid_argument("empty layout");
 for(std::size_t i=0;i<x.elements.size();++i){const Element a=x.elements[i];
  const unsigned stride=a.slot==0?x.vertexStride:a.slot==1?x.instanceStride:0;
  if(!stride||!a.bytes||a.offset>stride||a.bytes>stride-a.offset||a.perInstance!=(a.slot==1)||a.stepRate!=(a.perInstance?1u:0u))throw std::invalid_argument("invalid element");
  for(std::size_t j=i+1;j<x.elements.size();++j){const Element b=x.elements[j];
   if(a.slot==b.slot && a.offset < b.offset+b.bytes && b.offset < a.offset+a.bytes)throw std::invalid_argument("overlap");
  }
 }
}
struct Draw{std::vector<unsigned> indices;std::uint64_t vertexCount,instanceCapacity;};
void checkDraw(const Draw& d,std::uint64_t count,std::uint64_t instanceCount,std::uint64_t startIndex,std::int64_t baseVertex,std::uint64_t startInstance){ // [한국어 해설] 정점과 인스턴스의 시작/개수를 구분한다. base가 음수여도 유효 범위일 수 있으므로 안전한 차감으로 검사한다.
 if(startIndex>d.indices.size()||count>d.indices.size()-startIndex||startInstance>d.instanceCapacity||instanceCount>d.instanceCapacity-startInstance)throw std::out_of_range("draw span");
 if(count==0||instanceCount==0)return;
 for(std::uint64_t i=startIndex;i<startIndex+count;++i){const std::uint64_t index=d.indices[static_cast<std::size_t>(i)];
  if(baseVertex<0){const std::uint64_t magnitude=static_cast<std::uint64_t>(-(baseVertex+1))+1;
    if(index<magnitude || index-magnitude>=d.vertexCount)throw std::out_of_range("baseVertex underflow");
  }else{const std::uint64_t base=static_cast<std::uint64_t>(baseVertex);
    if(base>=d.vertexCount||index>=d.vertexCount-base)throw std::out_of_range("baseVertex overflow");
  }
 }
}
struct Span{unsigned first,count;};
struct DynamicInstances { // [한국어 해설] GPU가 사용 중인 범위의 논리 모델이다. 실제 GPU 완료 쿼리나 자원 rename을 실행하지 않는다.
 unsigned capacity,cursor=0;std::vector<Span> inFlight;
 explicit DynamicInstances(unsigned n):capacity(n){if(!n)throw std::invalid_argument("capacity zero");}
 void discard(){cursor=0;inFlight.clear();} // [한국어 해설] 기존 범위를 비우는 CPU 표현이다. 실제 DISCARD 동작의 드라이버 구현을 단정하지 않는다.
 unsigned reserveNoOverwrite(unsigned n){if(n==0||n>capacity||cursor>capacity-n)throw std::out_of_range("reserve size");
  for(const Span r:inFlight)if(cursor<r.first+r.count && r.first<cursor+n)throw std::logic_error("GPU overlap");
  const unsigned start=cursor;cursor+=n;return start;
 }
 void submit(unsigned start,unsigned n){if(!n||start>capacity||n>capacity-start||start+n>cursor)throw std::out_of_range("submit invalid");
   inFlight.push_back({start,n});
 }
};
int main(){unsigned checks=0;auto ck=[&](bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}; // [한국어 해설] stride/slot/step-rate와 극단 정수·인스턴스 예약 경계를 검사한다.
 auto reject=[&](bool ok){ck(ok,"must reject");};
 const Layout standard{20,64,{{0,0,12,0,false},{0,12,8,0,false},{1,0,16,1,true},{1,16,16,1,true},{1,32,16,1,true},{1,48,16,1,true}}};
 checkLayout(standard);ck(true,"valid layout");
 bool bad=false;try{checkLayout({20,64,{{0,0,12,1,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{1,0,16,0,true}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{2,0,16,1,true}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{0,8,16,0,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{0,0,12,0,false},{0,8,8,0,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{{0,0,0,0,false}}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 bad=false;try{checkLayout({20,64,{}});}catch(const std::invalid_argument&){bad=true;}reject(bad);
 Draw d{{0,1,2,2,3,0},4,100};
 checkDraw(d,6,12,0,0,0);ck(true,"draw valid");
 checkDraw(d,3,10,3,0,90);ck(true,"draw subrange");
 checkDraw(d,0,5,0,0,0);ck(true,"zero index count is no-op");
 checkDraw(d,6,0,0,0,0);ck(true,"zero instance count is no-op");
 auto drawReject=[&](std::uint64_t c,std::uint64_t ins,std::uint64_t begin,std::int64_t base,std::uint64_t first){
  bool caught=false;try{checkDraw(d,c,ins,begin,base,first);}catch(const std::out_of_range&){caught=true;}reject(caught);
 };
 drawReject(7,1,0,0,0);drawReject(1,1,6,0,0);drawReject(1,1,7,0,0);
 drawReject(1,1,0,0,100);drawReject(1,2,0,0,99);
 drawReject(6,1,0,1,0);drawReject(6,1,0,-1,0);
 drawReject(1,1,0,std::numeric_limits<std::int64_t>::min(),0);
 drawReject(1,1,0,std::numeric_limits<std::int64_t>::max(),0);
 drawReject(6,std::numeric_limits<std::uint64_t>::max(),0,0,1);
 const Draw offset{{1,2,3},3,1};checkDraw(offset,3,1,0,-1,0);ck(true,"negative base valid with safe offset");
DynamicInstances ring(8);ck(ring.reserveNoOverwrite(3)==0,"first reserve");ring.submit(0,3);ck(ring.inFlight.size()==1,"submit GPU range");
ck(ring.reserveNoOverwrite(2)==3,"second reserve");ring.submit(3,2);ck(ring.inFlight.size()==2,"second submit");
ck(ring.reserveNoOverwrite(3)==5,"last reserve");
bool ex=false;try{ring.reserveNoOverwrite(1);}catch(const std::out_of_range&){ex=true;}reject(ex);
ex=false;try{ring.reserveNoOverwrite(0);}catch(const std::out_of_range&){ex=true;}reject(ex);
ex=false;try{ring.submit(8,1);}catch(const std::out_of_range&){ex=true;}reject(ex);
ex=false;try{ring.submit(7,0);}catch(const std::out_of_range&){ex=true;}reject(ex);
 ex=false;try{ring.submit(10,1);}catch(const std::out_of_range&){ex=true;}reject(ex);
 ring.discard();ck(ring.cursor==0&&ring.inFlight.empty(),"discard models rename");
 ck(ring.reserveNoOverwrite(8)==0,"full range after discard");ring.submit(0,8);
 ex=false;try{ring.reserveNoOverwrite(1);}catch(const std::out_of_range&){ex=true;}reject(ex);
 ex=false;try{DynamicInstances empty(0);}catch(const std::invalid_argument&){ex=true;}reject(ex);
 std::cout<<"PASS "<<checks<<" DX11 instancing CPU contract checks\n";
}
