#include <iostream>
#include <stdexcept>
#include <utility>
#include <type_traits>
int checks = 0;
void Check(bool b, const char * w) {
  ++ checks;
  if (! b) throw std::runtime_error(w);
}
// 계수기는 이 예제의 명시적인 생성 작업만 관찰한다.
struct Packet {
  static inline int copies = 0, moves = 0;
  int id;
  explicit Packet(int n) : id(n) {
  }
  Packet(const Packet & p) : id(p.id) {
    ++ copies;
  }
  Packet(Packet && p) noexcept : id(std::exchange(p.id, - 1)) {
    ++ moves;
  }
};
// T가 추론되는 T&&만 이 문맥에서 forwarding reference다.
template < class T > Packet Relay(T && value) {
  return Packet(std::forward < T >(value));
}
Packet Consume(Packet && value) {
  return Packet(std::move(value));
}
int main() {
  // lvalue 호출자는 원본을 계속 사용하므로 복사되어야 한다.
  Packet source(3);
  auto a = Relay(source);
  Check(a.id == 3 && source.id == 3 && Packet::copies == 1, "lvalue copied");
  // 소비를 허락한 입력은 이동 생성자가 선택된다.
  auto b = Relay(std::move(source));
  Check(b.id == 3 && source.id == - 1 && Packet::moves == 1, "rvalue moved");
  // move는 const를 제거하지 않으므로 이 경로는 복사한다.
  const Packet immutable(9);
  auto c = Relay(std::move(immutable));
  Check(c.id == 9 && Packet::copies == 2, "const rvalue copies");
  // rvalue reference 변수도 이름으로 사용하면 lvalue 표현식이다.
  Packet temp(5);
  Packet && named = std::move(temp);
  static_assert(std::is_lvalue_reference_v < decltype((named)) >);
  Packet d(named);
  Check(Packet::copies == 3 && d.id == 5, "named rvalue reference is lvalue");
  auto e = Consume(std::move(temp));
  Check(e.id == 5 && temp.id == - 1, "explicit consuming boundary");
  static_assert(std::is_nothrow_move_constructible_v < Packet >);
  Check(Packet::moves == 2, "exact explicit moves");
  std::cout << "move: casts select operations, const is retained\nPASS: " << checks << " checks\n";
}
