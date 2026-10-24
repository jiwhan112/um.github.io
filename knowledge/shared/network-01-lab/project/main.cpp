// 2026-10-24 신규 독립 CPU 모델. 미복원된 NET01 원본 코드가 아니다.
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<std::uint8_t>;
using Frames=std::vector<Bytes>;
class Framer {
    std::array<std::uint8_t,4> header_{};
    std::size_t used_=0;
    std::uint32_t length_=0;
    Bytes body_;
    bool reading_body_=false, terminal_=false;
public:
    static constexpr std::size_t max_payload=32, max_feed=64;
    // 입력은 이미 받은 바이트 조각이다. 빈 Feed는 EOF가 아니다.
    Frames feed(const Bytes& input) {
        if(terminal_) throw std::logic_error("terminal parser");
        if(input.size()>max_feed){terminal_=true;throw std::length_error("feed cap");}
        Frames out;
        for(std::uint8_t byte:input){
            if(!reading_body_){
                header_[used_++]=byte;
                if(used_!=4)continue;
                // 각 바이트를 unsigned로 올린 뒤 시프트해 BE32 길이를 읽는다.
                length_=0;
                for(auto b:header_)length_=(length_<<8)|b;
                used_=0;
                if(length_>max_payload){terminal_=true;throw std::length_error("payload cap");}
                if(length_==0){out.push_back({});continue;}
                body_.clear();reading_body_=true;
            }else{
                body_.push_back(byte);
                if(body_.size()==length_){out.push_back(body_);body_.clear();reading_body_=false;}
            }
        }
        return out;
    }
    // 명시적 입력 종료. 미완성 헤더/본문은 조용히 성공시키지 않는다.
    void finish(){
        if(terminal_)throw std::logic_error("terminal parser");
        terminal_=true;
        if(used_!=0 || reading_body_)throw std::runtime_error("truncated frame");
    }
};
Bytes encode(const Bytes& p){
    if(p.size()>Framer::max_payload)throw std::length_error("payload cap");
    auto n=static_cast<std::uint32_t>(p.size());
    Bytes out{static_cast<std::uint8_t>(n>>24),static_cast<std::uint8_t>(n>>16),
              static_cast<std::uint8_t>(n>>8),static_cast<std::uint8_t>(n)};
    out.insert(out.end(),p.begin(),p.end());return out;
}
int checks=0;
void check(bool v){++checks;if(!v)throw std::runtime_error("check failed");}
template<class E,class F> bool raises(F f){try{f();}catch(const E&){return true;}return false;}
int main(){
    const Frames expected{{0,255},{}};
    Bytes wire=encode(expected[0]);auto empty=encode(expected[1]);wire.insert(wire.end(),empty.begin(),empty.end());
    // 10바이트 스트림의 9개 경계마다 자르기/유지: 가능한 분할 512가지를 모두 확인한다.
    for(unsigned mask=0;mask<512;++mask){
        Framer f;Frames got;Bytes chunk;
        for(std::size_t i=0;i<wire.size();++i){
            chunk.push_back(wire[i]);
            if(i+1==wire.size() || (mask&(1u<<i))){auto v=f.feed(chunk);got.insert(got.end(),v.begin(),v.end());chunk.clear();}
        }
        check(got==expected);f.finish();
    }
    {Framer f;check(f.feed({}).empty());f.finish();check(raises<std::logic_error>([&]{f.feed({});}));}
    for(std::size_t n=1;n<6;++n){Framer f;auto w=encode({1,2});f.feed(Bytes(w.begin(),w.begin()+static_cast<std::ptrdiff_t>(n)));check(raises<std::runtime_error>([&]{f.finish();}));}
    {Framer f;check(f.feed(encode(Bytes(32,42)))==Frames{Bytes(32,42)});f.finish();}
    {Framer f;check(raises<std::length_error>([&]{f.feed({255,255,255,255});}));check(raises<std::logic_error>([&]{f.feed({0});}));}
    {Framer f;check(raises<std::length_error>([&]{f.feed(Bytes(65,0));}));}
    check(raises<std::length_error>([]{encode(Bytes(33,0));}));
    // 예외가 발생한 Feed의 결과는 반환되지 않는다. 복구/재전송 대신 연결 모델을 폐기한다.
    {Framer f;Bytes w=encode({1});w.insert(w.end(),{0,0,0,33});check(raises<std::length_error>([&]{f.feed(w);}));}
    std::cout<<"PASS "<<checks<<" framing CPU checks; 512 partitions\n";
}
