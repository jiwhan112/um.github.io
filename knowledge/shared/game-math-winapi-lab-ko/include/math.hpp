#ifndef STUDY_GAME_MATH_HPP
#define STUDY_GAME_MATH_HPP
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <utility>

namespace study {
// Educational scalar math. Finite, moderate-magnitude game coordinates are assumed
// by raw arithmetic operators; checked public algorithms reject invalid inputs.
// [한국어 해설] 유한한 보통 크기의 좌표를 전제로 한다. 기본 산술 자체가 모든 오버플로를 검사하지는 않는다.
struct Vec2 { double x{}, y{}; };
struct Vec3 { double x{}, y{}, z{}; };
inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x+b.x, a.y+b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x-b.x, a.y-b.y}; }
inline Vec2 operator*(Vec2 a, double s) { return {a.x*s, a.y*s}; }
inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
inline Vec3 operator*(Vec3 a, double s) { return {a.x*s, a.y*s, a.z*s}; }
inline bool Finite(Vec2 v) { return std::isfinite(v.x)&&std::isfinite(v.y); }
inline bool Finite(Vec3 v) { return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z); }
inline double Dot(Vec2 a, Vec2 b) { return a.x*b.x+a.y*b.y; }
inline double Dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
inline double Cross2(Vec2 a, Vec2 b) { return a.x*b.y-a.y*b.x; }
// [한국어 해설] 외적은 두 벡터에 수직이며 입력 순서를 바꾸면 부호가 뒤집힌다.
inline Vec3 Cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
// [한국어 해설] 길이가 0에 가깝거나 NaN·무한대이면 방향을 정의하지 않고 nullopt를 반환한다.
inline std::optional<Vec3> Normalize(Vec3 v, double epsilon=1e-12) {
    if (!Finite(v)||!std::isfinite(epsilon)||epsilon<0) return std::nullopt;
    const double length=std::hypot(v.x,v.y,v.z);
    if (!std::isfinite(length)||length<=epsilon) return std::nullopt;
    return Vec3{v.x/length,v.y/length,v.z/length};
}
// [한국어 해설] 2차원에서도 같은 실패 계약을 사용한다. 호출자는 optional을 확인한 뒤 역참조한다.
inline std::optional<Vec2> Normalize(Vec2 v, double epsilon=1e-12) {
    if (!Finite(v)||!std::isfinite(epsilon)||epsilon<0) return std::nullopt;
    const double length=std::hypot(v.x,v.y);
    if (!std::isfinite(length)||length<=epsilon) return std::nullopt;
    return Vec2{v.x/length,v.y/length};
}
inline void Nonnegative(double n) {
    if (!std::isfinite(n)||n<0) throw std::invalid_argument("finite nonnegative value required");
}
// [한국어 해설] 속도×시간을 이동 거리로 바꾸되 남은 거리보다 크면 목표에 고정하여 지나침을 막는다.
inline Vec2 MoveTowards(Vec2 p, Vec2 target, double speed, double dt) {
    Nonnegative(speed); Nonnegative(dt);
    if (!Finite(p)||!Finite(target)) throw std::invalid_argument("invalid point");
    const Vec2 d=target-p;
    const double distance=std::hypot(d.x,d.y), step=speed*dt;
    if (!std::isfinite(distance)||!std::isfinite(step)) throw std::overflow_error("motion range");
    if (distance<=step||distance==0) return target;
    return p+d*(step/distance);
}
// [한국어 해설] 각도를 (-π, π] 범위로 정리하고 정반대 방향은 +π로 통일한다.
inline double WrapAngle(double radians) {
    if (!std::isfinite(radians)) throw std::invalid_argument("invalid angle");
    constexpr double pi=std::numbers::pi;
    double r=std::remainder(radians,2*pi);
    if (r<=-pi) r+=2*pi; // deterministic choice at the opposite direction
    return r;
}
// [한국어 해설] 최단 각도 차를 이번 프레임의 최대 회전량 안으로 제한한다.
inline double TurnTowards(double current,double target,double rate,double dt) {
    Nonnegative(rate); Nonnegative(dt);
    const double bound=rate*dt;
    if (!std::isfinite(bound)) throw std::overflow_error("angular step");
    const double delta=WrapAngle(WrapAngle(target)-WrapAngle(current));
    return WrapAngle(current+std::clamp(delta,-bound,bound));
}
// [한국어 해설] 등가속도 공식이다. 이 함수는 바닥 충돌이나 착지 상태 전이를 처리하지 않는다.
inline double JumpHeight(double initialY,double initialSpeed,double acceleration,double time) {
    Nonnegative(time);
    if (!std::isfinite(initialY)||!std::isfinite(initialSpeed)||!std::isfinite(acceleration))
        throw std::invalid_argument("invalid jump input");
    const double result=initialY+initialSpeed*time+0.5*acceleration*time*time;
    if (!std::isfinite(result)) throw std::overflow_error("jump range");
    return result;
}
// [한국어 해설] 동차좌표의 w=1은 점, w=0은 방향이다. 행 벡터 v*M 규약을 끝까지 유지한다.
using Vec4=std::array<double,4>;
struct Mat4 { std::array<std::array<double,4>,4> m{}; };
inline Mat4 Identity() {
    Mat4 a; for (std::size_t i=0;i<4;++i) a.m[i][i]=1; return a;
}
// [한국어 해설] 합성 순서는 교환할 수 없다. 행 벡터 기준 Scale*Rotation*Translation은 왼쪽 변환부터 적용한다.
inline Mat4 operator*(const Mat4& a,const Mat4& b) {
    Mat4 r;
    for (std::size_t i=0;i<4;++i)
        for (std::size_t j=0;j<4;++j)
            for (std::size_t k=0;k<4;++k) r.m[i][j]+=a.m[i][k]*b.m[k][j];
    return r;
}
// [한국어 해설] 4성분 원시 결과만 계산한다. 원근 나눗셈이 필요하면 호출자가 w를 검사한 뒤 별도로 한다.
inline Vec4 Transform(Vec4 v,const Mat4& m) {
    Vec4 r{};
    for (std::size_t j=0;j<4;++j)
        for (std::size_t i=0;i<4;++i) r[j]+=v[i]*m.m[i][j];
    return r; // row-vector v * M; no implicit perspective divide
}
inline Mat4 Scale(double x,double y,double z) {
    Mat4 a=Identity(); a.m[0][0]=x; a.m[1][1]=y; a.m[2][2]=z; return a;
}
// [한국어 해설] 행 벡터 규약이므로 이동 성분은 마지막 열이 아닌 마지막 행에 들어간다.
inline Mat4 Translation(double x,double y,double z) {
    Mat4 a=Identity(); a.m[3][0]=x; a.m[3][1]=y; a.m[3][2]=z; return a;
}
// [한국어 해설] 양의 각도에서 x축 방향이 y축 방향으로 회전하는 행 벡터 행렬이다.
inline Mat4 RotationZ(double angle) {
    Mat4 a=Identity(); const double c=std::cos(angle),s=std::sin(angle);
    a.m[0][0]=c; a.m[0][1]=s; a.m[1][0]=-s; a.m[1][1]=c; return a;
}
// [한국어 해설] 비균일 크기 변환의 법선에는 역행렬의 전치를 사용한다. 일반 방향 변환과 구별한다.
inline Mat4 Transpose(const Mat4& a) {
    Mat4 r; for (std::size_t i=0;i<4;++i) for (std::size_t j=0;j<4;++j) r.m[i][j]=a.m[j][i];
    return r;
}
// Partial pivoting, absolute pivot tolerance: suitable for these bounded examples,
// not a condition-number-aware general-purpose numerical library.
// [한국어 해설] 부분 피벗 가우스-조던 소거다. 절대 임계값 판정이며 범용 수치 안정성을 보장하지 않는다.
inline std::optional<Mat4> Inverse(Mat4 a,double tolerance=1e-12) {
    if (!std::isfinite(tolerance)||tolerance<=0) return std::nullopt;
    for (const auto& row:a.m) for (double v:row) if (!std::isfinite(v)) return std::nullopt;
    Mat4 out=Identity();
    for (std::size_t col=0;col<4;++col) {
// [한국어 해설] 현재 열에서 절댓값이 큰 피벗을 골라 작은 수로 나누는 위험을 줄인다.
        std::size_t pivot=col;
        for (std::size_t row=col+1;row<4;++row)
            if (std::abs(a.m[row][col])>std::abs(a.m[pivot][col])) pivot=row;
        if (std::abs(a.m[pivot][col])<=tolerance) return std::nullopt;
        std::swap(a.m[col],a.m[pivot]); std::swap(out.m[col],out.m[pivot]);
        const double divisor=a.m[col][col];
        for (std::size_t j=0;j<4;++j) { a.m[col][j]/=divisor; out.m[col][j]/=divisor; }
        for (std::size_t row=0;row<4;++row) if (row!=col) {
            const double factor=a.m[row][col];
            for (std::size_t j=0;j<4;++j) { a.m[row][j]-=factor*a.m[col][j]; out.m[row][j]-=factor*out.m[col][j]; }
        }
    }
    for (const auto& row:out.m) for (double v:row) if (!std::isfinite(v)) return std::nullopt;
    return out;
}
// [한국어 해설] 월드는 y 위쪽, 화면은 y 아래쪽이다. 확대율은 양수이며 역변환에서도 같은 규약을 쓴다.
struct Camera2D {
    Vec2 position{}, anchor{}; double zoom{1};
    void Validate() const {
        if (!Finite(position)||!Finite(anchor)||!std::isfinite(zoom)||zoom<=0)
            throw std::invalid_argument("invalid camera");
    }
    Vec2 ToScreen(Vec2 world) const {
        Validate(); if (!Finite(world)) throw std::invalid_argument("invalid world");
        return {anchor.x+(world.x-position.x)*zoom, anchor.y-(world.y-position.y)*zoom};
    }
    Vec2 ToWorld(Vec2 screen) const {
        Validate(); if (!Finite(screen)) throw std::invalid_argument("invalid screen");
        return {position.x+(screen.x-anchor.x)/zoom,position.y-(screen.y-anchor.y)/zoom};
    }
};
struct Aabb { Vec2 min{},max{}; };
inline bool Valid(Aabb b) { return Finite(b.min)&&Finite(b.max)&&b.min.x<=b.max.x&&b.min.y<=b.max.y; }
// [한국어 해설] AABB는 경계 접촉도 충돌이다. 최소/최대 좌표가 뒤집힌 상자는 예외로 거부한다.
inline bool Overlap(Aabb a,Aabb b) {
    if (!Valid(a)||!Valid(b)) throw std::invalid_argument("invalid box");
    return a.min.x<=b.max.x&&b.min.x<=a.max.x&&a.min.y<=b.max.y&&b.min.y<=a.max.y;
}
// [한국어 해설] 두 중심 거리와 반지름 합을 비교한다. 음수 반지름과 비유한 입력을 허용하지 않는다.
inline bool CircleOverlap(Vec2 a,double ar,Vec2 b,double br) {
    Nonnegative(ar); Nonnegative(br);
    if (!Finite(a)||!Finite(b)) throw std::invalid_argument("invalid center");
    const double radius=ar+br;
    if (!std::isfinite(radius)) throw std::overflow_error("radius range");
    return std::hypot(a.x-b.x,a.y-b.y)<=radius; // touching counts as contact
}
// [한국어 해설] 투영 계수를 [0,1]로 제한한다. 길이가 거의 0인 선분은 시작점을 반환한다.
inline Vec2 ClosestOnSegment(Vec2 p,Vec2 a,Vec2 b) {
    if (!Finite(p)||!Finite(a)||!Finite(b)) throw std::invalid_argument("invalid segment");
    const Vec2 ab=b-a; const double lengthSquared=Dot(ab,ab);
    if (lengthSquared<=1e-24) return a;
    const double t=std::clamp(Dot(p-a,ab)/lengthSquared,0.0,1.0);
    return a+ab*t;
}
// Isometric projection plane uses right/down axes; do not mix it with Camera2D's
// world-up coordinates without an explicit sign conversion.
// [한국어 해설] 마름모 투영은 화면형 y 아래쪽 규약이다. Camera2D의 월드 좌표와 그대로 섞지 않는다.
inline Vec2 IsoCenter(double u,double v,double halfWidth,double halfHeight) {
    if (!std::isfinite(u)||!std::isfinite(v)||!std::isfinite(halfWidth)||!std::isfinite(halfHeight)||halfWidth<=0||halfHeight<=0)
        throw std::invalid_argument("invalid isometric input");
    return {(u-v)*halfWidth,(u+v)*halfHeight};
}
// [한국어 해설] 역투영한 타일 좌표를 중심 기준 반열린 구간으로 분류한다. 음수는 정수 절삭 대신 floor가 필요하다.
inline std::optional<std::pair<int,int>> PickIso(Vec2 plane,int width,int height,double hw,double hh) {
    if (!Finite(plane)||width<=0||height<=0||!std::isfinite(hw)||!std::isfinite(hh)||hw<=0||hh<=0)
        throw std::invalid_argument("invalid picking input");
    const double u=(plane.x/hw+plane.y/hh)/2, v=(plane.y/hh-plane.x/hw)/2;
    // Half-open cells [center-.5, center+.5), including negative outside positions.
    const double col=std::floor(u+0.5), row=std::floor(v+0.5);
    if (!std::isfinite(col)||!std::isfinite(row)||col<0||row<0||col>=width||row>=height) return std::nullopt;
    return std::pair{static_cast<int>(col),static_cast<int>(row)};
}
} // namespace study
#endif
