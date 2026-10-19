#include "engine_api.h"
#include <cmath>
#include <new>

/* Private type; the client cannot know its size or delete it. */
struct Engine_Handle {
  double elapsed;
  Engine_Handle() : elapsed(0.0) {}
};

extern "C" {
ENGINE_API Engine_Result ENGINE_CALL Engine_Create(Engine_Handle** out_handle) {
  if (!out_handle || *out_handle != 0) return ENGINE_INVALID_ARGUMENT;
  Engine_Handle* p = new (std::nothrow) Engine_Handle();
  if (!p) return ENGINE_OUT_OF_MEMORY;
  *out_handle = p;
  return ENGINE_SUCCESS;
}

ENGINE_API Engine_Result ENGINE_CALL Engine_Destroy(Engine_Handle** handle) {
  if (!handle) return ENGINE_INVALID_ARGUMENT;
  delete *handle;  // delete NULL is safe; allocation and free both happen here.
  *handle = 0;
  return ENGINE_SUCCESS;
}

ENGINE_API Engine_Result ENGINE_CALL Engine_Tick(Engine_Handle* handle,
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
