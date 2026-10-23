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
// [한국어 해설] 학습 모델의 입력 계약을 거부할 때 invalid_argument를 사용한다. 실제 D3D9 드라이버 검증 함수는 아니다.
inline void Require(bool ok, const char* message) {
    if (!ok) throw std::invalid_argument(message);
}
using Vec4 = std::array<double, 4>;
using Mat4 = std::array<std::array<double, 4>, 4>;
struct Vec2 { double x{}, y{}; };
// [한국어 해설] 유한성뿐 아니라 절댓값 10억 이하로 범위를 제한한다. 전체 IEEE754 공간을 처리하는 범용 수학 라이브러리가 아니다.
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
// [한국어 해설] 행 벡터 규약이므로 이동은 마지막 행에 놓인다. GPU 상수에 올리는 float 행렬과 CPU double 배열을 혼동하지 않는다.
inline Mat4 Translation(double x, double y, double z) {
    Require(Finite({x,y,z,1}), "translation range");
    auto m=Identity(); m[3]={x,y,z,1}; return m;
}
// [한국어 해설] 행렬 곱의 순서는 교환할 수 없다. 행 벡터의 W*V*P는 월드→뷰→투영 순서다.
inline Mat4 Multiply(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (std::size_t i=0; i<4; ++i)
        for (std::size_t j=0; j<4; ++j)
            for (std::size_t k=0; k<4; ++k) r[i][j]+=a[i][k]*b[k][j];
    for (auto row:r) Require(Finite(row), "matrix result range");
    return r;
}
// [한국어 해설] v*M으로 클립 좌표 4성분을 만든다. 여기서 w로 나누면 이후 동차 클리핑 계약이 깨진다.
inline Vec4 Transform(Vec4 v, const Mat4& m) {
    Require(Finite(v), "vertex range");
    Vec4 r{};
    for (std::size_t j=0; j<4; ++j)
        for (std::size_t i=0; i<4; ++i) r[j]+=v[i]*m[i][j];
    Require(Finite(r), "transform result range"); return r;
}
// Row vectors, +Z forward, conventional near->0 / far->1 depth mapping.
// [한국어 해설] 왼손 +Z 전방, near→0/far→1 투영이다. near는 양수이고 far보다 작아야 한다.
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
// [한국어 해설] 클리핑 중에는 동차 위치와 원시 UV를 함께 유지한다. UV/w는 래스터 보간 단계에서 계산한다.
struct Vertex { Vec4 clip{}; Vec2 uv{}; };
// [한국어 해설] D3D형 동차 클립 부피의 여섯 반공간: -w≤x≤w, -w≤y≤w, 0≤z≤w.
inline double PlaneDistance(Vec4 p, std::size_t plane) {
    switch(plane) {
    case 0: return p[0]+p[3]; case 1: return p[3]-p[0];
    case 2: return p[1]+p[3]; case 3: return p[3]-p[1];
    case 4: return p[2]; case 5: return p[3]-p[2];
    default: throw std::invalid_argument("plane index");
    }
}
// [한국어 해설] 정점 하나의 내부 여부다. 세 정점 모두 바깥이라는 이유만으로 삼각형 전체를 버릴 수는 없다.
inline bool Inside(Vec4 p, double tolerance=0) {
    if (!Finite(p)) return false;
    for (std::size_t k=0;k<6;++k)
        if (PlaneDistance(p,k)<-tolerance) return false;
    return true;
}
// [한국어 해설] 클립 공간 교점의 같은 t로 위치와 원시 속성을 보간한다.
inline Vertex Lerp(Vertex a, Vertex b, double t) {
    Vertex r{};
    for (std::size_t k=0;k<4;++k) r.clip[k]=std::lerp(a.clip[k],b.clip[k],t);
    r.uv={std::lerp(a.uv.x,b.uv.x,t),std::lerp(a.uv.y,b.uv.y,t)};
    return r;
}
// Convex polygon clipping, preserving raw (not uv/w) attributes until rasterization.
// [한국어 해설] 볼록 다각형을 여섯 평면에 순차 절단한다. 입력 제한은 64정점이며 일반 오목 다각형 분할기는 아니다.
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
// [한국어 해설] 변의 양끝이 평면의 다른 쪽이면 t=da/(da-db)로 교점을 추가한다.
            if (in_a!=in_b) output.push_back(Lerp(a,b,da/(da-db)));
            if (in_b) output.push_back(b);
            a=b; da=db;
        }
        polygon=std::move(output);
    }
    return polygon;
}
struct Viewport { double x{},y{},width{},height{},min_z{},max_z{1}; };
// [한국어 해설] 화면 xy, 투영 후 깊이, 1/w와 원시 UV를 보관해 깊이 보간과 UV 보간을 구별한다.
struct ScreenVertex { Vec2 p{}; double depth{}, inv_w{}; Vec2 uv{}; };
// [한국어 해설] 클리핑 뒤 양수 w를 확인하고 원근 나눗셈과 뷰포트 변환을 적용한다. 화면 y는 아래쪽이다.
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
// [한국어 해설] 유향 변에 대한 부호 있는 면적이다. 감김 순서와 내부 판정에 사용하지만 이 함수 자체가 컬링을 수행하지 않는다.
inline double Edge(Vec2 a, Vec2 b, Vec2 p) {
    return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);
}
// [한국어 해설] 화면 y 아래쪽·양의 면적 기준 공유 변의 소유 규칙이다. 인접 삼각형의 중복 덮기를 피한다.
inline bool TopLeft(Vec2 a, Vec2 b) {
    const double dx=b.x-a.x, dy=b.y-a.y;
    return dy<0 || (dy==0 && dx>0); // screen Y points down; positive signed area
}
// [한국어 해설] 퇴화 삼각형을 거부하고 감김을 정규화한 뒤 top-left 규칙으로 한 샘플의 덮임을 검사한다.
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
// [한국어 해설] 면적 비로 화면 무게중심 좌표를 얻는다. 퇴화 입력은 nullopt이며 외부 점의 가중치도 계산될 수 있다.
inline std::optional<std::array<double,3>> Barycentric(std::array<Vec2,3> v, Vec2 p) {
    for (auto q:v) if (!Finite({q.x,q.y,p.x,p.y})) return std::nullopt;
    const double area=Edge(v[0],v[1],v[2]);
    if (!std::isfinite(area) || std::abs(area)<1e-12) return std::nullopt;
    return std::array<double,3>{Edge(v[1],v[2],p)/area,
                              Edge(v[2],v[0],p)/area,
                              Edge(v[0],v[1],p)/area};
}
// [한국어 해설] Σ(λ*UV/w)를 Σ(λ/w)로 나눠 원근 보정한다. 화면 UV의 단순 선형 보간과 다르다.
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
// [한국어 해설] 이미 z/w로 변환한 깊이는 화면 가중치로 선형 보간한다. UV처럼 다시 원근 보정하지 않는다.
inline double AffineDepth(std::array<ScreenVertex,3> v, std::array<double,3> l) {
    return l[0]*v[0].depth+l[1]*v[1].depth+l[2]*v[2].depth;
}
// One-dimensional, level-0 linear sampling with clamp addressing, for texel-center tests.
// [한국어 해설] 1차원·레벨0·선형·clamp만 모델링한다. 텍셀 중심은 (i+0.5)/N이며 밉맵·이방성 필터는 범위 밖이다.
inline double LinearClamp(std::span<const double> texels, double u) {
    Require(!texels.empty() && texels.size()<=10000 && std::isfinite(u), "sampler input");
    const int n=static_cast<int>(texels.size());
    const double x=std::clamp(u,0.0,1.0)*n-0.5;
    const int left=static_cast<int>(std::floor(x)); const double t=x-left;
    const auto at=[&](int i) { return texels[static_cast<std::size_t>(std::clamp(i,0,n-1))]; };
    return std::lerp(at(left),at(left+1),t);
}
// Checked address arithmetic for one stream. A CPU range check, not a driver query.
// [한국어 해설] 한 정점 스트림의 CPU 주소 계산이다. base/index는 정점 단위, stream/element offset과 stride는 바이트 단위다.
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
// [한국어 해설] 곱셈 전에 나눗셈 형태로 범위를 검사하여 큰 index가 주소 계산에서 감기는 것을 막는다.
    Require(vertex<=(bytes-start-element_size)/stride, "vertex outside buffer");
    return start+static_cast<std::size_t>(vertex)*stride;
}
struct Color { double r{},g{},b{},a{1}; };
struct Pixel { Color color{}; double depth{1}; };
// [한국어 해설] 깊이 검사와 기록, 블렌딩, premultiplied 여부, 알파 컷오프를 서로 다른 상태로 둔다.
struct OutputState { bool depth_test{true}, depth_write{true}, blend{}, premultiplied{};
                     double alpha_cutoff{-1}; };
// Scope: LESS_EQUAL depth, optional alpha >= cutoff, ADD/source-over, RGBA writes.
// Separate alpha factors ONE and INV_SRC_ALPHA are represented explicitly here.
// [한국어 해설] LESS_EQUAL 깊이 비교와 제한된 source-over 출력 모델이다. 모든 D3D9 렌더 상태를 재현하지 않는다.
inline bool Fragment(Pixel& destination, Color source, double depth, OutputState state) {
    Require(Finite({source.r,source.g,source.b,source.a}) && source.a>=0 && source.a<=1,
            "fragment color");
    Require(std::isfinite(depth) && depth>=0 && depth<=1, "fragment depth");
// [한국어 해설] 알파 컷오프 탈락은 색과 깊이를 모두 바꾸지 않는다. 알파가 0인 블렌딩과는 다르다.
    if (source.a<state.alpha_cutoff) return false;
    if (state.depth_test && depth>destination.depth) return false;
    if (state.blend) {
// [한국어 해설] straight RGB는 source alpha를 곱하고 premultiplied RGB는 이미 곱해졌으므로 1을 사용한다.
        const double sf=state.premultiplied?1:source.a, df=1-source.a;
        destination.color={source.r*sf+destination.color.r*df,
                           source.g*sf+destination.color.g*df,
                           source.b*sf+destination.color.b*df,
                           source.a+destination.color.a*df};
    } else destination.color=source;
    // D3DRS_ZENABLE=FALSE disables both test and depth-buffer updates.
// [한국어 해설] 이 모델에서 깊이 검사를 끄면 깊이 기록도 꺼진다. 기록 플래그만 true여도 갱신하지 않는다.
    if (state.depth_test && state.depth_write) destination.depth=depth;
    return true;
}
} // namespace pipeline
#endif

