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
