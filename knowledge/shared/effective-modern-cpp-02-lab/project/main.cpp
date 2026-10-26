#include <iostream>
#include <stdexcept>
#include <vector>
#include <type_traits>
#include <cstdint>
int checks = 0;
void Check(bool b, const char * w) {
  ++ checks;
  if (! b) throw std::runtime_error(w);
}
// 암시적 단위 변환과 음수 값을 서로 다른 단계에서 차단한다.
struct Damage {
  int amount;
  explicit Damage(int n) : amount(n) {
    if (n < 0) throw std::invalid_argument("negative");
  }
  Damage(double) = delete;
};
enum class Slot : std::uint8_t {
  Head, Body
};
using DamageList = std::vector < Damage >;
// 파생 클래스가 같은 const 가상 함수 계약을 구현하도록 한다.
struct Rule {
  virtual ~ Rule() = default;
  virtual int Apply(int hp) const = 0;
};
struct Armor final : Rule {
  int Apply(int hp) const override {
    return hp > 2 ? hp - 2 : 0;
  }
};
int Select(int) {
  return 1;
}
int Select(const Rule *) {
  return 2;
}
constexpr int Slots() {
  return 2;
}
int main() {
  // 괄호는 개수/값, 중괄호는 원소 목록 생성자를 선택한다.
  std::vector < int > repeated(3, 7);
  std::vector < int > listed {
    3, 7
  };
  Check(repeated.size() == 3 && repeated[2] == 7, "count constructor");
  Check(listed.size() == 2 && listed[0] == 3, "list constructor");
  static_assert(Slots() == 2);
  static_assert(! std::is_convertible_v < Slot, int >);
  static_assert(! std::is_constructible_v < Damage, double >);
  Check(Select(0) == 1, "zero selects integer");
  Check(Select(nullptr) == 2, "nullptr selects pointer");
  // 정상 생성과 실행 시 값 검증 실패를 따로 확인한다.
  DamageList hits;
  hits.emplace_back(4);
  Check(hits.front().amount == 4, "explicit construction");
  bool rejected = false;
  try {
    Damage bad(- 1);
    (void) bad;
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  Check(rejected, "negative rejected before state");
  Armor armor;
  const Rule & rule = armor;
  Check(rule.Apply(1) == 0 && rule.Apply(8) == 6, "override contract");
  // 읽기 전용 반복자는 이 경로에서의 원소 변경만 제한한다.
  auto first = repeated.cbegin();
  Check(* first == 7, "read-only iterator");
  std::cout << "interface: construction and overloads explicit\nPASS: " << checks << " checks\n";
}
