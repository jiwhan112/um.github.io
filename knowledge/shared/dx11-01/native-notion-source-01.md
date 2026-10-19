<callout icon="📦" color="gray_bg">
	**DX11 01 · 전체 소스 지속 보관**
	<mention-date start="2026-10-16"/>. 이 페이지는 실제 검증에 사용한 CPU 계약 모델과 Windows D3D11 초기화 예제·CMake·README 전체를 보관합니다. 첨부 재다운로드 도구가 빈 결과를 반환하여 네이티브 본문을 재시작 기준으로 사용합니다.
</callout>
# DX11 init01 — full sources
## contract_model.cpp
```c++
#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

enum class FeatureLevel { FL11_1, FL11_0, FL10_1, FL10_0, FL9_3 };
enum class CreateResult { Ok, EInvalidArg, DebugLayerMissing, NoSupportedLevel };

struct RuntimeCaps {
    bool runtime11_1 = true;
    bool debugLayer = true;
    FeatureLevel maxLevel = FeatureLevel::FL11_1;
};

static int Rank(FeatureLevel f) {
    switch (f) {
        case FeatureLevel::FL11_1: return 5;
        case FeatureLevel::FL11_0: return 4;
        case FeatureLevel::FL10_1: return 3;
        case FeatureLevel::FL10_0: return 2;
        case FeatureLevel::FL9_3: return 1;
    }
    return 0;
}

struct CreateAttempt {
    CreateResult result;
    FeatureLevel granted;
    CreateAttempt(CreateResult r = CreateResult::NoSupportedLevel,
                  FeatureLevel g = FeatureLevel::FL9_3) : result(r), granted(g) {}
};

CreateAttempt TryCreate(const RuntimeCaps& caps,
                        const std::vector<FeatureLevel>& requested,
                        bool requestDebug) {
    if (requestDebug && !caps.debugLayer)
        return {CreateResult::DebugLayerMissing, FeatureLevel::FL9_3};

    for (FeatureLevel f : requested) {
        if (f == FeatureLevel::FL11_1 && !caps.runtime11_1)
            return {CreateResult::EInvalidArg, FeatureLevel::FL9_3};
    }

    for (FeatureLevel f : requested) {
        if (Rank(f) <= Rank(caps.maxLevel))
            return {CreateResult::Ok, f};
    }
    return {CreateResult::NoSupportedLevel, FeatureLevel::FL9_3};
}

struct NegotiatedDevice {
    CreateResult result;
    FeatureLevel granted;
    int attempts;
    NegotiatedDevice(CreateResult r = CreateResult::NoSupportedLevel,
                     FeatureLevel g = FeatureLevel::FL9_3, int a = 0)
        : result(r), granted(g), attempts(a) {}
};

NegotiatedDevice CreateWithRuntimeFallback(const RuntimeCaps& caps, bool requestDebug) {
    const std::vector<FeatureLevel> with11_1 = {
        FeatureLevel::FL11_1, FeatureLevel::FL11_0, FeatureLevel::FL10_1,
        FeatureLevel::FL10_0, FeatureLevel::FL9_3};
    const std::vector<FeatureLevel> without11_1(with11_1.begin() + 1, with11_1.end());

    CreateAttempt first = TryCreate(caps, with11_1, requestDebug);
    if (first.result != CreateResult::EInvalidArg)
        return {first.result, first.granted, 1};

    CreateAttempt second = TryCreate(caps, without11_1, requestDebug);
    return {second.result, second.granted, 2};
}

struct Resource { int id; };
struct View { int resourceId; };

class DeviceModel {
public:
    Resource CreateResource() { return {++next_}; }
    View CreateRenderTargetView(const Resource& r) {
        if (r.id <= 0) throw std::invalid_argument("invalid resource");
        return {r.id};
    }
private:
    int next_ = 0;
};

class ContextModel {
public:
    void OMSetRenderTarget(const View* view) {
        bound_ = view ? view->resourceId : 0;
    }
    bool Draw() const { return bound_ != 0; }
    void ClearState() { bound_ = 0; }
    int BoundResource() const { return bound_; }
private:
    int bound_ = 0;
};

class SwapChainModel {
public:
    explicit SwapChainModel(int resourceId) : backBufferResource_(resourceId) {}

    View AcquireRenderTargetView(DeviceModel& device) {
        if (viewOutstanding_) throw std::logic_error("view already outstanding");
        viewOutstanding_ = true;
        ++indirectRefs_;
        return device.CreateRenderTargetView({backBufferResource_});
    }

    void Bind(ContextModel& context, const View& view) {
        if (!viewOutstanding_ || view.resourceId != backBufferResource_)
            throw std::logic_error("view does not reference back buffer");
        context.OMSetRenderTarget(&view);
        bound_ = true;
    }

    void Unbind(ContextModel& context) {
        context.OMSetRenderTarget(nullptr);
        bound_ = false;
    }

    void ReleaseView() {
        if (!viewOutstanding_) throw std::logic_error("no view to release");
        viewOutstanding_ = false;
        --indirectRefs_;
    }

    bool ResizeBuffers(unsigned width, unsigned height) {
        if (width == 0 || height == 0) return false;
        if (bound_ || indirectRefs_ != 0) return false;
        ++generation_;
        backBufferResource_ += 100;
        width_ = width;
        height_ = height;
        return true;
    }

    unsigned Generation() const { return generation_; }
    int BackBufferResource() const { return backBufferResource_; }
    unsigned Width() const { return width_; }
    unsigned Height() const { return height_; }
private:
    int backBufferResource_;
    int indirectRefs_ = 0;
    bool viewOutstanding_ = false;
    bool bound_ = false;
    unsigned generation_ = 1;
    unsigned width_ = 800;
    unsigned height_ = 600;
};

int main() {
    int checks = 0;
    auto check = [&](bool ok, const char* msg) {
        ++checks;
        if (!ok) throw std::runtime_error(msg);
    };

    {
        RuntimeCaps caps;
        auto r = CreateWithRuntimeFallback(caps, true);
        check(r.result == CreateResult::Ok, "11.1 runtime creates device");
        check(r.granted == FeatureLevel::FL11_1, "grants 11.1");
        check(r.attempts == 1, "one attempt on 11.1 runtime");
    }
    {
        RuntimeCaps caps;
        caps.runtime11_1 = false;
        caps.maxLevel = FeatureLevel::FL11_0;
        auto r = CreateWithRuntimeFallback(caps, false);
        check(r.result == CreateResult::Ok, "11.0 runtime retry succeeds");
        check(r.granted == FeatureLevel::FL11_0, "fallback grants 11.0");
        check(r.attempts == 2, "E_INVALIDARG causes one retry without 11.1");
    }
    {
        RuntimeCaps caps;
        caps.debugLayer = false;
        auto r = CreateWithRuntimeFallback(caps, true);
        check(r.result == CreateResult::DebugLayerMissing, "missing debug layer stays visible");
        check(r.attempts == 1, "debug failure is not feature-level retry");
    }
    {
        RuntimeCaps caps;
        caps.maxLevel = FeatureLevel::FL10_0;
        auto r = CreateWithRuntimeFallback(caps, false);
        check(r.result == CreateResult::Ok, "lower feature level can be negotiated");
        check(r.granted == FeatureLevel::FL10_0, "first supported requested level wins");
    }

    DeviceModel device;
    ContextModel context;
    Resource offscreen = device.CreateResource();
    View offscreenView = device.CreateRenderTargetView(offscreen);
    check(!context.Draw(), "draw without render target rejected by model");
    context.OMSetRenderTarget(&offscreenView);
    check(context.Draw(), "context binds view then draw can proceed");
    context.ClearState();
    check(context.BoundResource() == 0, "ClearState unbinds model state");

    SwapChainModel swap(10);
    View backBufferView = swap.AcquireRenderTargetView(device);
    check(backBufferView.resourceId == 10, "view references current back buffer");
    swap.Bind(context, backBufferView);
    check(context.BoundResource() == 10, "back buffer RTV bound through context");
    check(!swap.ResizeBuffers(1280, 720), "resize rejected while back buffer bound/referenced");
    swap.Unbind(context);
    check(!swap.ResizeBuffers(1280, 720), "resize still rejected while view reference exists");
    swap.ReleaseView();
    check(swap.ResizeBuffers(1280, 720), "resize succeeds after unbind and release");
    check(swap.Generation() == 2, "resize changes back buffer generation");
    check(swap.Width() == 1280 && swap.Height() == 720, "new size recorded");
    check(swap.BackBufferResource() != 10, "old back buffer identity not reused by model");
    check(!swap.ResizeBuffers(0, 720), "zero-width resize rejected by teaching policy");

    View newView = swap.AcquireRenderTargetView(device);
    check(newView.resourceId == swap.BackBufferResource(), "view recreated from resized buffer");
    swap.Bind(context, newView);
    check(context.Draw(), "draw resumes after RTV recreation");
    swap.Unbind(context);
    swap.ReleaseView();

    std::cout << "PASS " << checks << " DX11 initialization contract checks (CPU model; not D3D11 GPU)\n";
    return 0;
}
```
## d3d11_init_window.cpp
```c++
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

struct HrError : std::runtime_error {
    HRESULT hr;
    HrError(HRESULT value, const char* where) : std::runtime_error(where), hr(value) {}
};
static void CheckHR(HRESULT hr, const char* where) {
    if (FAILED(hr)) throw HrError(hr, where);
}
#define DX11_HR(expr) CheckHR((expr), #expr)

struct WindowState {
    bool quit = false;
    bool resizePending = false;
    UINT width = 0;
    UINT height = 0;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    WindowState* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        const auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        state = static_cast<WindowState*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    }
    switch (msg) {
        case WM_SIZE:
            if (state && wp != SIZE_MINIMIZED) {
                state->width = LOWORD(lp);
                state->height = HIWORD(lp);
                state->resizePending = state->width != 0 && state->height != 0;
            }
            return 0;
        case WM_KEYDOWN:
            if (state && wp == VK_ESCAPE) state->quit = true;
            return 0;
        case WM_CLOSE:
            if (state) state->quit = true;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

class Dx11App {
public:
    void Initialize(HWND hwnd, UINT width, UINT height) {
        hwnd_ = hwnd;
        CreateDeviceAndSwapChain(width, height);
        RecreateBackBufferViews(width, height);
    }

    void Resize(UINT width, UINT height) {
        if (width == 0 || height == 0) return;
        context_->OMSetRenderTargets(0, nullptr, nullptr);
        rtv_.Reset();
        dsv_.Reset();
        depth_.Reset();

        DX11_HR(swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
        RecreateBackBufferViews(width, height);
    }

    void Render() {
        const float clear[4] = {0.08f, 0.12f, 0.18f, 1.0f};
        context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), dsv_.Get());
        context_->ClearRenderTargetView(rtv_.Get(), clear);
        context_->ClearDepthStencilView(dsv_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        DX11_HR(swapChain_->Present(1, 0));
    }

    ~Dx11App() {
        if (context_) {
            context_->ClearState();
            context_->Flush();
        }
    }

private:
    void CreateDeviceAndSwapChain(UINT width, UINT height) {
        DXGI_SWAP_CHAIN_DESC desc{};
        desc.BufferDesc.Width = width;
        desc.BufferDesc.Height = height;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.OutputWindow = hwnd_;
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // legacy helper path for a minimal sample

        UINT flags = 0;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        const D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3
        };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
            &desc, swapChain_.GetAddressOf(), device_.GetAddressOf(),
            &featureLevel_, context_.GetAddressOf());

        // A Direct3D 11.0 runtime rejects an explicit 11_1 entry with E_INVALIDARG.
        if (hr == E_INVALIDARG) {
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                levels + 1, ARRAYSIZE(levels) - 1, D3D11_SDK_VERSION,
                &desc, swapChain_.GetAddressOf(), device_.GetAddressOf(),
                &featureLevel_, context_.GetAddressOf());
        }
        // In a debug build, do not silently hide a missing/mismatched debug layer.
        DX11_HR(hr);
    }

    void RecreateBackBufferViews(UINT width, UINT height) {
        ComPtr<ID3D11Texture2D> backBuffer;
        DX11_HR(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
        DX11_HR(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv_.GetAddressOf()));

        D3D11_TEXTURE2D_DESC depthDesc{};
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        DX11_HR(device_->CreateTexture2D(&depthDesc, nullptr, depth_.GetAddressOf()));
        DX11_HR(device_->CreateDepthStencilView(depth_.Get(), nullptr, dsv_.GetAddressOf()));

        context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), dsv_.Get());
        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context_->RSSetViewports(1, &viewport);
    }

    HWND hwnd_ = nullptr;
    D3D_FEATURE_LEVEL featureLevel_ = D3D_FEATURE_LEVEL_9_3;
    ComPtr<IDXGISwapChain> swapChain_;
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<ID3D11RenderTargetView> rtv_;
    ComPtr<ID3D11Texture2D> depth_;
    ComPtr<ID3D11DepthStencilView> dsv_;
};

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    const wchar_t kClassName[] = L"Dx11Init01Window";
    WindowState state;
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kClassName;
    if (!RegisterClassW(&wc)) return 2;

    HWND hwnd = CreateWindowExW(0, kClassName, L"DX11 init: Device / Context / SwapChain",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 640,
        nullptr, nullptr, instance, &state);
    if (!hwnd) { UnregisterClassW(kClassName, instance); return 3; }

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);
    RECT rc{};
    if (!GetClientRect(hwnd, &rc)) { DestroyWindow(hwnd); UnregisterClassW(kClassName, instance); return 4; }
    state.width = static_cast<UINT>(rc.right - rc.left);
    state.height = static_cast<UINT>(rc.bottom - rc.top);

    int exitCode = 0;
    try {
        Dx11App app;
        app.Initialize(hwnd, state.width, state.height);
        while (!state.quit) {
            MSG msg{};
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) { state.quit = true; break; }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            if (state.quit) break;
            if (IsIconic(hwnd)) { Sleep(30); continue; }
            if (state.resizePending) {
                app.Resize(state.width, state.height);
                state.resizePending = false;
            }
            app.Render();
        }
    } catch (const HrError& e) {
        std::ostringstream ss;
        ss << e.what() << " failed, HRESULT=0x" << std::hex
           << static_cast<unsigned long>(e.hr);
        MessageBoxA(hwnd, ss.str().c_str(), "Direct3D 11 error", MB_OK | MB_ICONERROR);
        exitCode = 1;
    } catch (const std::exception& e) {
        MessageBoxA(hwnd, e.what(), "Initialization error", MB_OK | MB_ICONERROR);
        exitCode = 1;
    }

    DestroyWindow(hwnd);
    UnregisterClassW(kClassName, instance);
    return exitCode;
}
```
## CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_init_contract LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 study default; 11 compatibility")
set(CMAKE_CXX_STANDARD ${LAB_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_executable(dx11_init_contract contract_model.cpp)
if(MSVC)
    target_compile_options(dx11_init_contract PRIVATE /W4 /EHsc)
else()
    target_compile_options(dx11_init_contract PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME dx11_init_contract COMMAND dx11_init_contract)

if(WIN32)
    add_executable(dx11_init_window WIN32 d3d11_init_window.cpp)
    target_compile_definitions(dx11_init_window PRIVATE UNICODE _UNICODE)
    target_link_libraries(dx11_init_window PRIVATE d3d11 dxgi)
    if(MSVC)
        target_compile_options(dx11_init_window PRIVATE /W4 /EHsc)
    endif()
endif()
```
## README
# DX11 init01 lab
- `contract_model.cpp`: CPU-only contract model for feature-level fallback, device/context roles, and swap-chain resize references.
- `d3d11_init_window.cpp`: complete Win32 + Direct3D 11 initialization/clear/resize sample. Not compiled in this Linux run.
- `CMakeLists.txt`: builds the CPU target everywhere and the Windows target only on WIN32.
Verified in this run:
- GCC 14.2 C++20: 24/24 checks, CTest 1/1
- GCC 14.2 C++11: 24/24 checks, CTest 1/1
- Clang 17 C++20 ASan+UBSan: 24/24 checks, CTest 1/1
- Windows/MSVC/D3D11/GPU: NOT RUN
