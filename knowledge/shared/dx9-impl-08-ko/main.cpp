#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

// Cross-platform CPU model, NOT a D3D9 implementation.
enum Cull { CullNone, CullCW, CullCCW };
struct State {
    bool depthTest;
    bool depthWrite;
    bool alphaBlend;
    Cull cull;
};
bool same(const State& a, const State& b) {
    return a.depthTest == b.depthTest && a.depthWrite == b.depthWrite &&
           a.alphaBlend == b.alphaBlend && a.cull == b.cull;
}
struct DeviceModel { State state; };
class ScopedState { // [한국어 해설] 범위를 벗어나면 CPU 상태 스냅샷을 복원한다. 실제 D3D9 StateBlock 실행 검사가 아니다.
public:
    explicit ScopedState(DeviceModel& dev) : device_(dev), old_(dev.state) {}
    ~ScopedState() { device_.state = old_; }
private:
    ScopedState(const ScopedState&);
    ScopedState& operator=(const ScopedState&);
    DeviceModel& device_;
    State old_;
};
void set_sky_first(DeviceModel& d) { // [한국어 해설] sky-first 정책은 깊이 검사와 기록을 끈다. 다른 스카이 렌더 정책과 혼용하지 않는다.
    d.state = State{false, false, false, CullNone};
}
void set_opaque(DeviceModel& d) {
    d.state = State{true, true, false, CullCCW};
}
void set_ui(DeviceModel& d) {
    d.state = State{false, false, true, CullNone};
}
struct Point { double x, y; };
bool pixel_to_centered_ortho(double x, double y, double width, double height, // [한국어 해설] 픽셀 y 아래쪽을 중심 원점 y 위쪽으로 변환한다. 이 원문 모델은 NaN 거부를 구현하지 않는다.
                            Point& out) {
    if (width <= 0 || height <= 0 || x < 0 || y < 0 || x > width || y > height)
        return false;
    out = Point{x - width / 2.0, height / 2.0 - y};
    return true;
}
Point centered_ortho_to_pixel(const Point& p, double width, double height) {
    return Point{p.x + width / 2.0, height / 2.0 - p.y};
}
struct Color { double r, g, b; };
struct Sample { Color color; double depth; };
void draw_opaque(Sample& dst, const Color& src, double depth, const State& s) { // [한국어 해설] 학습 모델의 LESSEQUAL 판정이다. 실제 장치 전체 렌더 상태를 재현하지 않는다.
    if (s.depthTest && depth > dst.depth) return; // educational ZFUNC=LESSEQUAL
    dst.color = src;
    if (s.depthWrite) dst.depth = depth;
}
void draw_sky_first(Sample& dst, const Color& sky, const State& s) {
    assert(!s.depthTest && !s.depthWrite && !s.alphaBlend);
    dst.color = sky;
}
void draw_translucent(Sample& dst, const Color& src, double a, const State& s) { // [한국어 해설] straight alpha RGB 합성을 계산하며 이 함수는 깊이 비교를 별도로 하지 않는다.
    assert(s.alphaBlend && !s.depthWrite);
    dst.color = Color{a*src.r+(1-a)*dst.color.r,
                      a*src.g+(1-a)*dst.color.g,
                      a*src.b+(1-a)*dst.color.b};
}
bool near(double a, double b) { return std::fabs(a - b) < 1e-10; }
int main() { // [한국어 해설] sky→opaque→UI 이후 상태 복원과 화면좌표 왕복·경계를 검사한다.
    unsigned checks = 0;
#define CHECK(c) do { ++checks; if (!(c)) throw std::runtime_error(#c); } while (0)
    DeviceModel device{State{true,true,false,CullCCW}};
    State original = device.state;
    Sample framebuffer{{0,0,0}, 1.0};
    {
        ScopedState restore(device);
        set_sky_first(device);
        CHECK(!device.state.depthTest);
        CHECK(!device.state.depthWrite);
        CHECK(device.state.cull == CullNone);
        draw_sky_first(framebuffer, Color{0.2,0.4,0.8}, device.state);
        CHECK(near(framebuffer.color.b, 0.8));
        CHECK(near(framebuffer.depth, 1.0));
        set_opaque(device);
        CHECK(device.state.depthTest);
        CHECK(device.state.depthWrite);
        draw_opaque(framebuffer, Color{1,0,0}, 0.3, device.state);
        CHECK(near(framebuffer.color.r, 1.0));
        CHECK(near(framebuffer.depth, 0.3));
        draw_opaque(framebuffer, Color{0,1,0}, 0.8, device.state);
        CHECK(near(framebuffer.color.r, 1.0)); // occluded
        set_ui(device);
        CHECK(device.state.alphaBlend);
        CHECK(!device.state.depthTest);
        CHECK(!device.state.depthWrite);
        draw_translucent(framebuffer, Color{0,0,1}, 0.5, device.state);
        CHECK(near(framebuffer.color.r, 0.5));
        CHECK(near(framebuffer.color.b, 0.5));
        CHECK(near(framebuffer.depth, 0.3));
    }
    CHECK(same(device.state, original)); // no state leak
    Point p{};
    CHECK(pixel_to_centered_ortho(0,0,800,600,p));
    CHECK(near(p.x, -400));
    CHECK(near(p.y, 300));
    CHECK(pixel_to_centered_ortho(400,300,800,600,p));
    CHECK(near(p.x, 0));
    CHECK(near(p.y, 0));
    CHECK(pixel_to_centered_ortho(800,600,800,600,p));
    CHECK(near(p.x, 400));
    CHECK(near(p.y, -300));
    Point q = centered_ortho_to_pixel(p,800,600);
    CHECK(near(q.x, 800));
    CHECK(near(q.y, 600));
    CHECK(!pixel_to_centered_ortho(1,1,0,600,p));
    CHECK(!pixel_to_centered_ortho(-1,1,800,600,p));
    CHECK(!pixel_to_centered_ortho(10,650,800,600,p));
    CHECK(!pixel_to_centered_ortho(1,1,800,-1,p));
    std::cout << "PASS " << checks << " explicit checks\n";
}