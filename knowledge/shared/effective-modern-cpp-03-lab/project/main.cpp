#include <utility>
#include <iostream>
#include <memory>
#include <stdexcept>
int checks = 0;
void Check(bool b, const char * w) {
  ++ checks;
  if (! b) throw std::runtime_error(w);
}
// 단일 스레드 실습에서 살아 있는 CPU 객체 수만 추적한다.
struct Texture {
  static inline int alive = 0;
  int id;
  explicit Texture(int n) : id(n) {
    ++ alive;
  }
  ~ Texture() {
    -- alive;
  }
};
// 리소스는 공유 소유하고 타깃은 생존을 연장하지 않는 관찰자로 둔다.
struct Entity {
  std::shared_ptr < Texture > texture;
  std::weak_ptr < Entity > target;
};
int Read(const std::weak_ptr < Texture > & ref) {
  // lock 한 번으로 살아 있는 동안의 임시 소유권을 얻는다.
  auto strong = ref.lock();
  return strong ? strong->id : - 1;
}
int main() {
  // 단독 소유자를 옮기면 원래 포인터는 비워진다.
  {
    auto staging = std::make_unique < Texture >(7);
    auto upload = std::move(staging);
    Check(! staging && upload->id == 7, "exclusive transfer");
  }
  Check(Texture::alive == 0, "exclusive lifetime ends");
  std::weak_ptr < Texture > observer;
  {
    auto asset = std::make_shared < Texture >(42);
    observer = asset;
    auto a = std::make_shared < Entity >();
    auto b = std::make_shared < Entity >();
    a->texture = asset;
    b->texture = asset;
    a->target = b;
    b->target = a;
    Check(Read(observer) == 42, "observer promotes");
    // 지역 asset을 비워도 엔티티의 강한 소유권은 남아 있다.
    asset.reset();
    Check(Read(observer) == 42, "entities keep resource alive");
    // 약한 타깃 간선은 제거되는 엔티티를 붙잡지 않는다.
    b.reset();
    Check(a->target.expired(), "weak graph edge does not retain entity");
    Check(Read(observer) == 42, "remaining owner retains asset");
    // 마지막 강한 리소스 소유자가 사라지면 Texture가 소멸한다.
    a.reset();
    Check(observer.expired(), "last owner releases texture");
  }
  Check(Read(observer) == - 1, "expired handled");
  Check(Texture::alive == 0, "all resources released");
  // get() 주소에서 별도 shared_ptr를 만들면 제어 블록이 둘이 된다. 여기서는 실행하지 않는다.
  std::cout << "ownership: strong resource / weak observer\nPASS: " << checks << " checks\n";
}
