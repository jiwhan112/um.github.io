#include "fontlab.hpp"

static std::string bytes(std::initializer_list<int> seq) { // [한국어 해설] 원시 UTF-8 오류 입력을 정확한 바이트로 구성한다. 환경 문자열 인코딩에 맡기지 않는다.
  std::string s;
  for(std::initializer_list<int>::const_iterator it=seq.begin();it!=seq.end();++it) s.push_back(static_cast<char>(*it));
  return s;
}

int main() { // [한국어 해설] 디코더·atlas·advance·bearing·줄바꿈을 CPU 검사한다. 글꼴 래스터화나 GPU 출력 검사는 아니다.
  using namespace fontlab;
  unsigned n=0;
  const auto ck=[&](bool b,const char* why) {if(!b) throw std::runtime_error(why);++n;};
  const auto throws=[&](const std::function<void()>& fn) {try {fn();return false;}catch(const std::invalid_argument&){return true;}};
  ck(decode("").empty(),"utf8 empty");
  ck(decode("ABC").size()==3,"ascii");
  ck(decode("A")[0]==U'A',"ascii cp");
  ck(decode(bytes({0xc2,0xa2}))[0]==0xa2,"utf8 2 bytes");
  ck(decode(bytes({0xe2,0x82,0xac}))[0]==0x20ac,"utf8 3 bytes");
  ck(decode(bytes({0xf0,0x9f,0x98,0x80}))[0]==0x1f600,"utf8 4 bytes");
  ck(throws([]{decode(bytes({0x80}));}),"orphan continuation");
  ck(throws([]{decode(bytes({0xc0,0xaf}));}),"overlong 2");
  ck(throws([]{decode(bytes({0xe0,0x80,0x80}));}),"overlong 3");
  ck(throws([]{decode(bytes({0xf0,0x80,0x80,0x80}));}),"overlong 4");
  ck(throws([]{decode(bytes({0xed,0xa0,0x80}));}),"surrogate");
  ck(throws([]{decode(bytes({0xf4,0x90,0x80,0x80}));}),"above max");
  ck(throws([]{decode(bytes({0xf5,0x80,0x80,0x80}));}),"illegal head 5");
  ck(throws([]{decode(bytes({0xe2,0x82}));}),"truncated");
  ck(throws([]{decode(bytes({0xc2})+"A");}),"invalid continuation");
  ck(throws([]{decode(bytes({0xff}));}),"invalid FF");
  ck(scalar(0)&&scalar(0x10ffff),"scalar boundaries");
  ck(!scalar(0xd800)&&!scalar(0xdfff),"surrogate range");
  ck(!scalar(0x110000),"scalar upper");
  ck(inside({0,0,1,1},8,8),"inside origin");
  ck(inside({7,7,1,1},8,8),"inside edge");
  ck(!inside({7,7,2,1},8,8),"outside right");
  ck(!inside({0,0,0,1},8,8),"empty rect");
  ck(!overlap({0,0,2,2},{2,0,2,2}),"touching atlas border");
  ck(overlap({0,0,2,2},{1,1,2,2}),"overlap test");
  ck(throws([]{Atlas a(0,8,10);}),"invalid width");
  ck(throws([]{Atlas a(8,8,0);}),"invalid line");
  Atlas a(64,64,16); // [한국어 해설] 작은 합성 glyph로 위치/UV 계약을 검사한다. 실제 한글 shaping 품질을 검증하는 테스트가 아니다.
  const Glyph ga={U'A',{0,0,8,10},0,2,9},gb={U'B',{8,0,8,10},0,2,9},gq={U'?',{16,0,8,10},0,2,9};
  a.add(ga);a.add(gb);a.add(gq);
  ck(throws([&]{a.add(ga);}),"duplicate glyph");
  ck(throws([&]{a.add({U'C',{7,0,2,10},0,2,9});}),"atlas overlap");
  ck(throws([&]{a.add({U'C',{63,0,2,2},0,2,9});}),"glyph out of atlas");
  ck(throws([&]{a.add({U'C',{32,0,8,10},0,2,0});}),"glyph zero advance");
  ck(throws([&]{a.add({U'C',{32,0,8,10},0,2,INFINITY});}),"glyph infinite advance");
  const Layout empty=a.draw("",1);
  ck(empty.lines==1&&empty.quads.empty()&&empty.maxAdvance==0,"empty layout");
  const Layout one=a.draw("A",1);
  ck(one.quads.size()==1&&one.lines==1,"one quad");
  ck(one.quads[0].x0==0&&one.quads[0].y0==2,"bearing");
  ck(one.quads[0].x1==8&&one.quads[0].y1==12,"pixel dimensions");
  ck(std::abs(one.quads[0].u1-0.125f)<0.0001f,"normalized UV");
  ck(one.maxAdvance==9,"advance");
  const Layout many=a.draw("AB",2);
  ck(many.quads.size()==2&&many.maxAdvance==36,"scaled advance");
  ck(many.quads[1].x0==18&&many.quads[1].y0==4,"second glyph pen");
  const Layout rows=a.draw("A\nB",1);
  ck(rows.lines==2&&rows.quads.size()==2,"line break");
  ck(rows.quads[1].y0==18&&rows.maxAdvance==9,"line height");
  ck(a.draw("Z",1).quads[0].u0==0.25f,"fallback atlas");
  ck(throws([&]{a.draw("A",0);}),"scale 0");
  ck(throws([&]{a.draw("A",INFINITY);}),"scale inf");
  ck(throws([&]{a.draw("A\rB",1);}),"CR normalization");
  ck(throws([&]{a.draw(bytes({0xc0,0xaf}),1);}),"UTF8 checked at layout");
  ck(a.draw("A\n",1).lines==2,"trailing newline");
  ck(a.draw("\n",1).quads.empty(),"newline no quad");
  std::cout<<"PASS "<<n<<" Font UTF-8 atlas layout CPU contract checks\n";
}
