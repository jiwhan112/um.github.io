#ifndef STUDY_DX9_PIPELINE_HPP
#define STUDY_DX9_PIPELINE_HPP
// Educational scalar model. Not a D3D9 driver, rasterizer conformance test,
// or production renderer. See README.md for the deliberately limited scope.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pipeline {
inline void Require(bool ok, const char* message) {
    if (!ok) throw std::invalid_argument(message);
}
using Vec4 = std::array<double, 4>;
using Mat4 = std::array<std::array<double, 4>, 4>;
struct Vec2 { double x{}, y{}; };
inline bool Finite(Vec4 v) {
    return std::all_of(v.begin(), v.end(), [](double x) {
        return std::isfinite(x) && std::abs(x) <= 1e9;
    });
}
inline Mat4 Identity() {
    Mat4 m{};
    for (std::size_t i=0; i<4; ++i) m[i][i]=1;
    return m;
}
inline Mat4 Translation(double x, double y, double z) {
    Require(Finite({x,y,z,1}), "translation range");
    auto m=Identity(); m[3]={x,y,z,1}; return m;
}
inline Mat4 Multiply(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (std::size_t i=0; i<4; ++i)
        for (std::size_t j=0; j<4; ++j)
            for (std::size_t k=0; k<4; ++k) r[i][j]+=a[i][k]*b[k][j];
    for (auto row:r) Require(Finite(row), "matrix result range");
    return r;
}
inline Vec4 Transform(Vec4 v, const Mat4& m) {
    Require(Finite(v), "vertex range");
    Vec4 r{};
    for (std::size_t j=0; j<4; ++j)
        for (std::size_t i=0; i<4; ++i) r[j]+=v[i]*m[i][j];
    Require(Finite(r), "transform result range"); return r;
}
// Row vectors, +Z forward, conventional near->0 / far->1 depth mapping.
inline Mat4 PerspectiveLH(double fov_y, double aspect, double near_z, double far_z) {
    constexpr double pi=3.14159265358979323846;
    Require(std::isfinite(fov_y) && fov_y>0 && fov_y<pi, "fov");
    Require(std::isfinite(aspect) && aspect>0, "aspect");
    Require(std::isfinite(near_z) && std::isfinite(far_z) &&
            near_z>0 && far_z>near_z, "near/far");
    const double sy=1/std::tan(fov_y/2), q=far_z/(far_z-near_z);
    Mat4 m{}; m[0][0]=sy/aspect; m[1][1]=sy; m[2][2]=q;
    m[2][3]=1; m[3][2]=-near_z*q;
    for (auto row:m) Require(Finite(row), "projection range");
    return m;
}
struct Vertex { Vec4 clip{}; Vec2 uv{}; };
inline double PlaneDistance(Vec4 p, std::size_t plane) {
    switch(plane) {
    case 0: return p[0]+p[3]; case 1: return p[3]-p[0];
    case 2: return p[1]+p[3]; case 3: return p[3]-p[1];
    case 4: return p[2]; case 5: return p[3]-p[2];
    default: throw std::invalid_argument("plane index");
    }
}
inline bool Inside(Vec4 p, double tolerance=0) {
    if (!Finite(p)) return false;
    for (std::size_t k=0;k<6;++k)
        if (PlaneDistance(p,k)<-tolerance) return false;
    return true;
}
inline Vertex Lerp(Vertex a, Vertex b, double t) {
    Vertex r{};
    for (std::size_t k=0;k<4;++k) r.clip[k]=std::lerp(a.clip[k],b.clip[k],t);
    r.uv={std::lerp(a.uv.x,b.uv.x,t),std::lerp(a.uv.y,b.uv.y,t)};
    return r;
}
// Convex polygon clipping, preserving raw (not uv/w) attributes until rasterization.
inline std::vector<Vertex> Clip(std::span<const Vertex> input) {
    Require(input.size()<=64, "polygon limit");
    for (auto v:input) Require(Finite(v.clip) &&
        std::isfinite(v.uv.x) && std::isfinite(v.uv.y) &&
        std::abs(v.uv.x)<=1e9 && std::abs(v.uv.y)<=1e9, "nonfinite vertex");
    std::vector<Vertex> polygon(input.begin(), input.end());
    for (std::size_t plane=0;plane<6 && !polygon.empty();++plane) {
        std::vector<Vertex> output;
        Vertex a=polygon.back(); double da=PlaneDistance(a.clip,plane);
        for (Vertex b:polygon) {
            const double db=PlaneDistance(b.clip,plane);
            const bool in_a=da>=0, in_b=db>=0;
            if (in_a!=in_b) output.push_back(Lerp(a,b,da/(da-db)));
            if (in_b) output.push_back(b);
            a=b; da=db;
        }
        polygon=std::move(output);
    }
    return polygon;
}
struct Viewport { double x{},y{},width{},height{},min_z{},max_z{1}; };
struct ScreenVertex { Vec2 p{}; double depth{}, inv_w{}; Vec2 uv{}; };
inline ScreenVertex Project(Vertex v, Viewport vp) {
    Require(Finite(v.clip) && v.clip[3]>1e-12, "w is not projectable");
    Require(Finite({vp.x,vp.y,vp.width,vp.height}) && vp.width>0 && vp.height>0,
            "viewport rectangle");
    Require(std::isfinite(vp.min_z) && std::isfinite(vp.max_z) &&
            vp.min_z>=0 && vp.min_z<=vp.max_z && vp.max_z<=1, "viewport depth");
    const double iw=1/v.clip[3];
    return {{vp.x+(v.clip[0]*iw+1)*vp.width/2,
             vp.y+(1-v.clip[1]*iw)*vp.height/2},
            vp.min_z+v.clip[2]*iw*(vp.max_z-vp.min_z), iw, v.uv};
}
inline double Edge(Vec2 a, Vec2 b, Vec2 p) {
    return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);
}
inline bool TopLeft(Vec2 a, Vec2 b) {
    const double dx=b.x-a.x, dy=b.y-a.y;
    return dy<0 || (dy==0 && dx>0); // screen Y points down; positive signed area
}
inline bool Covers(std::array<Vec2,3> v, Vec2 p) {
    for (auto q:v) if (!Finite({q.x,q.y,p.x,p.y})) return false;
    const double area=Edge(v[0],v[1],v[2]);
    if (!std::isfinite(area) || std::abs(area)<1e-12) return false;
    if (area<0) std::swap(v[1],v[2]);
    for (std::size_t k=0;k<3;++k) {
        const auto a=v[k], b=v[(k+1)%3]; const double e=Edge(a,b,p);
        if (e<0 || (e==0 && !TopLeft(a,b))) return false;
    }
    return true;
}
inline std::optional<std::array<double,3>> Barycentric(std::array<Vec2,3> v, Vec2 p) {
    for (auto q:v) if (!Finite({q.x,q.y,p.x,p.y})) return std::nullopt;
    const double area=Edge(v[0],v[1],v[2]);
    if (!std::isfinite(area) || std::abs(area)<1e-12) return std::nullopt;
    return std::array<double,3>{Edge(v[1],v[2],p)/area,
                              Edge(v[2],v[0],p)/area,
                              Edge(v[0],v[1],p)/area};
}
inline Vec2 PerspectiveUV(std::array<ScreenVertex,3> v, std::array<double,3> l) {
    double sum=0; Vec2 uv{};
    for (std::size_t k=0;k<3;++k) {
        Require(std::isfinite(l[k]) && std::isfinite(v[k].inv_w) && v[k].inv_w>0,
                "interpolation input");
        const double q=l[k]*v[k].inv_w; sum+=q;
        uv.x+=q*v[k].uv.x; uv.y+=q*v[k].uv.y;
    }
    Require(std::isfinite(sum) && sum>1e-12, "interpolation denominator");
    return {uv.x/sum,uv.y/sum};
}
inline double AffineDepth(std::array<ScreenVertex,3> v, std::array<double,3> l) {
    return l[0]*v[0].depth+l[1]*v[1].depth+l[2]*v[2].depth;
}
// One-dimensional, level-0 linear sampling with clamp addressing, for texel-center tests.
inline double LinearClamp(std::span<const double> texels, double u) {
    Require(!texels.empty() && texels.size()<=10000 && std::isfinite(u), "sampler input");
    const int n=static_cast<int>(texels.size());
    const double x=std::clamp(u,0.0,1.0)*n-0.5;
    const int left=static_cast<int>(std::floor(x)); const double t=x-left;
    const auto at=[&](int i) { return texels[static_cast<std::size_t>(std::clamp(i,0,n-1))]; };
    return std::lerp(at(left),at(left+1),t);
}
// Checked address arithmetic for one stream. A CPU range check, not a driver query.
inline std::size_t FetchOffset(std::size_t bytes, std::size_t stream_offset,
                              std::size_t stride, std::int64_t base,
                              std::uint32_t index, std::size_t element_offset,
                              std::size_t element_size) {
    Require(stride>0 && element_size>0 && element_offset<=stride &&
            element_size<=stride-element_offset, "vertex element layout");
    Require(base>=std::numeric_limits<std::int32_t>::min() &&
            base<=std::numeric_limits<std::int32_t>::max(), "base index range");
    const std::int64_t absolute=base+static_cast<std::int64_t>(index);
    Require(absolute>=0, "negative vertex index");
    const auto vertex=static_cast<std::uint64_t>(absolute);
    Require(stream_offset<=bytes && element_offset<=bytes-stream_offset, "stream offset");
    const std::size_t start=stream_offset+element_offset;
    Require(element_size<=bytes-start, "truncated element");
    Require(vertex<=(bytes-start-element_size)/stride, "vertex outside buffer");
    return start+static_cast<std::size_t>(vertex)*stride;
}
struct Color { double r{},g{},b{},a{1}; };
struct Pixel { Color color{}; double depth{1}; };
struct OutputState { bool depth_test{true}, depth_write{true}, blend{}, premultiplied{};
                     double alpha_cutoff{-1}; };
// Scope: LESS_EQUAL depth, optional alpha >= cutoff, ADD/source-over, RGBA writes.
// Separate alpha factors ONE and INV_SRC_ALPHA are represented explicitly here.
inline bool Fragment(Pixel& destination, Color source, double depth, OutputState state) {
    Require(Finite({source.r,source.g,source.b,source.a}) && source.a>=0 && source.a<=1,
            "fragment color");
    Require(std::isfinite(depth) && depth>=0 && depth<=1, "fragment depth");
    if (source.a<state.alpha_cutoff) return false;
    if (state.depth_test && depth>destination.depth) return false;
    if (state.blend) {
        const double sf=state.premultiplied?1:source.a, df=1-source.a;
        destination.color={source.r*sf+destination.color.r*df,
                           source.g*sf+destination.color.g*df,
                           source.b*sf+destination.color.b*df,
                           source.a+destination.color.a*df};
    } else destination.color=source;
    // D3DRS_ZENABLE=FALSE disables both test and depth-buffer updates.
    if (state.depth_test && state.depth_write) destination.depth=depth;
    return true;
}
} // namespace pipeline
#endif

