// 2026-10-24 신규 독립 모델: NET03 미복원 원본과 같은 프로그램이라고 주장하지 않는다.
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<std::uint8_t>;
struct Packet{std::uint16_t seq;std::uint8_t type;Bytes payload;};
constexpr std::size_t cap=1200;
void be16(Bytes& b,std::uint16_t v){b.push_back(static_cast<std::uint8_t>(v>>8));b.push_back(static_cast<std::uint8_t>(v));}
std::uint16_t read16(const Bytes& b,std::size_t p){return static_cast<std::uint16_t>((static_cast<unsigned>(b[p])<<8)|b[p+1]);}
Bytes encode(const Packet& p){
    if(p.type>3 || p.payload.size()>cap)throw std::invalid_argument("packet contract");
    Bytes out;be16(out,p.seq);be16(out,static_cast<std::uint16_t>(p.payload.size()));out.push_back(p.type);
    out.insert(out.end(),p.payload.begin(),p.payload.end());return out;
}
Packet decode(const Bytes& b){
    // UDP 헤더가 아니라 앱 헤더5바이트다. OS의 잘림 여부는 호출자 책임이다.
    if(b.size()<5 || b.size()>5+cap)throw std::invalid_argument("datagram size");
    const auto length=read16(b,2);
    if(length>cap || b[4]>3 || b.size()!=5+static_cast<std::size_t>(length))throw std::invalid_argument("header mismatch");
    return Packet{read16(b,0),b[4],Bytes(b.begin()+5,b.end())};
}
enum class Order {same,newer,older,ambiguous};
Order order(std::uint16_t candidate,std::uint16_t last){
    // unsigned16 변환으로 modulo 65536 차이를 얻는다. signed narrowing에 기대지 않는다.
    auto delta=static_cast<std::uint16_t>(candidate-last);
    if(delta==0)return Order::same;
    if(delta==32768)return Order::ambiguous;
    return delta<32768?Order::newer:Order::older;
}
class Receiver {
    bool initialized_=false;std::uint16_t last_=0;
public:
    bool accept(const Bytes& bytes){
        Packet p=decode(bytes); // 검증 실패 시 last_는 변경되지 않는다.
        if(initialized_ && order(p.seq,last_)!=Order::newer)return false;
        last_=p.seq;initialized_=true;return true;
    }
};
int checks=0;
void check(bool v){++checks;if(!v)throw std::runtime_error("check failed");}
template<class F>bool rejects(F f){try{f();}catch(const std::invalid_argument&){return true;}return false;}
int main(){
    // 결정적 다양한 길이/이진 값의 codec 왕복. 네트워크를 전송하지 않는다.
    for(unsigned i=0;i<2048;++i){
        Packet p{static_cast<std::uint16_t>(i*37),static_cast<std::uint8_t>(i%4),Bytes(i%1201)};
        for(std::size_t j=0;j<p.payload.size();++j)p.payload[j]=static_cast<std::uint8_t>(j+i);
        auto q=decode(encode(p));check(q.seq==p.seq && q.type==p.type && q.payload==p.payload);
    }
    check(encode({0x1234,2,{0,255}})==Bytes({0x12,0x34,0,2,2,0,255}));
    for(std::size_t n=0;n<5;++n)check(rejects([&]{decode(Bytes(n));}));
    check(decode(encode({0,0,Bytes(1200,1)})).payload.size()==1200);
    check(rejects([]{encode({0,0,Bytes(1201)});}));
    check(rejects([]{encode({0,4,{}});}));
    for(const Bytes& b:std::vector<Bytes>{{0,1,0,1,0},{0,1,0,0,0,9},{0,1,0,0,4},{0,1,4,177,0}})check(rejects([&]{decode(b);}));
    check(order(0,65535)==Order::newer);check(order(65535,0)==Order::older);
    check(order(1,1)==Order::same);check(order(32768,0)==Order::ambiguous);check(order(0,32768)==Order::ambiguous);
    check(order(32767,0)==Order::newer);check(order(32769,0)==Order::older);
    // 70000개 순차 번호는 한 번 이상 wrap한다. 같은 패킷 재전달은 거부한다.
    Receiver r;
    for(unsigned i=0;i<70000;++i){auto b=encode({static_cast<std::uint16_t>(i),0,{}});check(r.accept(b));check(!r.accept(b));}
    Receiver fresh;check(fresh.accept(encode({10,0,{}})));
    check(rejects([&]{fresh.accept({0,11,0,1,0});}));check(fresh.accept(encode({11,0,{}})));
    check(!fresh.accept(encode({10,0,{}})));check(!fresh.accept(encode({static_cast<std::uint16_t>(11+32768),0,{}})));
    std::cout<<"PASS "<<checks<<" datagram CPU checks; 2048 roundtrips; 70000 sequence steps\n";
}
