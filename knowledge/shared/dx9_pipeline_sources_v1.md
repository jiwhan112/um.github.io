# DX9 렌더링 파이프라인 — 파일별 전체 소스 v1.0

각 제목을 해당 상대 경로의 파일로 저장하면 됩니다. 일반 문서용 코드 블록을 중첩하기 위해 파일별 바깥 구분은 물결표를 사용했습니다. 원시 로그는 ZIP에 있습니다.

## README.md

~~~~markdown
# DX9 렌더링 파이프라인 — CPU 학습 실습 v1.0

개정일: 2026-10-13. C++20. 외부 라이브러리 없이 CMake로 빌드하는 **수치·상태 학습 모델**입니다.
D3D9 런타임이나 GPU를 호출하는 렌더러가 아닙니다. 창이나 이미지를 출력하지 않고 결과를 콘솔과 조건 검사로 확인합니다.

## 읽는 순서

| 실행 파일 | 소스 | 관찰하는 것 | 검사 횟수 |
|---|---|---|---:|
| vertex_input | src/vertex_input.cpp | 구조체 크기, stride, 속성 offset, 인덱스·버퍼 경계 | 22 |
| transform_clip | src/transform_clip.cpp | 로컬→화면, near/far, 동차 6평면 클리핑 | 79 |
| raster_uv | src/raster_uv.cpp | Top-left 소속, 무게중심, UV 원근 보정, 깊이·텍셀 중심 | 71 |
| depth_blend | src/depth_blend.cpp | 깊이 비교/쓰기, 알파 거부, 혼합 순서, premultiplied 표현 | 19 |
| pipeline_trace | src/pipeline_trace.cpp | 8×8 타깃의 단일 삼각형 처리 흐름 | 105 |
| **합계** | | 동일 조건을 각 빌드 구성에서 반복 | **296** |

`include/pipeline.hpp`는 공통 계산, `include/check.hpp`는 테스트 보조입니다.
`examples/texture_sm3.hlsl`은 D3D9 셰이더 대응을 설명하는 **컴파일·GPU 실행 미검증 참고 소스**입니다.
`docs/validation.md`에 실제 검증과 제한을, `logs/`에 최초 제공 시 검증 원시 로그를 담았습니다.

## 일반 빌드

C++20을 지원하는 컴파일러와 CMake 3.20 이상이 필요합니다.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

상세 출력을 보려면 `ctest --test-dir build -C Debug -V`를 실행합니다.
Windows/MSVC에서도 사용할 수 있는 설정을 넣었지만 **이번 제공본에서 실제 Windows 빌드를 수행한 것은 아닙니다**.
회사의 C++11 프로젝트에 그대로 복사하지 마세요. std::span, std::lerp 등 C++20 기능을 사용합니다.

## 재검증 스크립트

Python 3.9 이상과 CMake가 있는 환경에서 다음과 같이 실행합니다.

```sh
python3 verify.py --compiler g++ --build-dir ../pipeline-check-gcc
python3 verify.py --compiler clang++ --build-dir ../pipeline-check-clang
python3 verify.py --compiler clang++ --sanitizers --build-dir ../pipeline-check-san
```

Windows Python 명령은 `python`일 수 있습니다. MSVC는 Developer Command Prompt에서 컴파일러 인자를 생략해 CMake 기본 생성기를 쓰세요.
Sanitizer 옵션은 이 프로젝트에서 GCC/Clang 계열용이며 MSVC용으로 구현하지 않았습니다.
스크립트는 한 번 실행하고 종료합니다. 기존 디렉터리를 삭제하지 않습니다. 다른 컴파일러로 바꾸려면 새로운 build-dir을 지정합니다.

## 계산 계약

행 벡터 `p' = pM`, 행별 4×4 double 배열, 뷰 공간 +Z 전방, 일반 깊이 near→0/far→1을 사용합니다.
원근 행렬은 `0<fov<pi`, `aspect>0`, `0<near<far`만 지원합니다. Reverse-Z·무한 far·역투영은 범위 밖입니다.
클리핑은 볼록 다각형에 대한 6개 동차 평면과 원래 UV의 선형 교점 생성입니다. 입력 정점 수는 최대64이며 경계 중복 정점 제거는 보장하지 않습니다.
Project는 **클리핑을 대신하지 않습니다**. 호출자는 클리핑 후 w>1e-12인 정점을 전달해야 합니다.

Covers는 방향을 맞춘 뒤 Top-left 규칙으로 정수 중심 단일 샘플을 판정합니다. 그 자체로 앞/뒷면을 제거하지 않습니다.
Barycentric/PerspectiveUV는 유효한 화면 삼각형과 그 샘플을 대상으로 합니다. 원래 UV 및 입력 데이터가 유한하고 해당 계약을 만족해야 합니다.
깊이는 이미 나눈 z/w의 화면 선형 보간입니다. UV용 원근 보정 식을 다시 적용하지 않습니다.
텍스처 실습은 1차원 level0 선형 필터와 clamp만 지원합니다. 모든 입력 텍셀은 유한한 값이어야 합니다.

출력 모델은 LESS_EQUAL, 선택적인 연속 double 알파 cutoff, ADD/source-over RGB, 별도 알파 ONE/INVSRCALPHA, RGBA 쓰기만 지원합니다.
목적지 색·깊이는 유효하게 초기화되어야 합니다. 스텐실, 다른 비교/블렌딩 연산, 쓰기 마스크, 포맷 양자화, sRGB, MSAA는 없습니다.
`depth_test=false`면 깊이 쓰기도 하지 않는 D3DRS_ZENABLE=FALSE 의미를 사용합니다.

수치 헬퍼 일부는 절댓값1e9 범위와 절대 임계값1e-12를 둡니다. 모든 크기의 입력에 대한 안정성·조건수 평가·생산용 수학 라이브러리를 대체하지 않습니다.
FetchOffset은 안전한 단일 스트림 바이트 산술 예제입니다. 드라이버의 전체 유효성 검사·선언 정합성을 구현하지 않으며 stride0의 API 특수 사용도 제외했습니다.

## 핵심 관찰

- `(1,1,0,1)` → W(1,0,0) → V(0,0,5) → P(90°,4/3,1,11) → clip(1.5,1,4.4,5) → NDC(0.3,0.2,0.88) → 화면(520,240),깊이0.88.
- 꼭짓점이 모두 화면 밖이어도 삼각형 내부가 클립 영역과 교차할 수 있습니다.
- lambda=(.5,.25,.25), w=(1,2,4), UV=((0,0),(1,0),(0,1))에서 단순 UV는(.25,.25), 원근 보정은(2/11,1/11)입니다.
- 알파0인 면도 깊이 쓰기가 켜져 있으면 뒤 물체를 가릴 수 있습니다.
- 삼각형 추적의 커버된 샘플은25개이며, 샘플(4,4)의 UV=(.5,.5),깊이=.4입니다.

## 이번에 수행하지 않은 것

실제 D3D9/Windows/MSVC/FXC/HLSL/GPU 실행, 성능 측정, 드라이버 적합성, 모든 GPU 정밀도·서브픽셀 규칙의 모사는 수행하지 않았습니다.
Sanitizer 통과가 전체 입력과 모든 환경에서 안전하다는 증명은 아닙니다.
수식 테스트는 의도한 계산을 확인합니다. 실제 GPU 캡처와 비교한 결과로 제시하지 않습니다.

## 학습 문서

[Notion 학습 목차](https://app.notion.com/p/3e66577f53a549fe99a94ac26f68a685)

~~~~

## CMakeLists.txt

~~~~text
cmake_minimum_required(VERSION 3.20)
project(dx9_pipeline_lab LANGUAGES CXX)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC AND ENABLE_SANITIZERS)
  message(FATAL_ERROR "This lab sanitizer configuration requires GCC or Clang, not MSVC.")
endif()
enable_testing()
foreach(name IN ITEMS vertex_input transform_clip raster_uv depth_blend pipeline_trace)
  add_executable(${name} src/${name}.cpp)
  target_include_directories(${name} PRIVATE include)
  if(MSVC)
    target_compile_options(${name} PRIVATE /W4 /permissive- /utf-8)
  else()
    target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(ENABLE_SANITIZERS)
      target_compile_options(${name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
      target_link_options(${name} PRIVATE -fsanitize=address,undefined)
    endif()
  endif()
  add_test(NAME ${name} COMMAND ${name})
endforeach()

~~~~

## verify.py

~~~~python
#!/usr/bin/env python3
"""Configure, build and run this CPU lab once, saving logs in the chosen build dir."""
from __future__ import annotations
import argparse
from pathlib import Path
import shutil
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', help='C++ compiler name or executable path')
    parser.add_argument('--build-dir', type=Path, default=Path('build-verify'))
    parser.add_argument('--config', default='Debug')
    parser.add_argument('--sanitizers', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    build = args.build_dir.resolve()
    if build == root or build in root.parents:
        parser.error('Use a dedicated build directory, not the source or its parent.')
    cmake = shutil.which('cmake')
    ctest = shutil.which('ctest')
    if not cmake or not ctest:
        parser.error('CMake and CTest must be installed and available on PATH.')
    configure = [cmake, '-S', str(root), '-B', str(build),
                 f'-DCMAKE_BUILD_TYPE={args.config}']
    if args.compiler:
        compiler = shutil.which(args.compiler)
        if not compiler:
            parser.error(f'Compiler not found: {args.compiler}')
        configure.append(f'-DCMAKE_CXX_COMPILER={compiler}')
    configure.append(f'-DENABLE_SANITIZERS={"ON" if args.sanitizers else "OFF"}')
    commands = [
        ('configure', configure),
        ('build', [cmake, '--build', str(build), '--config', args.config, '--parallel', '2']),
        ('test', [ctest, '--test-dir', str(build), '-C', args.config, '-V', '--output-on-failure']),
    ]
    build.mkdir(parents=True, exist_ok=True)
    for name, command in commands:
        print(f'[{name}] ' + ' '.join(command), flush=True)
        result = subprocess.run(command, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True,
                                encoding='utf-8', errors='replace', timeout=300)
        (build / f'verify-{name}.log').write_text(result.stdout, encoding='utf-8')
        print(result.stdout)
        if result.returncode:
            return result.returncode
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (OSError, subprocess.TimeoutExpired) as exc:
        print(f'Verification failed: {exc}', file=sys.stderr)
        raise SystemExit(1)

~~~~

## include/check.hpp

~~~~cpp
#ifndef STUDY_PIPELINE_CHECK_HPP
#define STUDY_PIPELINE_CHECK_HPP
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
struct Checks {
    int count{};
    void That(bool ok,const char* label) {
        ++count; if (!ok) throw std::runtime_error(std::string("FAIL: ")+label);
    }
    void Near(double value,double expected,const char* label) {
        That(std::isfinite(value) && std::abs(value-expected)<=1e-9*(1+std::abs(expected)),label);
    }
    template<class F> void Rejects(F&& f,const char* label) {
        bool rejected=false;
        try { std::forward<F>(f)(); } catch(const std::invalid_argument&) { rejected=true; }
        That(rejected,label);
    }
};
template<class F> int Run(const char* name,F f) {
    try { Checks c; f(c); std::cout<<"PASS "<<name<<" checks="<<c.count<<'\n'; return 0; }
    catch(const std::exception& e) { std::cerr<<name<<": "<<e.what()<<'\n'; return 1; }
}
#endif

~~~~

## include/pipeline.hpp

~~~~cpp
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

~~~~

## src/depth_blend.cpp

~~~~cpp
#include "pipeline.hpp"
#include "check.hpp"
int main(){return Run("depth_blend",[](Checks& c){
    using namespace pipeline;
    const Color red{1,0,0,1},blue{0,0,1,1},half_red{1,0,0,.5},half_blue{0,0,1,.5};
    Pixel p{blue,1};c.That(Fragment(p,red,.4,{}),"near fragment accepted");
    c.That(!Fragment(p,blue,.8,{}),"occluded fragment rejected");c.Near(p.depth,.4,"depth stored");
    Pixel a{{0,0,0,1},1};OutputState translucent{true,false,true,false,-1};
    Fragment(a,half_blue,.8,translucent);Fragment(a,half_red,.2,translucent);
    c.Near(a.color.r,.5,"back to front red");c.Near(a.color.b,.25,"back to front blue");c.Near(a.depth,1,"transparent no z writes");
    Pixel b{{0,0,0,1},1};Fragment(b,half_red,.2,translucent);Fragment(b,half_blue,.8,translucent);
    c.Near(b.color.r,.25,"reversed red differs");c.Near(b.color.b,.5,"reversed blue differs");
    Pixel hole{blue,1};auto bad=translucent;bad.depth_write=true;
    Fragment(hole,{1,0,0,0},.2,bad);
    c.Near(hole.color.b,1,"zero alpha preserves color");c.Near(hole.depth,.2,"zero alpha can still write z");
    c.That(!Fragment(hole,red,.5,{}),"invisible foreground blocks later opaque");
    Pixel cut{blue,1};bad.alpha_cutoff=.5;
    c.That(!Fragment(cut,{1,0,0,0},.2,bad),"alpha test rejects");c.Near(cut.depth,1,"cutout did not write z");
    Pixel s{blue,1},m{blue,1};Fragment(s,half_red,.2,translucent);
    auto pm=translucent;pm.premultiplied=true;Fragment(m,{.5,0,0,.5},.2,pm);
    c.Near(s.color.r,m.color.r,"straight and premultiplied match with matching factors");
    c.Near(s.color.b,m.color.b,"destination factor matches");
    c.Near(s.color.a,1,"explicit separate alpha source-over");
    Pixel no_z{blue,.1};OutputState disabled{false,true,false,false,-1};
    c.That(Fragment(no_z,red,.8,disabled),"depth disabled bypasses test");
    c.Near(no_z.depth,.1,"depth disabled also suppresses writes");
    c.Rejects([&]{Fragment(no_z,red,-.1,{});},"invalid depth rejected");
    std::cout<<"transparent back-to-front RGB=(0.5,0,0.25), reverse=(0.25,0,0.5)\n";
});}

~~~~

## src/pipeline_trace.cpp

~~~~cpp
#include "pipeline.hpp"
#include "check.hpp"

int main() {
    return Run("pipeline_trace", [](Checks& c) {
        using namespace pipeline;
        // This triangle is entirely inside the six clip planes. Clipping that
        // creates a polygon is tested separately in transform_clip.cpp.
        const std::array<Vertex, 3> input{{
            {{-0.8, 0.8, 0.4, 1}, {0, 0}},
            {{0.8, 0.8, 0.4, 1}, {1, 0}},
            {{0, -0.8, 0.4, 1}, {0.5, 1}}
        }};
        const auto polygon = Clip(input);
        c.That(polygon.size() == 3, "triangle survives clipping");

        std::array<ScreenVertex, 3> screen{};
        for (std::size_t i = 0; i < 3; ++i) {
            screen[i] = Project(polygon[i], {0, 0, 8, 8, 0, 1});
        }
        const std::array<Vec2, 3> xy{screen[0].p, screen[1].p, screen[2].p};
        int covered = 0;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                const Vec2 point{static_cast<double>(x), static_cast<double>(y)};
                if (!Covers(xy, point)) {
                    continue;
                }
                ++covered;
                const auto weights = Barycentric(xy, point);
                c.That(weights.has_value(), "sample barycentric");
                const auto uv = PerspectiveUV(screen, *weights);
                const double depth = AffineDepth(screen, *weights);

                // An independent target sample, initially black with depth 1.
                Pixel output{{0, 0, 0, 1}, 1};
                c.That(Fragment(output, {uv.x, uv.y, 0, 1}, depth, {}),
                       "fragment passes output stage");
                c.That(uv.x >= -1e-12 && uv.x <= 1 + 1e-12 &&
                       uv.y >= -1e-12 && uv.y <= 1 + 1e-12,
                       "interpolated uv in range");
                c.Near(output.depth, 0.4, "constant depth across triangle");
            }
        }
        c.That(covered == 25, "25 covered samples");
        const auto center_weights = Barycentric(xy, {4, 4});
        if (!center_weights) {
            throw std::runtime_error("center has no barycentric coordinates");
        }
        const auto center_uv = PerspectiveUV(screen, *center_weights);
        c.Near(center_uv.x, 0.5, "center u");
        c.Near(center_uv.y, 0.5, "center v");

        auto reversed = xy;
        std::swap(reversed[1], reversed[2]);
        c.That(Edge(xy[0], xy[1], xy[2]) ==
                   -Edge(reversed[0], reversed[1], reversed[2]),
               "culling diagnostic");
        std::cout << "8x8 viewport: covered samples=" << covered
                  << ", sample (4,4): uv=(0.5,0.5), z=0.4\n";
    });
}

~~~~

## src/raster_uv.cpp

~~~~cpp
#include "pipeline.hpp"
#include "check.hpp"
int main(){return Run("raster_uv",[](Checks& c){
    using namespace pipeline;
    const std::array<Vec2,3> a{{{0,0},{4,0},{4,4}}},b{{{0,0},{4,4},{0,4}}};
    int total=0;
    for(int y=-1;y<=5;++y)for(int x=-1;x<=5;++x){
        const Vec2 sample{static_cast<double>(x),static_cast<double>(y)};
        const int covered=static_cast<int>(Covers(a,sample))+static_cast<int>(Covers(b,sample));
        const int wanted=(x>=0 && x<4 && y>=0 && y<4)?1:0;
        c.That(covered==wanted,"shared edge has exactly one owner");total+=covered;
    }
    c.That(total==16,"quad covers 16 integer pixel centers");
    auto reversed=a;std::swap(reversed[1],reversed[2]);
    c.That(Covers(reversed,{2,1})==Covers(a,{2,1}),"coverage invariant without culling");
    c.That(Edge(a[0],a[1],a[2])>0 && Edge(reversed[0],reversed[1],reversed[2])<0,"winding changes sign");
    std::array<ScreenVertex,3> t{{{{0,0},.2,1,{0,0}},{{4,0},.6,.5,{1,0}},{{0,4},.9,.25,{0,1}}}};
    auto l=Barycentric({t[0].p,t[1].p,t[2].p},{1,1});c.That(l.has_value(),"nondegenerate triangle");
    c.Near((*l)[0],.5,"weight0");c.Near((*l)[1],.25,"weight1");c.Near((*l)[2],.25,"weight2");
    auto uv=PerspectiveUV(t,*l);c.Near(uv.x,2.0/11,"perspective u");c.Near(uv.y,1.0/11,"perspective v");
    c.Near(AffineDepth(t,*l),.475,"depth uses screen-linear z/w");
    c.That(!Barycentric({Vec2{0,0},Vec2{1,1},Vec2{2,2}},{1,1}),"degenerate rejected");
    const std::array<double,4> texels{0,1,0,1};
    for(int i=0;i<4;++i)c.Near(LinearClamp(texels,(i+.5)/4),texels[static_cast<std::size_t>(i)],"texel center");
    c.Near(LinearClamp(texels,.25),.5,"uncorrected pixel mixes texels");
    c.Near(LinearClamp(texels,(1+.5)/4),1,"minus half-pixel vertex alignment");
    c.Near(LinearClamp(texels,-1),0,"left clamp");c.Near(LinearClamp(texels,2),1,"right clamp");
    c.Rejects([]{LinearClamp({},0);},"empty texture");
    const double nan=std::numeric_limits<double>::quiet_NaN();
    c.That(!Covers(a,{nan,0}),"nonfinite sample is not covered");
    c.That(!Barycentric(a,{0,nan}),"nonfinite barycentric input rejected");
    std::cout<<"weights=(0.5,0.25,0.25) affine_uv=(0.25,0.25) perspective_uv=(2/11,1/11) depth=0.475\n";
});}

~~~~

## src/transform_clip.cpp

~~~~cpp
#include "pipeline.hpp"
#include "check.hpp"
int main(){return Run("transform_clip",[](Checks& c){
    using namespace pipeline;
    constexpr double pi=3.14159265358979323846;
    auto p=PerspectiveLH(pi/2,4.0/3.0,1,11);
    auto w=Translation(1,0,0), v=Translation(0,0,5);
    auto clip=Transform({1,1,0,1},Multiply(Multiply(w,v),p));
    const Vec4 expected{1.5,1,4.4,5};
    for(std::size_t k=0;k<4;++k)c.Near(clip[k],expected[k],"clip result");
    c.That(Inside(clip),"inside frustum");
    auto screen=Project({clip,{}},{0,0,800,600,0,1});
    c.Near(screen.p.x,520,"screen x"); c.Near(screen.p.y,240,"screen y");
    c.Near(screen.depth,.88,"screen depth");
    for (double z:{1.0,2.0,6.0,11.0}) {
        auto q=Transform({0,0,z,1},p);
        c.Near(q[3],z,"clip w equals view z in this projection");
        c.Near(q[2]/q[3],1.1-1.1/z,"depth mapping");
    }
    auto near=Project({Transform({0,0,1,1},p),{}},{100,50,400,300,.2,.8});
    c.Near(near.p.x,300,"viewport x offset"); c.Near(near.p.y,200,"viewport y offset");
    c.Near(near.depth,.2,"min z remap");
    c.Rejects([]{PerspectiveLH(1,1,0,10);},"near zero");
    c.Rejects([]{PerspectiveLH(1,1,5,5);},"near equals far");
    c.Rejects([]{PerspectiveLH(1,0,1,10);},"aspect zero");
    c.Rejects([]{Project({{0,0,0,0},{}},{0,0,800,600,0,1});},"zero w");
    c.Rejects([]{Project({{0,0,0,-1},{}},{0,0,800,600,0,1});},"negative w");
    c.Rejects([]{Project({{0,0,0,1},{}},{0,0,0,600,0,1});},"zero viewport width");
    const std::array<Vertex,3> crossing{{{{-.5,-.5,-.5,1},{0,0}},{{.5,-.5,.5,1},{1,0}},{{0,.5,.5,1},{.5,1}}}};
    const auto polygon=Clip(crossing); c.That(polygon.size()==4,"near clip creates quad");
    int on_plane=0;
    for(auto q:polygon){c.That(Inside(q.clip,1e-12),"clipped vertex inside");if(std::abs(q.clip[2])<1e-12)++on_plane;}
    c.That(on_plane==2,"two new boundary vertices");
    c.Near(polygon[0].uv.x,.25,"new vertex raw attribute interpolation");
    std::array<Vertex,3> outside=crossing;
    for(auto& q:outside)q.clip[2]=-1;
    c.That(Clip(outside).empty(),"fully outside rejected");
    for(std::size_t plane=0;plane<6;++plane){
        auto q=Vec4{0,0,.5,1};
        if(plane<2)q[0]=plane==0?-2:2;
        else if(plane<4)q[1]=plane==2?-2:2;
        else q[2]=plane==4?-1:2;
        c.That(!Inside(q),"each clip plane rejects outside");
    }
    const std::array<Vertex,3> surround{{{{-3,-3,.5,1},{}},{{3,-3,.5,1},{}},{{0,3,.5,1},{}}}};
    for(auto q:surround)c.That(!Inside(q.clip),"source vertex is outside");
    auto enclosing=Clip(surround);
    c.That(enclosing.size()>=3,"all-outside vertices do not imply invisible triangle");
    for(auto q:enclosing)c.That(Inside(q.clip,1e-12),"frustum intersection survives");
    for(std::size_t plane=0;plane<6;++plane){
        std::array<Vertex,3> crossing_plane{{{{-.25,-.25,.5,1},{0,0}},{{.25,-.25,.5,1},{1,0}},{{0,.25,.5,1},{.5,1}}}};
        auto& q=crossing_plane[0].clip;
        if(plane<2)q[0]=plane==0?-2:2;
        else if(plane<4)q[1]=plane==2?-2:2;
        else q[2]=plane==4?-1:2;
        auto result=Clip(crossing_plane);
        c.That(result.size()>=3,"partial intersection retained for every plane");
        for(auto r:result)c.That(Inside(r.clip,1e-12),"all generated vertices obey planes");
    }
    std::cout<<"local=(1,1,0,1) world=(2,1,0,1) view=(2,1,5,1)\n"
             <<"clip=(1.5,1,4.4,5) ndc=(0.3,0.2,0.88) screen=(520,240,0.88)\n";
});}

~~~~

## src/vertex_input.cpp

~~~~cpp
#include "pipeline.hpp"
#include "check.hpp"
#include <type_traits>
struct VertexPT { float x,y,z,u,v; };
static_assert(std::is_standard_layout_v<VertexPT> && std::is_trivially_copyable_v<VertexPT>);
static_assert(sizeof(float)==4 && sizeof(VertexPT)==20 && offsetof(VertexPT,u)==12);
int main() { return Run("vertex_input",[](Checks& c) {
    using namespace pipeline;
    const std::array<VertexPT,4> vertices{{{-1,1,2,0,0},{1,1,2,1,0},{1,-1,2,1,1},{-1,-1,2,0,1}}};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto bytes=std::as_bytes(std::span(vertices));
    for(auto i:indices) {
        const auto off=FetchOffset(bytes.size(),0,sizeof(VertexPT),0,i,12,8);
        std::array<float,2> uv{}; std::memcpy(uv.data(),bytes.data()+off,sizeof(uv));
        c.Near(uv[0],vertices[i].u,"u matches declaration");
        c.Near(uv[1],vertices[i].v,"v matches declaration");
    }
    c.That(indices.size()/3==2,"two triangle-list primitives");
    c.That(FetchOffset(120,0,20,2,3,12,8)==112,"base index and byte offset differ");
    c.That(FetchOffset(100,20,20,0,3,12,8)==92,"stream offset in bytes");
    c.Rejects([]{FetchOffset(80,0,20,0,4,0,12);},"index beyond buffer");
    c.Rejects([]{FetchOffset(80,0,20,-1,0,0,12);},"negative effective index");
    c.Rejects([]{FetchOffset(80,0,16,0,0,12,8);},"stride too short");
    c.Rejects([]{FetchOffset(80,81,20,0,0,0,4);},"stream offset beyond buffer");
    c.Rejects([]{FetchOffset(80,0,0,0,0,0,4);},"zero stride unsupported by this model");
    c.Rejects([]{FetchOffset(80,0,20,0,0,19,4);},"element crosses stride");
    c.Rejects([]{FetchOffset(80,0,20,0,0xffffffffu,0,4);},"huge index cannot wrap");
    std::cout<<"quad: 4 vertices / 6 indices / 2 triangles / stride=20\n";
}); }

~~~~

## examples/texture_sm3.hlsl

~~~~hlsl
// Reference only: NOT compiled with FXC and NOT executed on a GPU in this revision.
// D3D9: compile VSMain as vs_3_0 and PSMain as ps_3_0 in your Windows toolchain.
// Host contract: POSITION0 float3, TEXCOORD0 float2; four float4 ROWS at c0;
// a 2D texture and sampler states at s0; valid target/depth/output state.
// The CPU lab uses double arrays: do NOT upload their bytes as float registers.
row_major float4x4 gWorldViewProjection : register(c0);
sampler2D gAlbedo : register(s0);

struct VSInput {
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
};
struct VSOutput {
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    // Keep homogeneous w for hardware clipping and perspective interpolation.
    output.position = mul(float4(input.position, 1.0), gWorldViewProjection);
    output.uv = input.uv;
    return output;
}

float4 PSMain(float2 uv : TEXCOORD0) : COLOR0 {
    return tex2D(gAlbedo, uv);
}

~~~~

## docs/validation.md

~~~~markdown
# DX9 렌더링 파이프라인 실습 검증 기록 v1.0

개정일: 2026-10-13. 대상: CPU 수치·상태 모델. 실제 Direct3D 런타임/GPU 검증이 아닙니다.

## 실행 환경
Linux x86-64; GCC 14.2.0 (Debian 14.2.0-19); Clang 17.0.0; CMake 3.31.6; C++20.

| 프로그램 | 조건 검사 | 확인 내용 |
|---|---:|---|
| vertex_input | 22 | stride, attribute offset, byte/index units, range rejection |
| transform_clip | 79 | W/V/P, homogeneous clip planes, viewport, invalid input |
| raster_uv | 71 | top-left coverage, barycentrics, perspective UV, depth, texel centers |
| depth_blend | 19 | depth test/write, alpha rejection, alpha-zero occlusion, blending order |
| pipeline_trace | 105 | 8x8 single-sample triangle trace, 25 covered samples |
| 합계 | 296 | 반복 샘플 조건 검사 포함 |

GCC, Clang, Clang+ASan/UBSan에서 각각 5/5 프로그램 및 296회 조건 검사가 통과했습니다.
동일 검사를 반복했으므로 서로 다른 조건 888개로 합산하지 않습니다.
컴파일 거부 예제는 없으며, 잘못된 입력의 함수 거부를 실행 중 검사했습니다.
최종 소스에 대해 verify.py의 구성·빌드·CTest 동작도 세 구성에서 실제 실행했습니다.

## 관찰 결과
- 로컬(1,1,0,1), 월드 이동(1,0,0), 뷰 이동(0,0,5), FOV90도/aspect4:3/near1/far11에서 clip=(1.5,1,4.4,5), NDC=(.3,.2,.88), 800x600 화면=(520,240),깊이=.88.
- near 교차 삼각형을 4정점 다각형으로 잘랐고, 새 교점의 UV도 검사했습니다.
- 세 꼭짓점이 모두 바깥에 있어도 삼각형이 클립 영역과 교차하는 사례를 검사했습니다.
- UV의 단순 보간(.25,.25)과 원근 보정(2/11,1/11)을 비교했습니다.
- 정수 샘플 중심을 이용한 두 삼각형의 공유 모서리 소속을 확인했습니다.
- 텍셀(0,1,0,1)에서 u=.25의 값=.5, u=.375의 값=1을 확인했습니다.
- 알파0도 깊이 쓰기가 켜지면 뒤 물체를 가릴 수 있음을 확인했습니다.
- 투명 파랑→빨강 RGB=(.5,0,.25), 반대 순서=(.25,0,.5).
- 통합 추적은 25개 샘플을 포함하며, 샘플(4,4)의 UV=(.5,.5),깊이=.4.

## 빌드 옵션과 로그
GCC/Clang: -Wall -Wextra -Wpedantic -Wconversion -Wshadow, C++20, 확장 비활성.
Sanitizer: -fsanitize=address,undefined -fno-omit-frame-pointer.
logs/의 gcc_*, clang_*, sanitizer_* 파일은 각 구성의 최종 configure/build/test 출력입니다.
전체 소스와 재검증 명령은 README 및 verify.py에 있습니다.

## 해석 제한
이 결과는 실제 화면 캡처나 GPU와의 비트 단위 대조가 아닙니다.
Windows/MSVC, FXC/HLSL, D3D9 자원·드로우·Present, GPU 실행과 성능은 미검증입니다.
examples/texture_sm3.hlsl은 CMake 기본 대상에서 제외한 미검증 참고 소스입니다.
스텐실·sRGB·MSAA·실제 2D 텍스처 필터·하드웨어 포맷 정밀도는 구현하지 않았습니다.
CPU 모델의 수치 범위·입력 계약과 1차원 샘플러 제한은 README에 명시했습니다.
검사기에서 진단이 없다는 사실만으로 모든 입력·환경의 안전성이 증명되지는 않습니다.

## 관련 문서
- [DX9 렌더링 파이프라인](https://app.notion.com/p/3e66577f53a549fe99a94ac26f68a685)
- [실습·진단 본문](https://app.notion.com/p/3f8a99703f408105948ddf79d5139bb1)

~~~~
