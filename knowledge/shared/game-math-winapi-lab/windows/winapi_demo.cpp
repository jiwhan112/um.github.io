// Windows 10 version 1703+ / Windows SDK. Reviewed source; NOT compiled or run
// in the Linux validation environment. No MFC, D3DX, or external assets needed.
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include <exception>
#include <stdexcept>
#include "loop.hpp"
#include "math.hpp"

namespace {
class BackBuffer {
    HDC dc_{};
    HBITMAP bitmap_{};
    HGDIOBJ previous_{};
    int width_{}, height_{};
public:
    BackBuffer() = default;
    BackBuffer(const BackBuffer&) = delete;
    BackBuffer& operator=(const BackBuffer&) = delete;
    ~BackBuffer() { Clear(); }
    void Clear() noexcept {
        if (dc_ && previous_) SelectObject(dc_, previous_);
        if (bitmap_) DeleteObject(bitmap_);
        if (dc_) DeleteDC(dc_);
        dc_ = nullptr; bitmap_ = nullptr; previous_ = nullptr;
        width_ = height_ = 0;
    }
    bool Ensure(HDC display, int width, int height) noexcept {
        if (width <= 0 || height <= 0) return false;
        if (dc_ && width == width_ && height == height_) return true;
        HDC nextDC = CreateCompatibleDC(display);
        if (!nextDC) return false;
        // Use the DISPLAY DC: a new memory DC initially has a monochrome bitmap.
        HBITMAP nextBitmap = CreateCompatibleBitmap(display, width, height);
        if (!nextBitmap) { DeleteDC(nextDC); return false; }
        HGDIOBJ old = SelectObject(nextDC, nextBitmap);
        if (!old || old == HGDI_ERROR) {
            DeleteObject(nextBitmap); DeleteDC(nextDC); return false;
        }
        Clear(); // Commit only after the replacement is ready.
        dc_ = nextDC; bitmap_ = nextBitmap; previous_ = old;
        width_ = width; height_ = height;
        return true;
    }
    HDC DC() const noexcept { return dc_; }
};
struct App {
    HWND hwnd{};
    UINT dpi{96};
    bool active{true}, minimized{}, sizing{}, resetClock{true}, fatal{}, following{};
    study::Vec2 position{200, 160}, target{200, 160};
    study::Input input;
    study::FixedStepper stepper;
    BackBuffer buffer;
    bool Paused() const noexcept { return !active || minimized || sizing; }
    double Scale() const noexcept { return static_cast<double>(dpi) / 96.0; }
    void ResetTime() noexcept { resetClock = true; stepper.Reset(); }
    void Update(double dt) {
        if (input.ConsumePressed(VK_SPACE)) {
            position = {200,160}; following = false;
        }
        study::Vec2 direction{
            static_cast<double>(input.Held(VK_RIGHT)) - static_cast<double>(input.Held(VK_LEFT)),
            static_cast<double>(input.Held(VK_DOWN)) - static_cast<double>(input.Held(VK_UP))};
        if (const auto unit = study::Normalize(direction)) {
            following = false;
            position = position + *unit * (180.0 * dt);
        } else if (following) {
            position = study::MoveTowards(position, target, 180.0, dt);
        }
        RECT client{};
        if (GetClientRect(hwnd, &client)) {
            const double width = static_cast<double>(client.right) / Scale();
            const double height = static_cast<double>(client.bottom) / Scale();
            position.x = std::clamp(position.x, 0.0, width);
            position.y = std::clamp(position.y, 0.0, height);
        }
    }
};
struct PaintScope {
    HWND hwnd;
    PAINTSTRUCT ps{};
    HDC dc;
    explicit PaintScope(HWND window) : hwnd(window), dc(BeginPaint(window, &ps)) {}
    ~PaintScope() { if (dc) EndPaint(hwnd, &ps); }
    PaintScope(const PaintScope&) = delete;
    PaintScope& operator=(const PaintScope&) = delete;
};
void DrawScene(HDC dc, const RECT& client, const App& app) {
    const int saved = SaveDC(dc);
    if (!saved) throw std::runtime_error("SaveDC failed");
    struct Restore { HDC dc; int state; ~Restore() { RestoreDC(dc, state); } } restore{dc, saved};
    if (!FillRect(dc, &client, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH))))
        throw std::runtime_error("FillRect failed");
    SelectObject(dc, GetStockObject(DC_PEN));
    SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCPenColor(dc, RGB(35,55,90));
    SetDCBrushColor(dc, RGB(90,155,225));
    const double scale = app.Scale();
    const auto pixel = [scale](double value) { return static_cast<int>(std::lround(value * scale)); };
    const int x = pixel(app.position.x), y = pixel(app.position.y), radius = pixel(14);
    Ellipse(dc, x-radius, y-radius, x+radius, y+radius);
    if (app.following) {
        SetDCPenColor(dc, RGB(190,60,60));
        const int tx = pixel(app.target.x), ty = pixel(app.target.y), r = pixel(7);
        MoveToEx(dc, tx-r, ty, nullptr); LineTo(dc, tx+r, ty);
        MoveToEx(dc, tx, ty-r, nullptr); LineTo(dc, tx, ty+r);
    }
    // Stock font is borrowed. The shape/UI coordinate scale is demonstrated;
    // full per-DPI typography and text layout are intentionally not implemented.
    SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc, TRANSPARENT);
    RECT text{12,12,client.right-12,70};
    DrawTextW(dc, L"Arrows: move | Click: follow | Space: reset | Esc: close\nFixed update: 120 Hz; active-window input only", -1,
              &text, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
}
void Paint(App& app) {
    PaintScope paint(app.hwnd);
    if (!paint.dc) throw std::runtime_error("BeginPaint failed");
    RECT client{};
    if (!GetClientRect(app.hwnd, &client)) throw std::runtime_error("GetClientRect failed");
    const int width = static_cast<int>(client.right), height = static_cast<int>(client.bottom);
    if (width <= 0 || height <= 0) return;
    if (app.buffer.Ensure(paint.dc, width, height)) {
        DrawScene(app.buffer.DC(), client, app);
        if (!BitBlt(paint.dc,0,0,width,height,app.buffer.DC(),0,0,SRCCOPY))
            DrawScene(paint.dc, client, app);
    } else {
        DrawScene(paint.dc, client, app); // Allocation failure fallback.
    }
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    App* app = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    try {
        if (message == WM_NCCREATE) {
            const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
            app = static_cast<App*>(create->lpCreateParams);
            if (!app) return FALSE;
            SetLastError(0);
            const LONG_PTR previous = SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
            if (previous == 0 && GetLastError() != 0) return FALSE;
            app->hwnd = hwnd;
            const UINT dpi = GetDpiForWindow(hwnd);
            if (dpi) app->dpi = dpi;
            return TRUE;
        }
        if (!app) return DefWindowProcW(hwnd, message, wParam, lParam);
        switch (message) {
        case WM_CLOSE:
            DestroyWindow(hwnd); return 0;
        case WM_DESTROY:
            PostQuitMessage(app->fatal ? 1 : 0); return 0;
        case WM_NCDESTROY:
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            app->hwnd = nullptr;
            return DefWindowProcW(hwnd, message, wParam, lParam);
        case WM_ACTIVATEAPP:
            app->active = wParam != 0;
            app->input.Clear(); app->ResetTime(); return 0;
        case WM_KILLFOCUS:
            app->input.Clear(); return 0;
        case WM_ENTERSIZEMOVE:
            app->sizing = true; app->ResetTime(); return 0;
        case WM_EXITSIZEMOVE:
            app->sizing = false; app->ResetTime(); return 0;
        case WM_SIZE:
            app->minimized = wParam == SIZE_MINIMIZED;
            app->ResetTime(); InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_DPICHANGED:
            app->dpi = static_cast<UINT>(HIWORD(wParam));
            if (app->dpi == 0) app->dpi = 96;
            if (const auto* rect = reinterpret_cast<const RECT*>(lParam)) {
                SetWindowPos(hwnd, nullptr, static_cast<int>(rect->left), static_cast<int>(rect->top),
                             static_cast<int>(rect->right-rect->left), static_cast<int>(rect->bottom-rect->top),
                             SWP_NOACTIVATE | SWP_NOZORDER);
            }
            InvalidateRect(hwnd, nullptr, FALSE); return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) { PostMessageW(hwnd, WM_CLOSE, 0, 0); return 0; }
            app->input.Key(static_cast<std::size_t>(wParam), true); return 0;
        case WM_KEYUP:
            app->input.Key(static_cast<std::size_t>(wParam), false); return 0;
        case WM_LBUTTONDOWN:
            SetFocus(hwnd);
            app->target = {static_cast<double>(GET_X_LPARAM(lParam))/app->Scale(),
                           static_cast<double>(GET_Y_LPARAM(lParam))/app->Scale()};
            app->following = true; return 0;
        case WM_ERASEBKGND:
            return 1; // Paint() fills the complete client area.
        case WM_PAINT:
            Paint(*app); return 0;
        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
    } catch (const std::exception& error) {
        OutputDebugStringA(error.what());
    } catch (...) {
        OutputDebugStringW(L"Unexpected exception in WindowProc");
    }
    // No C++ exception is allowed to cross the OS callback boundary.
    if (app) app->fatal = true;
    PostQuitMessage(1);
    return message == WM_NCCREATE ? FALSE : 0;
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    App app;
    int exitCode = 0;
    const wchar_t* className = L"StudyMathWinApiDemoV1";
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = static_cast<UINT>(sizeof(windowClass));
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = className;
    if (!RegisterClassExW(&windowClass)) return 1;
    try {
        LARGE_INTEGER frequency{}, previous{};
        if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0 || !QueryPerformanceCounter(&previous))
            throw std::runtime_error("Performance counter unavailable");
        HWND window = CreateWindowExW(0, className, L"Game Math + WinAPI Lab", WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT, 900, 650, nullptr, nullptr, instance, &app);
        if (!window) throw std::runtime_error("CreateWindowExW failed");
        // 800 x 600 logical units at the DPI of the newly created window.
        const int width = static_cast<int>(std::lround(800.0 * app.Scale()));
        const int height = static_cast<int>(std::lround(600.0 * app.Scale()));
        RECT desired{0,0,width,height};
        if (AdjustWindowRectExForDpi(&desired,WS_OVERLAPPEDWINDOW,FALSE,0,app.dpi))
            SetWindowPos(window,nullptr,0,0,static_cast<int>(desired.right-desired.left),
                         static_cast<int>(desired.bottom-desired.top),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        ShowWindow(window, showCommand);
        UpdateWindow(window);
        bool running = true;
        while (running && !app.fatal) {
            MSG message{};
            int processed = 0;
            while (processed < 256 && PeekMessageW(&message,nullptr,0,0,PM_REMOVE)) {
                ++processed;
                if (message.message == WM_QUIT) {
                    exitCode = static_cast<int>(message.wParam); running = false; break;
                }
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            if (!running || app.fatal || !app.hwnd) break;
            LARGE_INTEGER now{};
            if (!QueryPerformanceCounter(&now)) throw std::runtime_error("Performance counter failed");
            double elapsed = static_cast<double>(now.QuadPart-previous.QuadPart) /
                             static_cast<double>(frequency.QuadPart);
            previous = now;
            if (app.resetClock) { elapsed = 0.0; app.resetClock = false; }
            if (app.Paused()) {
                app.ResetTime();
                if (MsgWaitForMultipleObjectsEx(0,nullptr,INFINITE,QS_ALLINPUT,MWMO_INPUTAVAILABLE)==WAIT_FAILED)
                    throw std::runtime_error("Message wait failed");
                continue;
            }
            const auto frame = app.stepper.Advance(elapsed,[&app](double dt) { app.Update(dt); });
            if (frame.discardedSeconds > 0.0) OutputDebugStringW(L"Study demo dropped excess simulation time\n");
            InvalidateRect(app.hwnd,nullptr,FALSE);
            UpdateWindow(app.hwnd); // GDI presentation; not a display-vsync guarantee.
            if (processed == 256) continue;
            const double remainingMs = (1.0-frame.alpha)*study::FixedStepper::step*1000.0;
            const DWORD timeout = static_cast<DWORD>(std::clamp(std::ceil(remainingMs),1.0,9.0));
            if (MsgWaitForMultipleObjectsEx(0,nullptr,timeout,QS_ALLINPUT,MWMO_INPUTAVAILABLE)==WAIT_FAILED)
                throw std::runtime_error("Frame wait failed");
        }
    } catch (const std::exception& error) {
        OutputDebugStringA(error.what());
        MessageBoxW(app.hwnd,L"The study example stopped after an API failure. See debugger output.",
                    L"Study example",MB_OK|MB_ICONERROR);
        exitCode = 1;
    } catch (...) {
        exitCode = 1;
    }
    if (app.fatal) exitCode = 1;
    if (app.hwnd) DestroyWindow(app.hwnd); // Before App and its callback data are destroyed.
    UnregisterClassW(className, instance);
    return exitCode;
}
