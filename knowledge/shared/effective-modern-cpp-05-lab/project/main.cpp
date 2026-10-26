#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <concepts>
#include <utility>
#include <vector>
#include <initializer_list>
int checks = 0;
void Check(bool b, const char * w) {
  ++ checks;
  if (! b) throw std::runtime_error(w);
}
// 문자열을 소유하되 복사 생성자의 역할은 전달 생성자에 맡기지 않는다.
class Label {
  std::string text_;
  public : Label(const Label &) = default;
  Label(Label &&) noexcept = default;
  // 자기 타입을 배제하여 forwarding 생성자가 복사를 가로채지 못하게 한다.
  template < class S > requires(! std::same_as < std::remove_cvref_t < S >, Label > && std::constructible_from < std::string, S && >) explicit Label(S && s) : text_(std::forward < S >(s)) {
  }
  const std::string & text() const {
    return text_;
  }
};
int Sum(std::initializer_list < int > xs) {
  int result = 0;
  for (int n : xs) result += n;
  return result;
}
template < class F, class T > auto Call(F f, T && arg) {
  return f(std::forward < T >(arg));
}
int main() {
  // 자기 타입 복사가 일반 템플릿에 가로채이지 않는지 확인한다.
  Label a("enemy");
  Label b(a);
  Check(b.text() == "enemy", "copy not hijacked");
  Label c(std::string("player"));
  Check(c.text() == "player", "owned temporary string");
  static_assert(! std::is_constructible_v < Label, int >);
  static_assert(std::is_copy_constructible_v < Label >);
  // 타입이 있는 지역 목록의 수명 안에서 동기 호출한다.
  std::initializer_list < int > values = {
    1, 2, 3
  };
  Check(Call(Sum, values) == 6, "named list has type");
  Check(Sum( {
    4, 5
  }) == 9, "direct list works");
  // Call(Sum,{1,2,3})는 T를 추론할 수 없다. 명명한 목록을 전달한다.
  // 비트 필드에 참조를 붙이지 않고 명시적인 값 스냅샷을 만든다.
  struct Bits {
    unsigned value : 3;
  };
  Bits bits {
    5
  };
  unsigned materialized = bits.value;
  auto twice =[](unsigned x) {
    return x * 2;
  };
  Check(Call(twice, materialized) == 10, "bit-field materialized");
  const std::vector < int > empty;
  Check(empty.empty(), "empty is a normal case");
  std::cout << "forwarding: constrain inputs and materialize exceptions\nPASS: " << checks << " checks\n";
}
