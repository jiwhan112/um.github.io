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

    void Resize(UINT width, UINT height) { // [한국어 해설] 백 버퍼 바인딩과 RTV/깊이 참조를 해제한 후 Resize하고 새 뷰를 만든다.
        if (width == 0 || height == 0) return;
        context_->OMSetRenderTargets(0, nullptr, nullptr);
        rtv_.Reset();
        dsv_.Reset();
        depth_.Reset();

        DX11_HR(swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
        RecreateBackBufferViews(width, height);
    }

    void Render() { // [한국어 해설] Clear와 Present만 수행하는 초기화 예제다. 장면 지오메트리 렌더 검사는 아니다.
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
    void CreateDeviceAndSwapChain(UINT width, UINT height) { // [한국어 해설] 최소 레거시 swap-chain helper 경로다. flip 모델의 모든 정책을 설명하는 예제가 아니다.
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
        if (hr == E_INVALIDARG) { // [한국어 해설] 11.0 런타임과 11_1 요청의 호환 재시도다. 다른 오류까지 디버그 옵션을 끄며 숨기지 않는다.
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                levels + 1, ARRAYSIZE(levels) - 1, D3D11_SDK_VERSION,
                &desc, swapChain_.GetAddressOf(), device_.GetAddressOf(),
                &featureLevel_, context_.GetAddressOf());
        }
        // In a debug build, do not silently hide a missing/mismatched debug layer.
        DX11_HR(hr);
    }

    void RecreateBackBufferViews(UINT width, UINT height) { // [한국어 해설] 새 백 버퍼로 RTV를 만들고 크기에 맞는 깊이 버퍼/DSV와 뷰포트를 다시 설정한다.
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

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) { // [한국어 해설] Windows 전용 예제이며 이번 클라우드에서는 컴파일·창 실행하지 않았다.
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