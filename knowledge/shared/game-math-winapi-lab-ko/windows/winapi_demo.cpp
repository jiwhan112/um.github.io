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
// [한국어 해설] 메모리 DC와 비트맵을 소유하는 RAII 객체다. 복사를 금지해 GDI 핸들의 이중 해제를 막는다.
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
// [한국어 해설] 선택했던 이전 객체를 먼저 복원한 후 소유 비트맵과 DC를 해제한다.
    void Clear() noexcept {
        if (dc_ && previous_) SelectObject(dc_, previous_);
        if (bitmap_) DeleteObject(bitmap_);
        if (dc_) DeleteDC(dc_);
        dc_ = nullptr; bitmap_ = nullptr; previous_ = nullptr;
        width_ = height_ = 0;
    }
// [한국어 해설] 크기가 같으면 재사용한다. 대체 자원을 전부 만든 뒤 기존 버퍼를 교체한다.
    bool Ensure(HDC display, int width, int height) noexcept {
        if (width <= 0 || height <= 0) return false;
        if (dc_ && width == width_ && height == height_) return true;
        HDC nextDC = CreateCompatibleDC(display);
        if (!nextDC) return false;
        // Use the DISPLAY DC: a new memory DC initially has a monochrome bitmap.
// [한국어 해설] 새 메모리 DC의 기본 비트맵은 단색이다. 화면 DC를 기준으로 호환 비트맵을 만든다.
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
// [한국어 해설] 창보다 오래 살아야 하는 콜백 상태다. 화면 좌표는 오른쪽/아래쪽 논리 단위로 통일한다.
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
// [한국어 해설] 고정 dt로 이동한다. 대각선 입력을 정규화하여 축 방향보다 빨라지지 않도록 한다.
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
// [한국어 해설] BeginPaint와 EndPaint를 수명으로 묶는다. 예외 경로에서도 페인트 상태를 닫는다.
struct PaintScope {
    HWND hwnd;
    PAINTSTRUCT ps{};
    HDC dc;
    explicit PaintScope(HWND window) : hwnd(window), dc(BeginPaint(window, &ps)) {}
    ~PaintScope() { if (dc) EndPaint(hwnd, &ps); }
    PaintScope(const PaintScope&) = delete;
    PaintScope& operator=(const PaintScope&) = delete;
};
// [한국어 해설] SaveDC로 저장한 그리기 상태를 복원한다. stock 객체는 빌린 핸들이므로 DeleteObject하지 않는다.
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
// [한국어 해설] 버퍼 생성이나 BitBlt가 실패하면 화면 DC에 직접 그린다. 실패를 무조건 정상 버퍼 출력이라 숨기지 않는다.
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
// [한국어 해설] 운영체제 콜백 경계 밖으로 C++ 예외를 넘기지 않는다. 오류는 종료 상태로 변환한다.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    App* app = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    try {
// [한국어 해설] CreateWindow의 lpParam을 창별 상태 포인터로 연결한다. 반환값 0만으로 실패를 판단하지 않고 LastError도 본다.
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
// [한국어 해설] 창과 상태의 연결을 끊어 파괴된 HWND를 이후 루프에서 사용하지 않도록 한다.
        case WM_NCDESTROY:
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            app->hwnd = nullptr;
            return DefWindowProcW(hwnd, message, wParam, lParam);
// [한국어 해설] 비활성 전환에서 입력과 누적 시간을 초기화하여 복귀 직후 긴 시간만큼 뛰는 현상을 막는다.
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
// [한국어 해설] 새 DPI와 운영체제가 제안한 사각형을 반영한다. 완전한 DPI별 글꼴 재구성 예제는 아니다.
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
// [한국어 해설] 마우스 픽셀 좌표를 DPI 배율로 나누어 논리 좌표 목표로 변환한다.
        case WM_LBUTTONDOWN:
            SetFocus(hwnd);
            app->target = {static_cast<double>(GET_X_LPARAM(lParam))/app->Scale(),
                           static_cast<double>(GET_Y_LPARAM(lParam))/app->Scale()};
            app->following = true; return 0;
// [한국어 해설] Paint가 전체 배경을 채우므로 별도 배경 지우기를 생략한다.
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

// [한국어 해설] Windows 전용 진입점이다. Linux CPU 테스트 통과가 이 창의 실제 실행을 증명하지 않는다.
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
// [한국어 해설] 메시지 처리와 고정 Update를 분리한다. 메시지 폭주에도 Update 기회를 주도록 256개씩 제한한다.
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
// [한국어 해설] 중지 중에는 메시지를 기다리고 시간 기준을 초기화한다. 바쁜 대기 루프로 CPU를 소모하지 않는다.
            if (app.Paused()) {
                app.ResetTime();
                if (MsgWaitForMultipleObjectsEx(0,nullptr,INFINITE,QS_ALLINPUT,MWMO_INPUTAVAILABLE)==WAIT_FAILED)
                    throw std::runtime_error("Message wait failed");
                continue;
            }
// [한국어 해설] 측정한 실제 경과 시간을 stepper에 전달한다. 버린 시뮬레이션 시간은 디버그 출력으로 알린다.
            const auto frame = app.stepper.Advance(elapsed,[&app](double dt) { app.Update(dt); });
            if (frame.discardedSeconds > 0.0) OutputDebugStringW(L"Study demo dropped excess simulation time\n");
            InvalidateRect(app.hwnd,nullptr,FALSE);
            UpdateWindow(app.hwnd); // GDI presentation; not a display-vsync guarantee.
            if (processed == 256) continue;
// [한국어 해설] 다음 단계까지 남은 시간을 짧게 기다리되 메시지가 오면 먼저 깬다. 수직 동기화 보장은 아니다.
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
// [한국어 해설] App을 파괴하기 전에 창을 닫아 콜백이 이미 파괴된 상태 객체를 보지 않게 한다.
    if (app.hwnd) DestroyWindow(app.hwnd); // Before App and its callback data are destroyed.
    UnregisterClassW(className, instance);
    return exitCode;
}
