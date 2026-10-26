#include "asset_cache.hpp"
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>
int checks = 0;
void Check(bool b, const char * w) {
  ++ checks;
  if (! b) throw std::runtime_error(w);
}
int main() {
  // 빈 캐시에서 시작해 중복 삽입과 조회 정책을 확인한다.
  AssetCache cache;
  Check(cache.Size() == 0, "empty");
  Check(cache.Add("hero", 4), "first insert");
  Check(! cache.Add("hero", 9), "duplicate keeps original");
  Check(cache.Find("hero") == 4 && cache.Size() == 1, "map invariant");
  Check(cache.Find("none") == - 1, "missing key");
  // 실패한 값은 기존 원소 수를 바꾸지 않아야 한다.
  bool rejected = false;
  try {
    cache.Add("bad", - 1);
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  Check(rejected && cache.Size() == 1, "validation before mutation");
  // 이동 후 원본의 허용 연산도 클래스 계약으로 검사한다.
  AssetCache moved(std::move(cache));
  Check(moved.Find("hero") == 4, "pimpl moved");
  Check(cache.Size() == 0, "moved-from query contract");
  Check(cache.Add("rebuilt", 2), "moved-from object reusable");
  // 복사 불가와 예외 없는 이동은 컴파일 시점의 별도 검사다.
  static_assert(! std::is_copy_constructible_v < AssetCache >);
  static_assert(std::is_nothrow_move_constructible_v < AssetCache >);
  std::cout << "integration: explicit ownership and narrow build boundary\nPASS: " << checks << " checks\n";
}
