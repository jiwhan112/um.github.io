
<callout icon="📦" color="blue_bg">
	**수동 검수 12 · 02 실습의 전체 소스 보관 복구**
	<mention-date start="2026-10-16"/>. 기존 `dx9_dll_boundary_lab_v1.zip`의 공개 헤더·라이브러리 구현·클라이언트·테스트·CMake를 아래에 원형 그대로 보관합니다. 교재 <mention-page url="https://app.notion.com/p/3f8a99703f408172837fe0d04c131e68">02 · 엔진 DLL·클라이언트 ABI와 객체 소유권</mention-page>를 중복 작성하지 않습니다. Linux 공유 라이브러리 실행과 Windows DLL 실행은 별도입니다.
</callout>
## 1. 파일 배치와 재현
`include/engine_api.h`, `src/engine_api.cpp`, `src/client.cpp`, `tests/engine_contract_tests.cpp`, 루트의 `CMakeLists.txt`로 저장합니다. 외부 입력 파일·그래픽 에셋은 필요 없습니다. 원형 학습 표준은 C++11, CMake 3.16 이상입니다.
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++
cmake --build build -j 2
ctest --test-dir build --output-on-failure -V
./build/bin/client
```
이번 실행에서는 Linux x86-64의 GCC 14.2와 Clang 17 ASan/UBSan으로 빌드했습니다. Windows의 import library·DLL 로더·MSVC, 서로 다른 도구 버전 사이의 ABI 호환성·hot reload·동시성·실제 D3D9/GPU는 검증하지 않았습니다.
## 2. include/engine_api.h — 전체
```c++
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
```
## 3. src/engine_api.cpp — 전체
```c++
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
```
## 4. src/client.cpp — 전체
```c++
#include "engine_api.h"
#include <iostream>

// Non-copyable owning wrapper. A caller never calls delete on Engine_Handle.
class EngineOwner {
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

int main() {
  EngineOwner engine;
  if (Engine_Create(engine.Address()) != ENGINE_SUCCESS) return 1;
  if (Engine_Tick(engine.Get(), 0.016) != ENGINE_SUCCESS) return 2;
  if (Engine_Tick(engine.Get(), 0.034) != ENGINE_SUCCESS) return 3;
  double seconds = -1;
  if (Engine_GetElapsed(engine.Get(), &seconds) != ENGINE_SUCCESS) return 4;
  std::cout << "Elapsed: " << seconds << " seconds\n";
  return (seconds > 0.049999 && seconds < 0.050001) ? 0 : 5;
}
```
## 5. tests/engine_contract_tests.cpp — 전체
```c++
#include "engine_api.h"
#include <cmath>
#include <iostream>
#include <limits>

static int checked = 0;
static int failed = 0;
static void Check(bool ok, const char* description) {
  ++checked;
  if (!ok) { ++failed; std::cerr << "FAIL " << description << "\n"; }
}
int main() {
  Check(Engine_Create(0) == ENGINE_INVALID_ARGUMENT, "create null out");
  Engine_Handle* handle = 0;
  Check(Engine_Create(&handle) == ENGINE_SUCCESS && handle != 0, "create success");
  Check(Engine_Create(&handle) == ENGINE_INVALID_ARGUMENT, "reject overwriting owner");
  Check(Engine_Tick(0, 0.1) == ENGINE_INVALID_ARGUMENT, "tick null handle");
  Check(Engine_Tick(handle, -0.01) == ENGINE_INVALID_ARGUMENT, "reject negative delta");
  Check(Engine_Tick(handle, 1.01) == ENGINE_INVALID_ARGUMENT, "reject oversized delta");
  Check(Engine_Tick(handle, std::numeric_limits<double>::infinity()) == ENGINE_INVALID_ARGUMENT, "reject infinity");
  Check(Engine_Tick(handle, std::numeric_limits<double>::quiet_NaN()) == ENGINE_INVALID_ARGUMENT, "reject NaN");
  Check(Engine_Tick(handle, 0.0) == ENGINE_SUCCESS, "zero delta accepted");
  Check(Engine_Tick(handle, 0.125) == ENGINE_SUCCESS, "tick first");
  Check(Engine_Tick(handle, 0.375) == ENGINE_SUCCESS, "tick second");
  double time = -1;
  Check(Engine_GetElapsed(handle, 0) == ENGINE_INVALID_ARGUMENT, "get null out");
  Check(Engine_GetElapsed(0, &time) == ENGINE_INVALID_ARGUMENT, "get null handle");
  Check(Engine_GetElapsed(handle, &time) == ENGINE_SUCCESS && time == 0.5, "time after rejected ticks");
  Check(Engine_Reset(0) == ENGINE_INVALID_ARGUMENT, "reset null");
  Check(Engine_Reset(handle) == ENGINE_SUCCESS, "reset success");
  Check(Engine_GetElapsed(handle, &time) == ENGINE_SUCCESS && time == 0.0, "reset has zero time");
  Check(Engine_Destroy(0) == ENGINE_INVALID_ARGUMENT, "destroy null owner pointer");
  Check(Engine_Destroy(&handle) == ENGINE_SUCCESS && handle == 0, "destroy clears handle");
  Check(Engine_Destroy(&handle) == ENGINE_SUCCESS && handle == 0, "destroy empty is idempotent");
  Check(Engine_Tick(handle, 0.1) == ENGINE_INVALID_ARGUMENT, "use after release detects null");
  std::cout << "checks=" << checked << " failures=" << failed << "\n";
  return failed ? 1 : 0;
}
```
## 6. CMakeLists.txt — 전체
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx9_dll_boundary_lab LANGUAGES CXX)

add_library(engine SHARED src/engine_api.cpp)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(engine PRIVATE ENGINE_BUILD)
set_target_properties(engine PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_executable(client src/client.cpp)
add_executable(engine_contract_tests tests/engine_contract_tests.cpp)
foreach(target IN ITEMS client engine_contract_tests)
  target_link_libraries(${target} PRIVATE engine)
  set_target_properties(${target} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endforeach()

# On Windows, both engine.dll and executables are placed in build/bin/<config>.
# On Linux, CMake sets a build-tree RUNPATH to the engine library location.
enable_testing()
add_test(NAME client_smoke COMMAND client)
add_test(NAME exported_api_contract COMMAND engine_contract_tests)
```
## 7. 실제 재검증 결과와 한계
<mention-date start="2026-10-16" startTime="14:28" timeZone="Asia/Seoul"/> 두 구성의 configure/build/CTest는 모두 종료 코드 0입니다. CTest 2개는 클라이언트 실행 1개와 계약 테스트 실행 1개이며, 후자 안에 21개 조건 검사가 있습니다. 반복 실행을 독립적인 42개 계약으로 합산하지 않습니다.
```plain text
GCC 14.2 / Debug / Linux x86-64
client_smoke: Elapsed: 0.05 seconds
exported_api_contract: checks=21 failures=0
100% tests passed, 0 tests failed out of 2

Clang 17 / Debug / ASan+UBSan
client_smoke: Elapsed: 0.05 seconds
exported_api_contract: checks=21 failures=0
100% tests passed, 0 tests failed out of 2
Windows/MSVC/DLL loader/Direct3D9/GPU: NOT RUN
```
Sanitizer 재현은 clang++와 `-fsanitize=address,undefined -fno-omit-frame-pointer`를 CXX 옵션에, `-fsanitize=address,undefined`를 실행파일과 공유 라이브러리 링크 옵션에 모두 전달합니다. Windows 수동 명령은 CMake 구성 후 `cmake --build build --config Debug`, `ctest --test-dir build -C Debug --output-on-failure`입니다. 현재 성공 기록은 Linux에서 실제로 수행한 것만 의미합니다.
이 코드에서 delta의 0\~1초 제한은 예제의 규칙입니다. 핸들은 유효한 생성 결과여야 하며, 임의 주소·해제 후 복사본을 넘기는 경우까지 검증하는 세대 번호 핸들 체계가 아닙니다. 두 번째 Destroy의 성공은 같은 소유 변수에 null이 저장된 경우입니다. 메모리 부족 분기는 구현되어 있지만 이번 테스트에서 실제 할당 실패를 주입하지 않았습니다.
## 8. 원형 식별과 변경 기록
원형 ZIP SHA-256: `cb4315e5946ef28370d409df4fd76f2a037c9a8706049113b517be212d3dad33`.
<table header-row="true">
<tr>
<td>원형 파일</td>
<td>바이트 수</td>
<td>SHA-256</td>
</tr>
<tr>
<td>include/engine_api.h</td>
<td>1410</td>
<td>d1b427994d5a42984fd5256565752e5bee79ce92b7f5e7daf306d0e4a8992eed</td>
</tr>
<tr>
<td>src/engine_api.cpp</td>
<td>1540</td>
<td>37b17acd9ca5bf415d74e1604a5bf0e4703ae1fc1d12a73885fb72c8489e6994</td>
</tr>
<tr>
<td>src/client.cpp</td>
<td>924</td>
<td>1676fda792c7c9077999fcb457c7a659a6ec8005f34588e9feb8cd55350c68d8</td>
</tr>
<tr>
<td>tests/engine_contract_tests.cpp</td>
<td>2245</td>
<td>1d35c967026d5d9438e06c4ff024e451107f34bcc1ae9c6025a7503588889d62</td>
</tr>
<tr>
<td>CMakeLists.txt</td>
<td>1195</td>
<td>4707d77adc7941fc4ad5451f880632215b4f65ccdeb61c0ce9a7bdd393729107</td>
</tr>
</table>
이 해시는 회수한 원형 파일 기준이며 Notion의 줄끝 정규화까지 바이트 일치를 확인한 수치는 아닙니다. 실제 전체 소스·재현 안내·새 검증 결과는 이 페이지를 기준으로 다시 읽을 수 있습니다. 기존 02 교재는 보존했으며, 기술 설명과 공식 출처는 상위 교재로 연결합니다.
