#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "com_owner.h"

#pragma comment(lib, "d3d9.lib")

struct WindowState { bool resized = false; };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) { // [한국어 해설] 메시지는 크기 변경 상태만 표시한다. 실제 Reset은 렌더 루프의 장치 상태 확인 뒤 수행한다.
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

D3DPRESENT_PARAMETERS MakePresentParameters(HWND hwnd) { // [한국어 해설] 창 모드 Clear 전용 예제라 자동 깊이 버퍼를 사용하지 않는다. 이후 깊이 사용 실습과 구분한다.
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

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) { // [한국어 해설] Windows 전용 전체 예제다. 이번 클라우드 환경에서 컴파일·화면 실행하지 않았다.
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
                        // Later chapters must release D3DPOOL_DEFAULT resources here. // [한국어 해설] 이 예제에는 별도 DEFAULT 자원이 없다. 자원을 추가하면 Reset 전에 해당 참조를 해제해야 한다.
                        pp = MakePresentParameters(hwnd);
                        hr = device->Reset(&pp);
                        if (FAILED(hr)) {
                            if (hr != D3DERR_DEVICELOST) { code = 6; break; }
                            Sleep(30);
                            continue;
                        }
                        resetNeeded = false;
                        // Later chapters must reapply render states/recreate resources here. // [한국어 해설] Reset 성공 후에는 렌더 상태와 필요한 자원을 재설정한다. 기존 상태가 보존된다고 가정하지 않는다.
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
