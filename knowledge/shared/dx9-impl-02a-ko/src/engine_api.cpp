#include "engine_api.h"
#include <cmath>
#include <new>

/* Private type; the client cannot know its size or delete it. */
struct Engine_Handle { // [한국어 해설] 라이브러리 내부 타입이다. 생성과 파괴를 같은 모듈에서 처리하여 할당자 경계를 맞춘다.
  double elapsed;
  Engine_Handle() : elapsed(0.0) {}
};

extern "C" {
ENGINE_API Engine_Result ENGINE_CALL Engine_Create(Engine_Handle** out_handle) { // [한국어 해설] nothrow 할당 실패를 OUT_OF_MEMORY로 변환한다. 성공할 때만 out 핸들을 기록한다.
  if (!out_handle || *out_handle != 0) return ENGINE_INVALID_ARGUMENT;
  Engine_Handle* p = new (std::nothrow) Engine_Handle();
  if (!p) return ENGINE_OUT_OF_MEMORY;
  *out_handle = p;
  return ENGINE_SUCCESS;
}

ENGINE_API Engine_Result ENGINE_CALL Engine_Destroy(Engine_Handle** handle) { // [한국어 해설] delete 후 호출자의 핸들을 null로 만든다. 다른 복사본의 댕글링 포인터까지 무효화해 주지는 않는다.
  if (!handle) return ENGINE_INVALID_ARGUMENT;
  delete *handle;  // delete NULL is safe; allocation and free both happen here.
  *handle = 0;
  return ENGINE_SUCCESS;
}

ENGINE_API Engine_Result ENGINE_CALL Engine_Tick(Engine_Handle* handle, // [한국어 해설] 입력을 먼저 검사하고 성공 경로에서만 상태를 갱신한다. 무한히 누적한 시간의 범용 오버플로 보장은 없다.
                                                 double delta_seconds) {
  if (!handle || !std::isfinite(delta_seconds) ||
      delta_seconds < 0.0 || delta_seconds > 1.0) {
    return ENGINE_INVALID_ARGUMENT;
  }
  handle->elapsed += delta_seconds;
  return ENGINE_SUCCESS;
}

ENGINE_API Engine_Result ENGINE_CALL Engine_GetElapsed(
    const Engine_Handle* handle, double* out_seconds) {
  if (!handle || !out_seconds) return ENGINE_INVALID_ARGUMENT;
  *out_seconds = handle->elapsed;
  return ENGINE_SUCCESS;
}

ENGINE_API Engine_Result ENGINE_CALL Engine_Reset(Engine_Handle* handle) {
  if (!handle) return ENGINE_INVALID_ARGUMENT;
  handle->elapsed = 0.0;
  return ENGINE_SUCCESS;
}
}  // extern "C"
