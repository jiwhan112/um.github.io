
<callout icon="📦" color="blue_bg">
	**수동 검수 12 · 01 실습 소스 보관 복구**
	<mention-date start="2026-10-16"/>. 기존 `dx9_device_lab_v1.zip`의 실제 파일을 읽어 복구했습니다. 아래 C++ 3개 파일과 CMake 설정은 원형을 보존한 전체 소스입니다. 교재는 <mention-page url="https://app.notion.com/p/3f8a99703f408135a849f7c66e4d54ad">01 · DX9 장치 생성과 COM 참조 수명</mention-page>에 있습니다. 새 학습 단원을 중복 작성한 것이 아니라 이전에 누락된 Windows 소스를 포함하는 재현 자료입니다.
</callout>
## 실행 범위와 사용법
같은 폴더에 `com_owner.h`, `com_owner_tests.cpp`, `d3d9_window.cpp`, `CMakeLists.txt`를 각각 저장합니다. 원형 CMake의 표준은 C++11이며 이번 보관 복구에서 회사나 기존 프로젝트의 표준을 변경하지 않습니다. Linux에서는 fake COM 테스트만 빌드합니다. Windows에서는 별도 창 예제가 추가되지만 **이번 환경에서 Windows/MSVC·실제 COM·D3D9·GPU 실행은 검증하지 않았습니다.** 창 예제는 Clear/Present만 수행하며 삼각형·텍스처 사각형 프로젝트가 아닙니다.
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 2
ctest --test-dir build --output-on-failure -V
# 단일 CPU 테스트 직접 빌드
 g++ -std=c++11 -Wall -Wextra -Werror com_owner_tests.cpp -o com_owner_tests
 ./com_owner_tests
```
테스트는 assert를 사용하므로 `NDEBUG`가 정의되는 구성에서는 검사가 생략될 수 있습니다. 이번 재실행은 Debug로 수행했습니다. 성공 시 아래 출력과 종료 코드 0을 확인합니다. AddRef/Release 호출 횟수는 이 fake 객체 시나리오의 결과이지 일반 COM API의 모든 내부 참조를 측정한 값이 아닙니다.
## com_owner.h — 전체
```c++
#pragma once

#include <cassert>
#include <utility>

// Owns exactly one COM reference. The raw pointer passed to reset is adopted:
// it must already represent a reference owned by the caller.
template <class T>
class ComOwner {
public:
    ComOwner() noexcept : ptr_(nullptr) {}
    explicit ComOwner(T* ptr) noexcept : ptr_(ptr) {}

    ~ComOwner() { reset(); }

    ComOwner(const ComOwner&) = delete;
    ComOwner& operator=(const ComOwner&) = delete;

    ComOwner(ComOwner&& other) noexcept : ptr_(other.detach()) {}
    ComOwner& operator=(ComOwner&& other) noexcept {
        if (this != &other) reset(other.detach());
        return *this;
    }

    T* get() const noexcept { return ptr_; }
    T* operator->() const noexcept { assert(ptr_); return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    // For COM out-parameters. Releases any previous reference first.
    T** put() noexcept { reset(); return &ptr_; }

    T* detach() noexcept {
        T* old = ptr_;
        ptr_ = nullptr;
        return old;
    }

    void reset(T* replacement = nullptr) noexcept {
        if (ptr_ == replacement) return; // Never release-and-retain same pointer.
        T* old = ptr_;
        ptr_ = replacement;
        if (old) old->Release();
    }

private:
    T* ptr_;
};
```
## com_owner_tests.cpp — 전체
```c++
#include "com_owner.h"
#include <cassert>
#include <iostream>
#include <type_traits>
#include <utility>

struct Counts {
    int created = 0;
    int destroyed = 0;
    int addrefs = 0;
    int releases = 0;
};

struct FakeCom {
    explicit FakeCom(Counts& c) : stats(c), refs(1) { ++stats.created; }
    ~FakeCom() { assert(refs == 0); ++stats.destroyed; }
    unsigned long AddRef() { ++stats.addrefs; return ++refs; }
    unsigned long Release() {
        ++stats.releases;
        assert(refs > 0);
        const auto remaining = --refs;
        if (remaining == 0) delete this;
        return remaining;
    }
    Counts& stats;
    unsigned long refs;
};

int main() {
    static_assert(!std::is_copy_constructible<ComOwner<FakeCom>>::value, "COM owner must not copy");
    static_assert(std::is_move_constructible<ComOwner<FakeCom>>::value, "COM owner must move");
    Counts count;
    {
        ComOwner<FakeCom> a(new FakeCom(count));
        assert(a && a->refs == 1);
        ComOwner<FakeCom> b(std::move(a));
        assert(!a && b && b->refs == 1);
        b.reset();
        assert(!b && count.destroyed == 1);
    }
    {
        ComOwner<FakeCom> first;
        *first.put() = new FakeCom(count);  // Emulates a COM API out parameter.
        ComOwner<FakeCom> other(new FakeCom(count));
        first = std::move(other);
        assert(!other && first && count.destroyed == 2);
    }
    {
        ComOwner<FakeCom> first(new FakeCom(count));
        FakeCom* shared = first.get();
        shared->AddRef(); // A second owner requires a new reference.
        ComOwner<FakeCom> second(shared);
        first.reset();
        assert(second && second->refs == 1);
        second.reset();
        assert(count.destroyed == 4);
    }
    assert(count.created == 4);
    assert(count.destroyed == 4);
    assert(count.addrefs == 1);
    assert(count.releases == 5);
    std::cout << "COM lifetime tests passed: objects=" << count.created
              << ", AddRef=" << count.addrefs << ", Release=" << count.releases << '\n';
}
```
## d3d9_window.cpp — 전체, Windows 실행 미검증
아래는 회수한 원형입니다. CPU 테스트 통과를 이 파일의 컴파일·리사이즈·Lost Device 성공으로 해석하지 않습니다. Windows SDK, 아키텍처와 Unicode 설정, 실제 창 종료·최소화·복원 및 Reset 경로를 사용자 환경에서 검증해야 합니다.
```c++
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "com_owner.h"

#pragma comment(lib, "d3d9.lib")

struct WindowState { bool resized = false; };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_NCCREATE) {
        auto* params = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(params->lpCreateParams));
        return TRUE;
    }
    WindowState* state = reinterpret_cast<WindowState*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_SIZE:
            if (state && wp != SIZE_MINIMIZED) state->resized = true;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

D3DPRESENT_PARAMETERS MakePresentParameters(HWND hwnd) {
    D3DPRESENT_PARAMETERS pp = {};
    pp.Windowed = TRUE;
    pp.hDeviceWindow = hwnd;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferFormat = D3DFMT_UNKNOWN;
    pp.BackBufferWidth = 0;  // In windowed mode, derive from current client area.
    pp.BackBufferHeight = 0;
    pp.EnableAutoDepthStencil = FALSE; // Clear-only example uses no depth buffer.
    pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    return pp;
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    const wchar_t kClassName[] = L"DX9DeviceLifeExample";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (!RegisterClassW(&wc)) return 1;

    WindowState state;
    HWND hwnd = CreateWindowExW(0, kClassName, L"D3D9 Device and COM Lifetime",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, instance, &state);
    if (!hwnd) { UnregisterClassW(kClassName, instance); return 2; }
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    int code = 0;
    {
        ComOwner<IDirect3D9> d3d(Direct3DCreate9(D3D_SDK_VERSION));
        if (!d3d) {
            code = 3;
        } else {
            ComOwner<IDirect3DDevice9> device;
            D3DPRESENT_PARAMETERS pp = MakePresentParameters(hwnd);
            HRESULT hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, device.put());
            if (FAILED(hr)) {
                device.reset();
                pp = MakePresentParameters(hwnd);
                hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                    hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, device.put());
            }
            if (FAILED(hr) || !device) {
                code = 4;
            } else {
                state.resized = false;
                bool resetNeeded = false;
                MSG msg = {};
                bool running = true;
                while (running) {
                    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                        if (msg.message == WM_QUIT) { running = false; break; }
                        TranslateMessage(&msg);
                        DispatchMessageW(&msg);
                    }
                    if (!running) break;
                    RECT client = {};
                    if (!GetClientRect(hwnd, &client) ||
                        client.right <= client.left || client.bottom <= client.top) {
                        Sleep(30);
                        continue;
                    }
                    if (state.resized) { state.resized = false; resetNeeded = true; }
                    if (resetNeeded) {
                        hr = device->TestCooperativeLevel();
                        if (hr == D3DERR_DEVICELOST) { Sleep(30); continue; }
                        if (hr != D3D_OK && hr != D3DERR_DEVICENOTRESET) {
                            code = 5; break;
                        }
                        // Later chapters must release D3DPOOL_DEFAULT resources here.
                        pp = MakePresentParameters(hwnd);
                        hr = device->Reset(&pp);
                        if (FAILED(hr)) {
                            if (hr != D3DERR_DEVICELOST) { code = 6; break; }
                            Sleep(30);
                            continue;
                        }
                        resetNeeded = false;
                        // Later chapters must reapply render states/recreate resources here.
                    }
                    hr = device->Clear(0, nullptr, D3DCLEAR_TARGET,
                        D3DCOLOR_XRGB(25, 45, 75), 1.0f, 0);
                    if (hr == D3DERR_DEVICELOST) { resetNeeded = true; Sleep(30); continue; }
                    if (FAILED(hr)) { code = 7; break; }
                    hr = device->Present(nullptr, nullptr, nullptr, nullptr);
                    if (hr == D3DERR_DEVICELOST) resetNeeded = true;
                    else if (FAILED(hr)) { code = 8; break; }
                }
            }
        }
    } // Device releases before IDirect3D9; all COM objects before window destruction.
    DestroyWindow(hwnd);
    UnregisterClassW(kClassName, instance);
    return code;
}
```
## CMakeLists.txt — 원형 전체
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx9_device_lifetime LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(com_owner_tests com_owner_tests.cpp)
enable_testing()
add_test(NAME com_owner_tests COMMAND com_owner_tests)
if(WIN32)
    add_executable(d3d9_window WIN32 d3d9_window.cpp)
    target_link_libraries(d3d9_window PRIVATE d3d9)
endif()
```
## 이번 재검증 기록
<mention-date start="2026-10-16" startTime="14:28" timeZone="Asia/Seoul"/> Linux x86-64에서 원형 소스를 GCC 14.2 Debug와 Clang 17 Debug+ASan/UBSan으로 각각 다시 빌드했습니다. 두 구성 모두 configure/build/CTest 종료 코드 0이며 등록 테스트는 각각 1/1입니다. 같은 테스트를 반복했으며 실제 GPU 검사는 0개입니다.
```plain text
GCC Debug / CTest
1: COM lifetime tests passed: objects=4, AddRef=1, Release=5
100% tests passed, 0 tests failed out of 1

Clang Debug + -fsanitize=address,undefined -fno-omit-frame-pointer / CTest
1: COM lifetime tests passed: objects=4, AddRef=1, Release=5
100% tests passed, 0 tests failed out of 1
Windows/MSVC/Win32/D3D9/GPU: NOT RUN
```
Clang 재현 시 CMake 구성에 CXX compiler를 clang++로 지정하고 CXX·EXE 링크 옵션에 sanitizer 옵션을 함께 전달합니다. 단순히 실행 로그만 복구한 것이 아니라 위 소스 전체가 이 페이지에 저장되어 있습니다. 이번 로그의 전체 명령·stdout/stderr는 수동 검수 12 자료 묶음에도 보존합니다.
## 원본 식별과 보관 범위
원형 ZIP SHA-256: `b42f1f88bde58d88d4bb17b6dffe5533a7df13ecf7e904b21052f9f8e452aa49`.
<table header-row="true">
<tr>
<td>파일</td>
<td>원형 바이트 수</td>
<td>원형 SHA-256</td>
</tr>
<tr>
<td>com_owner.h</td>
<td>1314</td>
<td>65ebe40fd25f688c6129e7eebc062f583408f8b93292be7873da5edfd503a7c0</td>
</tr>
<tr>
<td>com_owner_tests.cpp</td>
<td>2044</td>
<td>073fc40f1e6f299e898c7039ea547ac16df5f06894b8008e16b22cb85e2048cc</td>
</tr>
<tr>
<td>d3d9_window.cpp</td>
<td>5297</td>
<td>13782456db74562471a54c4427c43a1b00b08a0ad1d49a18b9009bb51f2ab94c</td>
</tr>
<tr>
<td>CMakeLists.txt</td>
<td>391</td>
<td>a737bb3933329cf5a332596ae5278f64fdc0a010605e99c89b60fe090caf8ba5</td>
</tr>
</table>
표의 해시는 회수한 ZIP 안 파일의 바이트 기준입니다. Notion 코드 블록의 마지막 빈 줄 등 정규화까지 바이트 단위 일치가 검증됐다고 주장하지 않습니다. 이전 첨부는 삭제하지 않았습니다. 기술 해설·Microsoft API 출처는 상위 01 교재를 사용합니다. 원형 코드의 전체 입력 계약·Windows 실행·장치 누수·성능 검증 완료와 소스 보관 복구 완료를 구분합니다.
