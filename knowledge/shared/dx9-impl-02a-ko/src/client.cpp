#include "engine_api.h"
#include <iostream>

// Non-copyable owning wrapper. A caller never calls delete on Engine_Handle.
class EngineOwner { // [한국어 해설] 클라이언트 RAII 래퍼는 엔진의 파괴 API를 호출한다. opaque 타입에 직접 delete하지 않는다.
 public:
  EngineOwner() : handle_(0) {}
  ~EngineOwner() { Engine_Destroy(&handle_); }
  EngineOwner(const EngineOwner&) = delete;
  EngineOwner& operator=(const EngineOwner&) = delete;
  Engine_Handle** Address() { return &handle_; }
  Engine_Handle* Get() const { return handle_; }
 private:
  Engine_Handle* handle_;
};

int main() { // [한국어 해설] 공유 라이브러리에 두 tick을 전달하고 누적 0.05초를 확인하는 최소 클라이언트다.
  EngineOwner engine;
  if (Engine_Create(engine.Address()) != ENGINE_SUCCESS) return 1;
  if (Engine_Tick(engine.Get(), 0.016) != ENGINE_SUCCESS) return 2;
  if (Engine_Tick(engine.Get(), 0.034) != ENGINE_SUCCESS) return 3;
  double seconds = -1;
  if (Engine_GetElapsed(engine.Get(), &seconds) != ENGINE_SUCCESS) return 4;
  std::cout << "Elapsed: " << seconds << " seconds\n";
  return (seconds > 0.049999 && seconds < 0.050001) ? 0 : 5;
}
