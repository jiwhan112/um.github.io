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

CreateAttempt TryCreate(const RuntimeCaps& caps, // [한국어 해설] 런타임·디버그 계층·요청 feature level의 계약을 CPU로 모형화한다. 실제 API 반환값을 조회하지 않는다.
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

NegotiatedDevice CreateWithRuntimeFallback(const RuntimeCaps& caps, bool requestDebug) { // [한국어 해설] 11_1 항목 때문에 E_INVALIDARG가 난 경우만 목록에서 제외하고 재시도한다. 디버그 계층 실패는 숨기지 않는다.
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

class ContextModel { // [한국어 해설] Device는 생성, Context는 바인딩·명령 역할로 분리하는 학습 모델이다.
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

class SwapChainModel { // [한국어 해설] 백 버퍼 간접 참조와 바인딩을 추적한다. Resize 전에 둘 다 해제해야 하는 이유를 보여 준다.
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

    bool ResizeBuffers(unsigned width, unsigned height) { // [한국어 해설] 0크기와 남은 참조를 거부하고 성공할 때 자원 ID/세대를 갱신한다.
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

int main() { // [한국어 해설] feature level 협상과 Resize 계약을 CPU에서 검사한다. Windows 창 실행의 증거는 아니다.
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