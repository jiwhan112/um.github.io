#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

// CPU-only teaching model of serialization cost, NOT real TCP/IP timing.
// [한국어 해설] 몫과 나머지를 이용해 올림 나눗셈의 덧셈 오버플로를 피한다.
std::uint64_t CountChunks(std::uint64_t bytes,std::uint64_t maxPayload){
  if(maxPayload==0)throw std::invalid_argument("maxPayload must be positive");
  return bytes/maxPayload+(bytes%maxPayload!=0 ? 1:0);
}
// [한국어 해설] 고정 오버헤드 곱셈과 합산 전에 남은 uint64 범위로 검사한다.
std::uint64_t WireBytes(std::uint64_t bytes,std::uint64_t maxPayload,
                        std::uint64_t overheadPerChunk){
  const std::uint64_t count=CountChunks(bytes,maxPayload);
  const std::uint64_t limit=(std::numeric_limits<std::uint64_t>::max)();
  if(overheadPerChunk && count>(limit-bytes)/overheadPerChunk)
    throw std::overflow_error("wire size overflow");
  return bytes+count*overheadPerChunk;
}
// [한국어 해설] 바이트를 비트로 바꾸어 명목 bps로 나눈다. 실제 RTT가 아니다.
long double SerializationSeconds(std::uint64_t wireBytes,
                                  std::uint64_t bitsPerSecond){
  if(!bitsPerSecond)throw std::invalid_argument("zero bitrate");
  return static_cast<long double>(wireBytes)*8.0L /
         static_cast<long double>(bitsPerSecond);
}
// [한국어 해설] bit/s에서 Byte/s로 변환할 뿐 유효 처리량을 측정하지 않는다.
long double BytesPerSecond(std::uint64_t bitsPerSecond){
  return static_cast<long double>(bitsPerSecond)/8.0L;
}
struct Test{int n;Test():n(0){}void Check(bool b){++n;if(!b)throw std::runtime_error("failed test");}};
template<class F> bool Throws(F f){try{f();return false;}catch(const std::exception&){return true;}}
bool Near(long double a,long double b){return std::fabs(a-b)<1e-9L;}
// [한국어 해설] 원문의 동일 30개 조건을 재실행한다. 경계·0상한·오버플로를 포함한다.
int main(){
  Test t;
  t.Check(CountChunks(0,100)==0);
  t.Check(CountChunks(1,100)==1);
  t.Check(CountChunks(100,100)==1);
  t.Check(CountChunks(101,100)==2);
  t.Check(CountChunks(200,100)==2);
  t.Check(CountChunks(201,100)==3);
  t.Check(CountChunks(1,1)==1);
  t.Check(Throws([&]{CountChunks(1,0);}));
  t.Check(Throws([&]{CountChunks(0,0);}));
  t.Check(WireBytes(0,100,20)==0);
  t.Check(WireBytes(100,100,20)==120);
  t.Check(WireBytes(101,100,20)==141);
  t.Check(WireBytes(1000,100,20)==1200);
  t.Check(WireBytes(1000,100,0)==1000);
  t.Check(WireBytes(1,1,0)==1);
  t.Check(WireBytes(1,1,20)==21);
  t.Check(Throws([&]{WireBytes(2,1,(std::numeric_limits<std::uint64_t>::max)());}));
  t.Check(Throws([&]{WireBytes((std::numeric_limits<std::uint64_t>::max)(),1,1);}));
  t.Check(WireBytes((std::numeric_limits<std::uint64_t>::max)(),
                    (std::numeric_limits<std::uint64_t>::max)(),0)==
                    (std::numeric_limits<std::uint64_t>::max)());
  t.Check(Near(BytesPerSecond(8000),1000));
  t.Check(Near(BytesPerSecond(8000000),1000000));
  t.Check(Near(SerializationSeconds(1000,8000),1));
  t.Check(Near(SerializationSeconds(100,8000),0.1L));
  t.Check(Near(SerializationSeconds(1000,8000000),0.001L));
  t.Check(Near(SerializationSeconds(0,8),0));
  t.Check(Throws([&]{SerializationSeconds(1,0);}));
  t.Check(Near(SerializationSeconds(WireBytes(1000,100,20),8000),1.2L));
  t.Check(Near(SerializationSeconds(WireBytes(1000,1000,20),8000),1.02L));
  t.Check(CountChunks((std::numeric_limits<std::uint64_t>::max)(),
                      (std::numeric_limits<std::uint64_t>::max)())==1);
  t.Check(WireBytes(3,2,4)==11);
  std::cout<<"PASS "<<t.n<<" network-unit/packetization CPU checks\n";
}
