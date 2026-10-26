#include <type_traits>
#include <utility>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <functional>
#include <string>
int checks = 0;
void Check(bool b, const char * w) {
  ++ checks;
  if (! b) throw std::runtime_error(w);
}
// 실제 UI가 아닌 CPU 상태 객체다.
struct Hud {
  int health = 100;
};
std::function < int() > Observe(std::weak_ptr < Hud > hud) {
  return[hud] {
    auto owner = hud.lock();
    return owner ? owner->health : - 1;
  };
}
int main() {
  // 같은 외부 값이라도 캡처 정책에 따라 관찰 시점이 달라진다.
  int live = 10;
  auto snapshot =[live] {
    return live;
  };
  auto local =[& live] {
    return live;
  };
  live = 20;
  Check(snapshot() == 10, "value snapshot");
  Check(local() == 20, "bounded reference");
  // 콜백이 더 오래 살면 참조가 아니라 소유권 또는 약한 관찰자를 저장한다.
  // 콜백보다 HUD가 먼저 사라지는 정상 종료 경로도 검사한다.
  auto hud = std::make_shared < Hud >();
  auto callback = Observe(hud);
  Check(callback() == 100, "live owner");
  hud->health = 75;
  Check(callback() == 75, "current value");
  hud.reset();
  Check(callback() == - 1, "expired owner handled");
  // 이동 캡처 뒤에는 closure가 자원의 유일한 소유자다.
  auto payload = std::make_unique < std::string >("upload");
  auto oneShot =[p = std::move(payload)]() mutable {
    if (! p) return std::string("empty");
    std::string value = std::move(* p);
    p.reset();
    return value;
  };
  Check(! payload, "init capture owns payload");
  Check(oneShot() == "upload", "first use");
  Check(oneShot() == "empty", "repeat handled");
  // 이 closure는 이동 전용이다. C++20 std::function에는 넣지 않는다.
  static_assert(! std::is_copy_constructible_v < decltype(oneShot) >);
  std::cout << "capture: snapshot / borrow / weak / move ownership\nPASS: " << checks << " checks\n";
}
