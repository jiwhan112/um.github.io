#include <fontlab.hpp>
#include <iostream>
#ifndef EXPECT_REJECTION
#define EXPECT_REJECTION 1
#endif
// 한국어 해설: 원본은 0, 음수 높이를 허용하지만 수정판은 거절해야 한다.
int main() {
  using namespace fontlab;
  unsigned checks = 0;
  const auto check = [&](bool value) { if (!value) throw std::runtime_error("height regression"); ++checks; };
  const bool reject = EXPECT_REJECTION != 0;
  check(inside({0,0,8,0},64,64) == !reject);
  check(inside({0,0,8,-1},64,64) == !reject);
  for (int height : {0,-1}) {
    Atlas atlas(64,64,16); bool rejected = false;
    try { atlas.add({U'A',{0,0,8,height},0,2,9}); }
    catch (const std::invalid_argument&) { rejected = true; }
    check(rejected == reject);
  }
  // 한국어 해설: 양수 높이의 기존 동작과 quad 크기는 그대로 유지한다.
  check(inside({0,0,8,10},64,64));
  Atlas atlas(64,64,16); atlas.add({U'A',{0,0,8,10},0,2,9});
  const Layout layout = atlas.draw("A",1);
  check(layout.quads.size()==1 && layout.quads[0].y1==12 && layout.maxAdvance==9);
  std::cout << "PASS " << checks << " height boundary checks; rejection=" << reject << "\n";
}
