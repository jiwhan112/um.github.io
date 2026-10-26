#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// GPU 호출이 아닌, 이 단원의 명시적 계약을 검증하는 결정적 CPU 모델이다.
static unsigned checks = 0;
void Check(bool condition, const char* name) {
    ++checks;
    if (!condition) throw std::runtime_error(name); // Release에서도 검사를 유지한다.
}
template < class F > void Reject(F action, const char* name) {
    bool rejected = false;
    try {
        action();
    } catch (const std::exception&) {
        rejected = true;
    }
    Check(rejected, name);
}

enum class State { Common, CopyDest, PixelRead, RenderTarget, Uav };
// 잘못된 before 상태는 실제 추적 상태를 바꾸기 전에 거절한다.
struct Resource {
    State state = State::Common;
    void Transition(State before, State after) {
        if (state != before) throw std::logic_error("before state mismatch");
        state = after;
    }
    void Sample() const { if (state != State::PixelRead) throw std::logic_error("not readable");
    }
    void Draw() const { if (state != State::RenderTarget) throw std::logic_error("not RTV");
    }
};
int main() {
    try {
        Resource r;
        Reject([&] { r.Sample(); }, "no implicit promotion in model");
        r.Transition(State::Common, State::CopyDest);
        Check(r.state == State::CopyDest, "copy dest");
        Reject([&] { r.Transition(State::Common, State::PixelRead); }, "incorrect before");
        Check(r.state == State::CopyDest, "failure preserves tracked state");
        r.Transition(State::CopyDest, State::PixelRead);
        r.Sample();
        Check(true, "read after transition");
        Reject([&] { r.Draw(); }, "SRV not RTV");
        r.Transition(State::PixelRead, State::RenderTarget);
        r.Draw();
        Check(true, "render target");
        r.Transition(State::RenderTarget, State::Common);
        Check(r.state == State::Common, "present/common conceptual endpoint");
        std::cout << "PASS DX12-08: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
