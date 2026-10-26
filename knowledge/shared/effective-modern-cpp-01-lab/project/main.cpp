#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <utility>
int checks = 0;
void Check(bool ok, const char * why) {
  ++ checks;
  if (! ok) throw std::runtime_error(why);
}
// 반환 표현식의 참조 성질을 의도적으로 보존한다. 소유자는 호출자다.
template < class C > decltype(auto) Slot(C & c, std::size_t i) {
  return c.at(i);
}
int main() {
  // 값 복사와 읽기 전용 빌림은 서로 다른 저장 정책이다.
  const int budget = 60;
  auto copy = budget;
  const auto & borrow = budget;
  static_assert(std::is_same_v < decltype(copy), int >);
  static_assert(std::is_same_v < decltype(borrow), const int & >);
  static_assert(std::is_same_v < decltype((budget)), const int & >);
  copy = 30;
  Check(budget == 60 && copy == 30, "independent value");
  // 배열의 길이를 유지하는 참조와 첫 원소 주소를 비교한다.
  int values[3] = {
    1, 2, 3
  };
  auto pointer = values;
  auto & array = values;
  static_assert(std::is_same_v < decltype(pointer), int * >);
  static_assert(std::extent_v < std::remove_reference_t < decltype(array) >> == 3);
  Check(pointer == & values[0], "array decays by value");
  // 이 범위에서는 컨테이너를 재할당하지 않아 원소 참조가 유효하다.
  std::vector < int > health {
    90, 70
  };
  auto detached = Slot(health, 0);
  auto & live = Slot(health, 0);
  live = 80;
  detached = 12;
  Check(health[0] == 80 && detached == 12, "value versus reference");
  // vector<bool>는 bool&가 아닌 대리 객체를 돌려준다.
  std::vector < bool > flags {
    true, false
  };
  auto proxy = flags[0];
  bool snapshot = flags[0];
  flags[0] = false;
  Check(! static_cast < bool >(proxy), "proxy observes live bit");
  Check(snapshot, "explicit bool keeps old value");
  proxy = true;
  Check(flags[0], "proxy writes through");
  // 컨테이너가 사라진 뒤 proxy나 live를 사용하지 않는다.
  // 범위 밖 인덱스는 메모리 접근 전에 예외로 거절되어야 한다.
  bool rejected = false;
  try {
    (void) Slot(health, 9);
  } catch (const std::out_of_range &) {
    rejected = true;
  }
  Check(rejected, "bounds failure");
  std::cout << "deduction: value / reference / proxy separated\nPASS: " << checks << " checks\n";
}
