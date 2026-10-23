// DX9 Shader/Effect CPU learning model. No D3D9 API/GPU execution.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o shader_cpu && ./shader_cpu
#include <array>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

struct Vec2 { double x{}, y{}; };
struct Vec3 { double x{}, y{}, z{}; };
struct Vec4 { double x{}, y{}, z{}, w{}; };
struct Mat4 {
    std::array<std::array<double, 4>, 4> m{};
    static Mat4 identity() {
        Mat4 a{};
        for (int i = 0; i != 4; ++i) a.m[i][i] = 1;
        return a;
    }
    static Mat4 translation(double x, double y, double z) {
        Mat4 a = identity();
        a.m[3][0] = x; a.m[3][1] = y; a.m[3][2] = z;
        return a;
    }
};
Vec4 mul_row(Vec4 v, const Mat4& a) { // [한국어 해설] 행 벡터와 행렬의 곱이다. w=0인 방향에는 이동이 적용되지 않는다.
    const std::array<double, 4> p{v.x, v.y, v.z, v.w};
    Vec4 out;
    double r[4]{};
    for (int j = 0; j != 4; ++j)
        for (int i = 0; i != 4; ++i) r[j] += p[i] * a.m[i][j];
    out.x = r[0]; out.y = r[1]; out.z = r[2]; out.w = r[3];
    return out;
}
bool near(double a, double b, double eps = 1e-10) {
    return std::isfinite(a) && std::isfinite(b) && std::abs(a-b) <= eps;
}
bool near(Vec4 a, Vec4 b) {
    return near(a.x,b.x) && near(a.y,b.y) && near(a.z,b.z) && near(a.w,b.w);
}
bool inside_d3d9_clip(Vec4 p) {
    if (!(std::isfinite(p.x) && std::isfinite(p.y) &&
          std::isfinite(p.z) && std::isfinite(p.w) && p.w > 0)) return false;
    return -p.w <= p.x && p.x <= p.w && -p.w <= p.y && p.y <= p.w &&
           0 <= p.z && p.z <= p.w;
}
struct VSInput { Vec4 position; Vec2 uv; };
struct VSOutput { Vec4 clipPosition; Vec2 uv; double viewZ{}; };
VSOutput vertex_stage(VSInput in, const Mat4& worldView, const Mat4& worldViewProj) { // [한국어 해설] VS 계약을 CPU로 나타낸다. 실제 HLSL 컴파일이나 정점 셰이더 실행은 아니다.
    const auto view = mul_row(in.position, worldView);
    return {mul_row(in.position, worldViewProj), in.uv, view.z};
}
struct Interpolated { Vec2 uv; double viewZ{}; };
Interpolated perspective_interpolate(const std::array<VSOutput, 3>& v, // [한국어 해설] 화면 무게중심 λ를 w로 나눈 가중치를 사용해 UV와 뷰 깊이를 원근 보정한다.
                                     std::array<double, 3> bary) {
    double sum = 0;
    for (double b : bary) {
        if (!std::isfinite(b) || b < 0) throw std::invalid_argument("invalid barycentric component");
        sum += b;
    }
    if (!near(sum, 1.0, 1e-8)) throw std::invalid_argument("barycentric sum != 1");
    double denom = 0;
    double u = 0, texV = 0, z = 0;
    for (std::size_t i = 0; i != 3; ++i) {
        const double w = v[i].clipPosition.w;
        if (!(std::isfinite(w) && w > 0)) throw std::invalid_argument("nonpositive clip w");
        const double factor = bary[i] / w;
        denom += factor;
        u += factor * v[i].uv.x;
        texV += factor * v[i].uv.y;
        z += factor * v[i].viewZ;
    }
    if (!(std::isfinite(denom) && denom > 0)) throw std::invalid_argument("bad denominator");
    return {{u / denom, texV / denom}, z / denom};
}
double fog_visibility(double viewZ, double start, double end) { // [한국어 해설] 뷰 깊이로 선형 안개 가시율을 계산하고 [0,1]로 제한한다. start<end 계약을 검사한다.
    if (!(std::isfinite(start) && std::isfinite(end) && std::isfinite(viewZ) && end > start))
        throw std::invalid_argument("invalid fog range/depth");
    return std::clamp((end - viewZ) / (end - start), 0.0, 1.0);
}
Vec4 pixel_stage(Interpolated in, Vec4 sampled, Vec3 fogRGB, double start, double end) { // [한국어 해설] 안개는 RGB만 섞고 샘플 알파는 유지한다. 텍스처 샘플 값은 호출자가 제공한다.
    const double keep = fog_visibility(in.viewZ, start, end);
    return {fogRGB.x * (1-keep) + sampled.x * keep,
            fogRGB.y * (1-keep) + sampled.y * keep,
            fogRGB.z * (1-keep) + sampled.z * keep,
            sampled.w};
}
template <typename F>
bool throws_invalid(F fn) {
    try { fn(); }
    catch (const std::invalid_argument&) { return true; }
    return false;
}
int main() { // [한국어 해설] 복구한 원본의 34개 이름 있는 CPU 검사를 실행한다. GPU 화면 검증으로 해석하지 않는다.
    int checks = 0;
    auto check = [&](bool passed, const char* name) {
        ++checks;
        if (!passed) throw std::runtime_error(std::string("FAILED: ") + name);
    };
    try {
        const Mat4 I = Mat4::identity();
        check(near(mul_row({1,2,3,1},I),{1,2,3,1}), "identity");
        const Mat4 T = Mat4::translation(10,-2,5);
        check(near(mul_row({1,2,3,1},T),{11,0,8,1}), "row vector translate");
        check(near(mul_row({1,2,3,0},T),{1,2,3,0}), "direction unaffected");
        const VSInput input{{1,2,3,1},{0.25,0.75}};
        const VSOutput vertex = vertex_stage(input,T,I);
        check(near(vertex.clipPosition,{1,2,3,1}), "VS clip position");
        check(near(vertex.viewZ,8), "VS viewZ");
        check(near(vertex.uv.x,0.25) && near(vertex.uv.y,0.75), "VS uv passthrough");
        check(inside_d3d9_clip({0,0,0,1}), "near clip inclusive");
        check(inside_d3d9_clip({1,-1,1,1}), "far and side inclusive");
        check(!inside_d3d9_clip({0,0,-0.01,1}), "negative z reject");
        check(!inside_d3d9_clip({0,0,1.01,1}), "z beyond w reject");
        check(!inside_d3d9_clip({0,0,0,0}), "w zero reject");
        check(!inside_d3d9_clip({1.01,0,0.5,1}), "outside right reject");
        const std::array<VSOutput, 3> tri{{
            {{0,0,0.5,1},{0,0},2},
            {{0,0,1,2},{1,0},4},
            {{0,0,2,4},{0,1},8}
        }};
        const Interpolated sample = perspective_interpolate(tri,{1.0/3,1.0/3,1.0/3});
        check(near(sample.uv.x,2.0/7), "perspective u");
        check(near(sample.uv.y,1.0/7), "perspective v");
        check(near(sample.viewZ,24.0/7), "perspective view depth");
        check(!near(sample.uv.x,1.0/3), "not affine interpolation");
        const auto vertexOne = perspective_interpolate(tri,{0,1,0});
        check(near(vertexOne.uv.x,1) && near(vertexOne.viewZ,4), "corner interpolation");
        check(throws_invalid([&] { (void)perspective_interpolate(tri,{0.4,0.4,0.4}); }), "reject bary sum");
        check(throws_invalid([&] { (void)perspective_interpolate(tri,{-0.1,0.5,0.6}); }), "reject negative bary");
        auto badTri = tri; badTri[1].clipPosition.w = 0;
        check(throws_invalid([&] { (void)perspective_interpolate(badTri,{0.2,0.3,0.5}); }), "reject w zero");
        check(near(fog_visibility(1,2,8),1), "fog before start");
        check(near(fog_visibility(2,2,8),1), "fog at start");
        check(near(fog_visibility(5,2,8),0.5), "fog halfway");
        check(near(fog_visibility(8,2,8),0), "fog at end");
        check(near(fog_visibility(9,2,8),0), "fog after end");
        check(throws_invalid([&] { (void)fog_visibility(4,2,2); }), "fog equal range reject");
        check(throws_invalid([&] { (void)fog_visibility(4,8,2); }), "fog reversed range reject");
        check(throws_invalid([&] { (void)fog_visibility(std::numeric_limits<double>::quiet_NaN(),2,8); }), "fog NaN reject");
        const Vec4 col = pixel_stage(sample,{2.0/7,1.0/7,0.25,0.8},{0.2,0.3,0.4},2,8);
        const double v = 16.0/21;
        check(near(col.x,0.2*(1-v)+(2.0/7)*v), "pixel fog red");
        check(near(col.y,0.3*(1-v)+(1.0/7)*v), "pixel fog green");
        check(near(col.z,0.4*(1-v)+0.25*v), "pixel fog blue");
        check(near(col.w,0.8), "alpha preserved");
        check(near(pixel_stage({{0,0},8},{1,0,0,0.5},{0.2,0.3,0.4},2,8),{0.2,0.3,0.4,0.5}), "full fog");
        check(near(pixel_stage({{0,0},2},{1,0,0,0.5},{0.2,0.3,0.4},2,8),{1,0,0,0.5}), "no fog");
        std::cout << "PASS " << checks << " distinct named checks (CPU model only)\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
