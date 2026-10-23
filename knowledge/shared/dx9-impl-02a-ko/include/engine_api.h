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

typedef struct Engine_Handle Engine_Handle; // [한국어 해설] 불투명 핸들로 구현 크기를 숨긴다. 호출자는 delete 대신 짝인 Engine_Destroy를 사용한다.

typedef enum Engine_Result { // [한국어 해설] C ABI 경계는 결과 코드로 실패를 전달한다. C++ 예외나 STL 객체를 경계 밖으로 넘기지 않는다.
  ENGINE_SUCCESS = 0,
  ENGINE_INVALID_ARGUMENT = 1,
  ENGINE_OUT_OF_MEMORY = 2
} Engine_Result;

/* *out_handle must be NULL on entry; Engine_Destroy is its matching release. */
ENGINE_API Engine_Result ENGINE_CALL Engine_Create(Engine_Handle** out_handle); // [한국어 해설] out 포인터가 유효하고 그 대상이 비어 있어야 한다. 기존 소유자를 덮어쓰지 않는다.
ENGINE_API Engine_Result ENGINE_CALL Engine_Destroy(Engine_Handle** handle);
/* Accept finite seconds within [0, 1]. A rejected tick leaves state unchanged. */
ENGINE_API Engine_Result ENGINE_CALL Engine_Tick(Engine_Handle* handle, double delta_seconds); // [한국어 해설] 유한한 [0,1]초만 허용한다. 거부된 tick은 누적 시간을 변경하지 않는다.
ENGINE_API Engine_Result ENGINE_CALL Engine_GetElapsed(const Engine_Handle* handle,
                                                       double* out_seconds);
ENGINE_API Engine_Result ENGINE_CALL Engine_Reset(Engine_Handle* handle);

#ifdef __cplusplus
}
#endif
#endif
