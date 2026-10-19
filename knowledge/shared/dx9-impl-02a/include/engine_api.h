#ifndef DX9_LAB_ENGINE_API_H_
#define DX9_LAB_ENGINE_API_H_

/* Deliberately C-compatible public ABI; implementation stays in the library. */
#if defined(_WIN32)
#  if defined(ENGINE_BUILD)
#    define ENGINE_API __declspec(dllexport)
#  else
#    define ENGINE_API __declspec(dllimport)
#  endif
#  define ENGINE_CALL __cdecl
#else
#  if defined(__GNUC__) || defined(__clang__)
#    define ENGINE_API __attribute__((visibility("default")))
#  else
#    define ENGINE_API
#  endif
#  define ENGINE_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Engine_Handle Engine_Handle;

typedef enum Engine_Result {
  ENGINE_SUCCESS = 0,
  ENGINE_INVALID_ARGUMENT = 1,
  ENGINE_OUT_OF_MEMORY = 2
} Engine_Result;

/* *out_handle must be NULL on entry; Engine_Destroy is its matching release. */
ENGINE_API Engine_Result ENGINE_CALL Engine_Create(Engine_Handle** out_handle);
ENGINE_API Engine_Result ENGINE_CALL Engine_Destroy(Engine_Handle** handle);
/* Accept finite seconds within [0, 1]. A rejected tick leaves state unchanged. */
ENGINE_API Engine_Result ENGINE_CALL Engine_Tick(Engine_Handle* handle, double delta_seconds);
ENGINE_API Engine_Result ENGINE_CALL Engine_GetElapsed(const Engine_Handle* handle,
                                                       double* out_seconds);
ENGINE_API Engine_Result ENGINE_CALL Engine_Reset(Engine_Handle* handle);

#ifdef __cplusplus
}
#endif
#endif
