# 게임 수학·WinAPI 실습 전체 소스 v1.0

2026-10-13. 플랫폼 독립 예제만 실행 검증. Windows 소스는 컴파일·실행 미검증.

## CMakeLists.txt

````text
cmake_minimum_required(VERSION 3.20)
project(GameMathWinApiLab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(ENABLE_SANITIZERS "Enable ASan and UBSan for portable tests" OFF)
add_library(study_options INTERFACE)
target_include_directories(study_options INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(MSVC)
  target_compile_options(study_options INTERFACE /W4 /permissive- /utf-8)
else()
  target_compile_options(study_options INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  if(ENABLE_SANITIZERS)
    target_compile_options(study_options INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(study_options INTERFACE -fsanitize=address,undefined)
  endif()
endif()
find_package(Threads REQUIRED)
enable_testing()
foreach(name IN ITEMS vector_motion transforms geometry loop_input tile_document pathfinding)
  add_executable(${name} src/${name}.cpp)
  target_link_libraries(${name} PRIVATE study_options Threads::Threads)
  add_test(NAME ${name} COMMAND ${name})
endforeach()
if(WIN32)
  enable_language(RC)
  add_executable(winapi_demo WIN32 windows/winapi_demo.cpp windows/app.rc)
  target_include_directories(winapi_demo PRIVATE windows)
  target_compile_definitions(winapi_demo PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
  target_link_libraries(winapi_demo PRIVATE study_options user32 gdi32)
  if(MSVC)
    target_link_options(winapi_demo PRIVATE /MANIFEST:NO)
  elseif(MINGW)
    target_link_options(winapi_demo PRIVATE -municode)
  endif()
endif()

````

## README.md

````text
# 게임 수학 · MFC · WinAPI 실습 팩 v1.0

개정일: 2026-10-13. C++20 학습 기준. 회사 프로젝트나 외부 SDK의 원본 코드가 아닌 독립 학습 예제입니다.

## 실행한 범위와 실행하지 않은 범위

Linux x86-64 / GCC 14.2.0 / Clang 17.0.0 / CMake 3.31.6에서 플랫폼 독립 프로그램 6개를 빌드하고 실행했습니다. Clang ASan·UBSan 구성도 통과했습니다. `logs/`에는 실제 명령의 구성·빌드·CTest 로그가 있습니다.

**Windows/MSVC와 windows/winapi_demo.cpp는 컴파일하거나 실행하지 않았습니다.** Windows SDK, MFC, GPU, DirectX, FMOD 실행 환경이 이 검증 환경에 없습니다. GUI 소스는 공식 API 계약을 대조한 정적 검토 결과입니다. 실행 확인 목록은 docs/validation.md에 있습니다. MFC GUI 프로젝트, D3DX 또는 FMOD 바이너리는 포함하지 않습니다.

## 폴더

- include/math.hpp: 벡터, 스칼라 행렬, 기하, 카메라, 타일 좌표.
- include/loop.hpp: 입력 에지, 고정 시간 간격, 애니메이션.
- include/grid.hpp: 맵 파싱, 편집/Undo/Redo, A*.
- include/check.hpp: 정상 조건 검사 도우미.
- src/: 독립 실행 가능한 프로그램 6개.
- windows/: 별도의 Win32 GUI 예제와 DPI manifest.
- docs/validation.md: 실제 결과와 미검증 항목.
- logs/: 이번 실제 실행 로그.
- verify.py: 임시 빌드 디렉터리에서 같은 테스트를 재실행하는 도구.

## 빌드

압축을 푼 이 폴더에서:

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Clang 검사기 사용:

```shell
cmake -S . -B build-san -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build-san --config Debug
ctest --test-dir build-san -C Debug --output-on-failure
```

선택 사항: Python 3으로 별도의 임시 빌드/로그 만들기.

```shell
python verify.py --compiler g++
python verify.py --compiler clang++ --sanitizers
```

Windows에서는 MSVC C++ 도구와 Windows SDK를 준비하고 x64 개발자 터미널 또는 Visual Studio CMake 기능을 사용합니다. `cmake -S . -B build`와 `cmake --build build --config Debug`로 구성합니다. 자동 선택된 생성기에 따라 실행 파일은 `build/Debug/winapi_demo.exe` 또는 `build/winapi_demo.exe`에 있습니다. 다른 생성기로 이미 구성한 build 폴더는 재사용하지 마세요.

Win32 예제는 Windows 10 version 1703 이상을 대상으로 작성했습니다. PMv2 manifest를 app.rc에 포함하고 MSVC의 자동 manifest 생성을 `/MANIFEST:NO`로 끕니다. 폰트의 정교한 DPI별 재구성은 구현하지 않았습니다.

## GUI 조작과 설계

방향키: 이동. 클릭: 목표를 향해 이동. Space: 위치 초기화. Esc: 종료.
120Hz는 고정 Update 간격이지 화면 주사율 보장이 아닙니다. 한 루프에서 최대 8회, 프레임 경과 입력 최대 0.25초로 제한합니다. 밀린 시간을 일부 버리는 로컬 데모 정책이며 네트워크 결정적 시뮬레이션을 구현한 것이 아닙니다. 최소화·비활성·크기 조정 중에는 정지하고 복귀 시 시간 기준을 재설정합니다.
App 좌표는 x 오른쪽/y 아래쪽의 논리 단위이며 DPI/96으로 출력합니다. math.hpp의 Camera2D는 별도의 x 오른쪽/y 위쪽 월드를 화면으로 변환합니다. 두 규약을 무심코 혼용하지 마세요.

## 수학 및 데이터 계약

행 벡터 p*M, 행별 저장의 4x4 행렬을 사용합니다. Transform은 w를 포함한 4성분 원시 결과이며 자동 원근 나눗셈을 하지 않습니다. 행렬/벡터 기본 산술은 중간값이 유한 범위에 들어가는 보통 크기의 값에 대한 학습용이며 모든 함수가 모든 입력의 범위를 검사하지는 않습니다. 정규화 등 검사 함수도 전체 IEEE754 입력 공간에서 수치 안정성을 증명하는 것은 아닙니다.

역행렬은 부분 피벗 Gauss–Jordan과 절대 임계값 1e-12를 사용합니다. 조건수·스케일 적응형 판정을 포함한 범용 수치 라이브러리가 아닙니다. AABB/원은 접촉도 충돌로 취급합니다. 마름모 타일 소속은 중심 주변 반열린 구간입니다. 카메라/타일 투영의 y축 규약은 헤더 주석과 문서를 함께 읽습니다.

맵 포맷: `MAP1 width height` 뒤 정확히 width*height개의 정수. 폭/높이 1..256, ID 0..4095, 텍스트 4MiB 이하. 행 구분은 저장 출력 형식이며 파서는 공백 구분 토큰을 읽습니다. 임시 모델에 완전히 읽고 검증한 뒤 교체합니다. 파일 시스템 원자적 저장이나 전원 장애 복구까지 제공하지 않습니다.

A*: 0은 통과 가능, 다른 ID는 막힘. 네 방향 이동 비용1, Manhattan 휴리스틱. 반환 경로는 시작과 목표를 포함합니다. 가중치·대각선·움직이는 장애물은 다루지 않습니다.

## 학습 노트

- 게임 수학·MFC: https://app.notion.com/p/3bde90ebffa44ef0a6455ca8e68a7886
- WinAPI 게임 프로그래밍: https://app.notion.com/p/fffca9a4ce864ac7bade7efe90186be7

## 참고 자료

계산 구현과 테스트는 이 실습용으로 작성했습니다. 아래 자료는 API 계약과 알고리즘 설명을 대조한 근거입니다.

- DirectXMath 규약: https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-getting-started
- Win32 창: https://learn.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
- QPC: https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
- GDI bitmap: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatiblebitmap
- DPI: https://learn.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged
- MFC: https://learn.microsoft.com/en-us/cpp/mfc/document-view-architecture
- A*: https://inst.eecs.berkeley.edu/~cs188/textbook/search/informed.html
- 시간 단계: https://gafferongames.com/post/fix_your_timestep/

````

## docs/validation.md

````text
# 게임 수학·WinAPI v1.0 — 실제 검증 기록

날짜: 2026-10-13. 환경: Linux x86-64. GCC 14.2.0, Clang 17.0.0, CMake 3.31.6. C++20 / Debug. 경고 옵션: -Wall -Wextra -Wpedantic -Wconversion -Wshadow.

| 프로그램 | 조건 검사 | 확인 대상 |
|---|---:|---|
| vector_motion | 19 | 내외적, 정규화 실패, 이동, 각도, 점프 |
| transforms | 29 | SRT, 역행렬 왕복/실패, 법선, 카메라 |
| geometry | 78 | 충돌 접촉, 선분, 8x8 타일 중심과 경계 |
| loop_input | 484 | 고정 시간 단계, 입력 에지, 애니메이션 |
| tile_document | 26 | 편집/Undo/Redo, 파싱 실패, 작업 스냅샷 |
| pathfinding | 93,561 | A*와 독립 BFS 비교, 반환 경로의 유효성 |
| 합계 | 94,197 | 반복 원소 검사 포함 |

GCC, Clang, Clang ASan·UBSan 각각 6/6 프로그램 통과. 정상 빌드 로그에는 컴파일러 경고가 없었습니다. 검사기 실행에 ASAN_OPTIONS=detect_leaks=1, UBSAN_OPTIONS=halt_on_error=1을 적용했습니다. 실행 중 해당 검사기 진단을 관찰하지 않았습니다.

A* 비교 범위: 3x3 격자의 장애물 배치 512개 × 시작9 × 도착9 = 41,472개 시나리오. BFS와 도달 가능성/최단 길이를 비교하며 경로 끝점, 통과 가능 여부, 연속 칸의 인접성도 검사했습니다. 이 검사를 대각선/가중치/이동 장애물/모든 크기의 지도에 대한 증명으로 확대하지 않습니다.

math.hpp, loop.hpp, grid.hpp를 각각 단독 include한 번역 단위를 GCC와 Clang에서 -Werror -fsyntax-only로 확인했습니다.

## 검증하지 않은 범위

Windows/MSVC에서의 컴파일, Win32 GUI, MFC DDX/스플리터/대화상자, DPI 모니터 이동, GDI 객체 수 안정성, GPU/DirectX/D3DX/FMOD 실행, ThreadSanitizer 경쟁 검사, 성능 벤치마크는 수행하지 않았습니다. Win32 소스는 정적 검토만 수행했습니다.

## Windows 수동 확인 목록 (아직 실행하지 않음)

1. 창 생성과 Esc/닫기 버튼 종료. 예외 메시지나 남는 프로세스 여부.
2. 대각선과 수평 이동 거리 비교, 클릭 목표 도착 시 지나침 여부.
3. 키를 누른 채 Alt+Tab한 뒤 복귀해 입력이 남지 않는지.
4. 최소화 복구와 긴 리사이즈 이후 상태가 급격히 진행하지 않는지.
5. 100/150/200% DPI, 모니터 간 이동 시 클릭과 도형 위치의 일치.
6. 여러 차례 리사이즈 후 GDI 객체 수가 계속 증가하지 않는지.
7. 버퍼 할당/복사 실패, 매우 작거나 큰 클라이언트 영역 처리.
8. 창 파괴 이후 App에 접근하는 콜백이 없는지.

## 결과의 해석

동일 조건을 세 구성에서 반복한 것이며 이를 독립된 조건 282,591개로 표시하지 않습니다. ASan·UBSan 통과는 실행된 경로에 대한 관찰이고 모든 메모리 오류나 수치 오류가 없다는 증명이 아닙니다. 실습의 파일 포맷 검증은 문법/범위의 검증이며 원자적 디스크 저장은 구현하지 않았습니다.

````

## include/check.hpp

````cpp
#ifndef STUDY_CHECK_HPP
#define STUDY_CHECK_HPP
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
inline int checks=0;
inline void Check(bool condition,const char* label) {
    ++checks; if (!condition) throw std::runtime_error(label);
}
inline bool Near(double a,double b,double eps=1e-9) {
    return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=eps*std::max({1.0,std::abs(a),std::abs(b)});
}
template<class F> void Throws(F&& f) {
    bool threw=false; try { f(); } catch(const std::exception&) { threw=true; }
    Check(threw,"expected exception");
}
inline int Done(const char* name) { std::cout<<name<<": "<<checks<<" checks passed\n"; return 0; }
#endif

````

## include/grid.hpp

````cpp
#ifndef STUDY_GRID_HPP
#define STUDY_GRID_HPP
#include <functional>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>
namespace study {
struct Cell { int x{},y{}; bool operator==(const Cell&) const=default; };
class Grid {
    int width_,height_;
    std::vector<int> tiles_;
    static std::size_t Count(int w,int h) {
        if (w<=0||h<=0||w>256||h>256) throw std::invalid_argument("dimensions must be 1..256");
        return static_cast<std::size_t>(w)*static_cast<std::size_t>(h);
    }
public:
    Grid(int w,int h):width_(w),height_(h),tiles_(Count(w,h)) {}
    int Width() const noexcept { return width_; }
    int Height() const noexcept { return height_; }
    bool Contains(Cell p) const noexcept { return p.x>=0&&p.y>=0&&p.x<width_&&p.y<height_; }
    std::size_t Index(Cell p) const {
        if (!Contains(p)) throw std::out_of_range("cell");
        return static_cast<std::size_t>(p.y)*static_cast<std::size_t>(width_)+static_cast<std::size_t>(p.x);
    }
    int Tile(Cell p) const { return tiles_[Index(p)]; }
    void Set(Cell p,int tile) {
        if (tile<0||tile>4095) throw std::invalid_argument("tile id must be 0..4095");
        tiles_[Index(p)]=tile;
    }
    // For pathfinding only: 0 = walkable; all other tile IDs are blocked.
    bool Walkable(Cell p) const { return Contains(p)&&Tile(p)==0; }
    std::string Serialize() const {
        std::ostringstream out; out<<"MAP1 "<<width_<<' '<<height_<<'\n';
        for (int y=0;y<height_;++y) {
            for (int x=0;x<width_;++x) out<<Tile({x,y})<<(x+1==width_?'\n':' ');
        }
        if (!out) throw std::runtime_error("serialization failed");
        return out.str();
    }
    static Grid Parse(std::string_view text) {
        if (text.size()>4*1024*1024) throw std::invalid_argument("map exceeds 4 MiB");
        std::istringstream in{std::string(text)};
        std::string magic; int width{},height{};
        if (!(in>>magic>>width>>height)||magic!="MAP1") throw std::invalid_argument("map header");
        Grid candidate(width,height);
        for (int y=0;y<height;++y) for (int x=0;x<width;++x) {
            int tile{};
            if (!(in>>tile)) throw std::invalid_argument("missing tile");
            candidate.Set({x,y},tile);
        }
        in>>std::ws;
        if (!in.eof()) throw std::invalid_argument("trailing map data");
        return candidate;
    }
};
struct Edit { Cell cell; int before{},after{}; };
class TileDocument {
    Grid grid_;
    std::vector<Edit> undo_,redo_;
public:
    explicit TileDocument(Grid grid):grid_(std::move(grid)) {}
    const Grid& Data() const noexcept { return grid_; }
    void Paint(Cell cell,int tile) {
        if (tile<0||tile>4095) throw std::invalid_argument("tile");
        const int old=grid_.Tile(cell);
        if (old==tile) return;
        undo_.push_back(Edit{cell,old,tile}); // allocate history before modifying data
        grid_.Set(cell,tile); redo_.clear();
    }
    bool Undo() {
        if (undo_.empty()) return false;
        const Edit edit=undo_.back(); redo_.push_back(edit);
        grid_.Set(edit.cell,edit.before); undo_.pop_back(); return true;
    }
    bool Redo() {
        if (redo_.empty()) return false;
        const Edit edit=redo_.back(); undo_.push_back(edit);
        grid_.Set(edit.cell,edit.after); redo_.pop_back(); return true;
    }
    void Load(std::string_view text) {
        Grid candidate=Grid::Parse(text); // parse fully before committing
        grid_=std::move(candidate); undo_.clear(); redo_.clear();
    }
};
inline std::optional<std::vector<Cell>> AStar(const Grid& grid,Cell start,Cell goal) {
    if (!grid.Walkable(start)||!grid.Walkable(goal)) return std::nullopt;
    const int width=grid.Width(),count=width*grid.Height(); // bounded by 256*256
    const auto id=[width](Cell p){return p.y*width+p.x;};
    const auto point=[width](int n){return Cell{n%width,n/width};};
    const auto h=[goal](Cell p){return std::abs(goal.x-p.x)+std::abs(goal.y-p.y);};
    std::vector<int> distance(static_cast<std::size_t>(count),std::numeric_limits<int>::max());
    std::vector<int> parent(static_cast<std::size_t>(count),-1);
    using Entry=std::tuple<int,int,int>; // f, g, id; deterministic tiebreak
    std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> open;
    distance[static_cast<std::size_t>(id(start))]=0;
    open.emplace(h(start),0,id(start));
    constexpr std::array<Cell,4> neighbors{{{1,0},{0,1},{-1,0},{0,-1}}};
    while (!open.empty()) {
        const auto [f,g,node]=open.top(); open.pop(); (void)f;
        if (g!=distance[static_cast<std::size_t>(node)]) continue; // stale entry
        const Cell at=point(node);
        if (at==goal) {
            std::vector<Cell> path;
            for (int p=node;p!=-1;p=parent[static_cast<std::size_t>(p)]) path.push_back(point(p));
            std::reverse(path.begin(),path.end()); return path;
        }
        for (Cell offset:neighbors) {
            const Cell next{at.x+offset.x,at.y+offset.y};
            if (!grid.Walkable(next)) continue;
            const int n=id(next),candidate=g+1;
            if (candidate>=distance[static_cast<std::size_t>(n)]) continue;
            distance[static_cast<std::size_t>(n)]=candidate; parent[static_cast<std::size_t>(n)]=node;
            open.emplace(candidate+h(next),candidate,n);
        }
    }
    return std::nullopt;
}
}
#endif

````

## include/loop.hpp

````cpp
#ifndef STUDY_LOOP_HPP
#define STUDY_LOOP_HPP
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
namespace study {
class Input {
    std::array<bool,256> held_{}, pressed_{};
public:
    void Key(std::size_t code,bool down) {
        if (code>=held_.size()) return;
        if (down&&!held_[code]) pressed_[code]=true;
        held_[code]=down;
    }
    bool Held(std::size_t code) const { return code<held_.size()&&held_[code]; }
    bool ConsumePressed(std::size_t code) {
        if (code>=pressed_.size()) return false;
        const bool result=pressed_[code]; pressed_[code]=false; return result;
    }
    void Clear() noexcept { held_.fill(false); pressed_.fill(false); }
};
struct StepResult { int steps{}; double alpha{}, discardedSeconds{}; };
class FixedStepper {
    double accumulator_{};
public:
    static constexpr double step=1.0/120.0;
    static constexpr double maxFrame=0.25;
    static constexpr int maxSteps=8;
    void Reset() noexcept { accumulator_=0; }
    template<class Update> StepResult Advance(double elapsed,Update&& update) {
        if (!std::isfinite(elapsed)||elapsed<0) throw std::invalid_argument("invalid elapsed time");
        StepResult result;
        const double admitted=std::min(elapsed,maxFrame);
        result.discardedSeconds=elapsed-admitted;
        accumulator_+=admitted;
        while (accumulator_+1e-12>=step&&result.steps<maxSteps) {
            update(step);
            accumulator_=std::max(0.0,accumulator_-step);
            ++result.steps;
        }
        if (accumulator_>=step) {
            const double remainder=std::fmod(accumulator_,step);
            result.discardedSeconds+=accumulator_-remainder;
            accumulator_=remainder;
        }
        result.alpha=accumulator_/step;
        return result;
    }
};
class AnimationClock {
    int frames_,index_{}; double interval_,remainder_{};
public:
    AnimationClock(int frames,double interval):frames_(frames),interval_(interval) {
        if (frames<=0||!std::isfinite(interval)||interval<1e-6)
            throw std::invalid_argument("invalid animation");
    }
    void Advance(double seconds) {
        if (!std::isfinite(seconds)||seconds<0||seconds>60)
            throw std::invalid_argument("invalid animation delta");
        const double total=remainder_+seconds;
        const double steps=std::floor((total+1e-12)/interval_);
        remainder_=std::max(0.0,total-steps*interval_);
        const int offset=static_cast<int>(std::fmod(steps,static_cast<double>(frames_)));
        index_=static_cast<int>((static_cast<long long>(index_)+offset)%frames_);
    }
    int Index() const noexcept { return index_; }
};
}
#endif

````

## include/math.hpp

````cpp
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
inline Vec3 Cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
inline std::optional<Vec3> Normalize(Vec3 v, double epsilon=1e-12) {
    if (!Finite(v)||!std::isfinite(epsilon)||epsilon<0) return std::nullopt;
    const double length=std::hypot(v.x,v.y,v.z);
    if (!std::isfinite(length)||length<=epsilon) return std::nullopt;
    return Vec3{v.x/length,v.y/length,v.z/length};
}
inline std::optional<Vec2> Normalize(Vec2 v, double epsilon=1e-12) {
    if (!Finite(v)||!std::isfinite(epsilon)||epsilon<0) return std::nullopt;
    const double length=std::hypot(v.x,v.y);
    if (!std::isfinite(length)||length<=epsilon) return std::nullopt;
    return Vec2{v.x/length,v.y/length};
}
inline void Nonnegative(double n) {
    if (!std::isfinite(n)||n<0) throw std::invalid_argument("finite nonnegative value required");
}
inline Vec2 MoveTowards(Vec2 p, Vec2 target, double speed, double dt) {
    Nonnegative(speed); Nonnegative(dt);
    if (!Finite(p)||!Finite(target)) throw std::invalid_argument("invalid point");
    const Vec2 d=target-p;
    const double distance=std::hypot(d.x,d.y), step=speed*dt;
    if (!std::isfinite(distance)||!std::isfinite(step)) throw std::overflow_error("motion range");
    if (distance<=step||distance==0) return target;
    return p+d*(step/distance);
}
inline double WrapAngle(double radians) {
    if (!std::isfinite(radians)) throw std::invalid_argument("invalid angle");
    constexpr double pi=std::numbers::pi;
    double r=std::remainder(radians,2*pi);
    if (r<=-pi) r+=2*pi; // deterministic choice at the opposite direction
    return r;
}
inline double TurnTowards(double current,double target,double rate,double dt) {
    Nonnegative(rate); Nonnegative(dt);
    const double bound=rate*dt;
    if (!std::isfinite(bound)) throw std::overflow_error("angular step");
    const double delta=WrapAngle(WrapAngle(target)-WrapAngle(current));
    return WrapAngle(current+std::clamp(delta,-bound,bound));
}
inline double JumpHeight(double initialY,double initialSpeed,double acceleration,double time) {
    Nonnegative(time);
    if (!std::isfinite(initialY)||!std::isfinite(initialSpeed)||!std::isfinite(acceleration))
        throw std::invalid_argument("invalid jump input");
    const double result=initialY+initialSpeed*time+0.5*acceleration*time*time;
    if (!std::isfinite(result)) throw std::overflow_error("jump range");
    return result;
}
using Vec4=std::array<double,4>;
struct Mat4 { std::array<std::array<double,4>,4> m{}; };
inline Mat4 Identity() {
    Mat4 a; for (std::size_t i=0;i<4;++i) a.m[i][i]=1; return a;
}
inline Mat4 operator*(const Mat4& a,const Mat4& b) {
    Mat4 r;
    for (std::size_t i=0;i<4;++i)
        for (std::size_t j=0;j<4;++j)
            for (std::size_t k=0;k<4;++k) r.m[i][j]+=a.m[i][k]*b.m[k][j];
    return r;
}
inline Vec4 Transform(Vec4 v,const Mat4& m) {
    Vec4 r{};
    for (std::size_t j=0;j<4;++j)
        for (std::size_t i=0;i<4;++i) r[j]+=v[i]*m.m[i][j];
    return r; // row-vector v * M; no implicit perspective divide
}
inline Mat4 Scale(double x,double y,double z) {
    Mat4 a=Identity(); a.m[0][0]=x; a.m[1][1]=y; a.m[2][2]=z; return a;
}
inline Mat4 Translation(double x,double y,double z) {
    Mat4 a=Identity(); a.m[3][0]=x; a.m[3][1]=y; a.m[3][2]=z; return a;
}
inline Mat4 RotationZ(double angle) {
    Mat4 a=Identity(); const double c=std::cos(angle),s=std::sin(angle);
    a.m[0][0]=c; a.m[0][1]=s; a.m[1][0]=-s; a.m[1][1]=c; return a;
}
inline Mat4 Transpose(const Mat4& a) {
    Mat4 r; for (std::size_t i=0;i<4;++i) for (std::size_t j=0;j<4;++j) r.m[i][j]=a.m[j][i];
    return r;
}
// Partial pivoting, absolute pivot tolerance: suitable for these bounded examples,
// not a condition-number-aware general-purpose numerical library.
inline std::optional<Mat4> Inverse(Mat4 a,double tolerance=1e-12) {
    if (!std::isfinite(tolerance)||tolerance<=0) return std::nullopt;
    for (const auto& row:a.m) for (double v:row) if (!std::isfinite(v)) return std::nullopt;
    Mat4 out=Identity();
    for (std::size_t col=0;col<4;++col) {
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
inline bool Overlap(Aabb a,Aabb b) {
    if (!Valid(a)||!Valid(b)) throw std::invalid_argument("invalid box");
    return a.min.x<=b.max.x&&b.min.x<=a.max.x&&a.min.y<=b.max.y&&b.min.y<=a.max.y;
}
inline bool CircleOverlap(Vec2 a,double ar,Vec2 b,double br) {
    Nonnegative(ar); Nonnegative(br);
    if (!Finite(a)||!Finite(b)) throw std::invalid_argument("invalid center");
    const double radius=ar+br;
    if (!std::isfinite(radius)) throw std::overflow_error("radius range");
    return std::hypot(a.x-b.x,a.y-b.y)<=radius; // touching counts as contact
}
inline Vec2 ClosestOnSegment(Vec2 p,Vec2 a,Vec2 b) {
    if (!Finite(p)||!Finite(a)||!Finite(b)) throw std::invalid_argument("invalid segment");
    const Vec2 ab=b-a; const double lengthSquared=Dot(ab,ab);
    if (lengthSquared<=1e-24) return a;
    const double t=std::clamp(Dot(p-a,ab)/lengthSquared,0.0,1.0);
    return a+ab*t;
}
// Isometric projection plane uses right/down axes; do not mix it with Camera2D's
// world-up coordinates without an explicit sign conversion.
inline Vec2 IsoCenter(double u,double v,double halfWidth,double halfHeight) {
    if (!std::isfinite(u)||!std::isfinite(v)||!std::isfinite(halfWidth)||!std::isfinite(halfHeight)||halfWidth<=0||halfHeight<=0)
        throw std::invalid_argument("invalid isometric input");
    return {(u-v)*halfWidth,(u+v)*halfHeight};
}
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

````

## src/geometry.cpp

````cpp
#include "math.hpp"
#include "check.hpp"
using namespace study;
int main() {
    Check(Overlap({{0,0},{2,2}},{{2,1},{3,3}}),"AABB touching is contact");
    Check(!Overlap({{0,0},{2,2}},{{2.01,1},{3,3}}),"AABB separated");
    Check(Overlap({{0,0},{2,2}},{{.5,.5},{1,1}}),"AABB contained");
    Throws([]{ (void)Overlap({{2,0},{1,1}},{{0,0},{1,1}}); });
    Check(CircleOverlap({0,0},1,{2,0},1),"circle touching");
    Check(!CircleOverlap({0,0},1,{2.1,0},1),"circle separated");
    Throws([]{ (void)CircleOverlap({},-1,{},1); });
    const auto vertical=ClosestOnSegment({2,3},{0,0},{0,5});
    Check(Near(vertical.x,0)&&Near(vertical.y,3),"vertical segment");
    const auto endpoint=ClosestOnSegment({2,9},{0,0},{0,5});
    Check(Near(endpoint.y,5),"clamp segment");
    const auto degenerate=ClosestOnSegment({5,6},{1,2},{1,2});
    Check(Near(degenerate.x,1)&&Near(degenerate.y,2),"degenerate segment");
    for (int y=0;y<8;++y) for (int x=0;x<8;++x) {
        const auto picked=PickIso(IsoCenter(x,y,32,16),8,8,32,16);
        Check(picked&&picked->first==x&&picked->second==y,"isometric center roundtrip");
    }
    Check(!PickIso(IsoCenter(-.6,0,32,16),8,8,32,16),"negative outside not truncated to zero");
    const auto edge=PickIso(IsoCenter(.5,0,32,16),8,8,32,16);
    Check(edge&&edge->first==1,"half-open shared boundary");
    Check(!PickIso(IsoCenter(7.5,0,32,16),8,8,32,16),"right outer boundary excluded");
    Throws([]{ (void)PickIso({},2,2,0,16); });
    return Done("geometry");
}

````

## src/loop_input.cpp

````cpp
#include <limits>
#include "loop.hpp"
#include "check.hpp"
using namespace study;
int main() {
    for (int fps:{30,60,144}) {
        FixedStepper stepper; int count{}; double position{};
        for (int frame=0;frame<fps;++frame) {
            const auto result=stepper.Advance(1.0/fps,[&](double dt){++count;position+=120*dt;});
            Check(result.alpha>=0&&result.alpha<1.00000001,"alpha range");
            Check(Near(result.discardedSeconds,0),"ordinary frame not dropped");
        }
        Check(count==120&&Near(position,120),"same second different render rate");
    }
    FixedStepper stepper;
    const auto stalled=stepper.Advance(1.0,[](double){});
    Check(stalled.steps==8&&stalled.discardedSeconds>.9,"catch-up cap and discarded time");
    stepper.Reset(); Check(stepper.Advance(0,[](double){}).steps==0,"pause reset");
    Throws([&]{ (void)stepper.Advance(-1,[](double){}); });
    Throws([&]{ (void)stepper.Advance(std::numeric_limits<double>::quiet_NaN(),[](double){}); });
    Input input; input.Key(32,true); input.Key(32,true);
    Check(input.Held(32)&&input.ConsumePressed(32),"press edge");
    Check(!input.ConsumePressed(32),"edge consumed once");
    input.Key(32,false); input.Key(32,true); input.Key(32,false);
    Check(!input.Held(32)&&input.ConsumePressed(32),"short tap preserved");
    input.Key(32,true); input.Clear(); Check(!input.Held(32)&&!input.ConsumePressed(32),"focus loss clears input");
    input.Key(999,true); Check(!input.Held(999),"out of range key ignored");
    AnimationClock animation(4,.1); animation.Advance(.25);
    Check(animation.Index()==2,"animation handles multiple frames");
    animation.Advance(.05); Check(animation.Index()==3,"animation keeps remainder");
    animation.Advance(.2); Check(animation.Index()==1,"animation wraps");
    Throws([]{ AnimationClock bad(0,.1); });
    return Done("loop_input");
}

````

## src/pathfinding.cpp

````cpp
#include "grid.hpp"
#include "check.hpp"
using namespace study;
int BfsDistance(const Grid& g,Cell start,Cell goal) {
    if (!g.Walkable(start)||!g.Walkable(goal)) return -1;
    const auto id=[&](Cell p){return p.y*g.Width()+p.x;};
    std::vector<int> distance(static_cast<std::size_t>(g.Width()*g.Height()),-1);
    std::queue<Cell> q; q.push(start); distance[static_cast<std::size_t>(id(start))]=0;
    const std::array<Cell,4> offsets{{{0,-1},{-1,0},{0,1},{1,0}}};
    while (!q.empty()) {
        const Cell p=q.front();q.pop(); if (p==goal) return distance[static_cast<std::size_t>(id(p))];
        for (auto d:offsets) {
            Cell n{p.x+d.x,p.y+d.y};
            if (!g.Walkable(n)||distance[static_cast<std::size_t>(id(n))]!=-1) continue;
            distance[static_cast<std::size_t>(id(n))]=distance[static_cast<std::size_t>(id(p))]+1; q.push(n);
        }
    }
    return -1;
}
int main() {
    int scenarios{};
    for (unsigned mask=0;mask<512;++mask) {
        Grid grid(3,3);
        for (unsigned bit=0;bit<9;++bit) if ((mask&(1u<<bit))!=0)
            grid.Set({static_cast<int>(bit%3),static_cast<int>(bit/3)},1);
        for (int s=0;s<9;++s) for (int t=0;t<9;++t) {
            const Cell start{s%3,s/3},goal{t%3,t/3}; ++scenarios;
            const auto path=AStar(grid,start,goal); const int distance=BfsDistance(grid,start,goal);
            Check(path.has_value()==(distance>=0),"A* reachability agrees with BFS");
            if (!path) continue;
            Check(path->front()==start&&path->back()==goal,"path endpoints");
            Check(path->size()==static_cast<std::size_t>(distance+1),"A* optimal length");
            for (std::size_t i=0;i<path->size();++i) {
                Check(grid.Walkable((*path)[i]),"path avoids obstacle");
                if (i) Check(std::abs((*path)[i].x-(*path)[i-1].x)+std::abs((*path)[i].y-(*path)[i-1].y)==1,"four-way path adjacency");
            }
        }
    }
    Check(!AStar(Grid(3,3),{-1,0},{0,0}),"outside endpoint rejected");
    std::cout<<scenarios<<" exhaustive 3x3 map/start/goal scenarios\n";
    return Done("pathfinding");
}

````

## src/tile_document.cpp

````cpp
#include "grid.hpp"
#include "check.hpp"
#include <future>
using namespace study;
int main() {
    TileDocument doc(Grid(4,3));
    doc.Paint({1,1},7); Check(doc.Data().Tile({1,1})==7,"paint");
    Check(doc.Undo()&&doc.Data().Tile({1,1})==0,"undo");
    Check(doc.Redo()&&doc.Data().Tile({1,1})==7,"redo");
    Check(doc.Undo(),"second undo"); doc.Paint({0,0},5);
    Check(!doc.Redo(),"new edit discards redo");
    const std::string original=doc.Data().Serialize();
    const auto copy=Grid::Parse(original);
    Check(copy.Serialize()==original,"map text roundtrip");
    for (std::string bad:{"MAP2 1 1 0","MAP1 0 2","MAP1 257 1","MAP1 1 1 -1", "MAP1 1 1 4096","MAP1 2 1 0","MAP1 1 1 0 junk"}) {
        Throws([&]{doc.Load(bad);});
        Check(doc.Data().Serialize()==original,"failed load preserves document");
    }
    Throws([&]{doc.Paint({-1,0},2);});
    Check(doc.Data().Serialize()==original,"failed paint preserves document");
    doc.Load("MAP1 2 2\n0 1\n2 3\n"); Check(doc.Data().Tile({1,1})==3,"valid load");
    Check(!doc.Undo()&&!doc.Redo(),"load invalidates edit history");
    // Workers receive a value snapshot. Applying to UI/document is a separate action.
    const int submittedRevision=1, currentRevision=2;
    auto job=std::async(std::launch::async,[snapshot=Grid(3,3)]{return AStar(snapshot,{0,0},{2,2});});
    const auto path=job.get();
    Check(path&&path->size()==5,"worker computes from snapshot");
    Check(submittedRevision!=currentRevision,"stale result must be rejected");
    return Done("tile_document");
}

````

## src/transforms.cpp

````cpp
#include "math.hpp"
#include "check.hpp"
using namespace study;
int main() {
    const Mat4 s=Scale(2,2,1),r=RotationZ(std::numbers::pi/2),t=Translation(10,0,0);
    const Mat4 world=s*r*t; const auto p=Transform({1,0,0,1},world);
    Check(Near(p[0],10)&&Near(p[1],2)&&Near(p[3],1),"row SRT");
    const auto reverse=Transform({1,0,0,1},t*r*s);
    Check(Near(reverse[0],0)&&Near(reverse[1],22),"noncommutative transform");
    const auto direction=Transform({1,0,0,0},world);
    Check(Near(direction[0],0)&&Near(direction[1],2),"direction ignores translation");
    const auto inverse=Inverse(world); Check(inverse.has_value(),"inverse exists");
    const auto restored=Transform(p,*inverse);
    Check(Near(restored[0],1)&&Near(restored[1],0)&&Near(restored[3],1),"point roundtrip");
    const auto identity=world*(*inverse);
    for (std::size_t i=0;i<4;++i) for (std::size_t j=0;j<4;++j)
        Check(Near(identity.m[i][j],i==j?1.0:0.0),"matrix identity");
    Check(!Inverse(Scale(1,0,1)),"singular rejected");
    Mat4 bad=Identity(); bad.m[1][1]=std::numeric_limits<double>::quiet_NaN();
    Check(!Inverse(bad),"NaN inverse rejected");
    const auto hierarchy=Transform({0,0,0,1},Translation(2,0,0)*RotationZ(std::numbers::pi/2)*Translation(10,0,0));
    Check(Near(hierarchy[0],10)&&Near(hierarchy[1],2),"local then parent");
    const Mat4 nonuniform=Scale(2,1,1); const auto normalMatrix=Transpose(*Inverse(nonuniform));
    const auto tangent=Transform({1,1,0,0},nonuniform);
    const auto normal=Transform({-1,1,0,0},normalMatrix);
    Check(Near(tangent[0]*normal[0]+tangent[1]*normal[1],0),"inverse transpose normal");
    const auto wrong=Transform({-1,1,0,0},nonuniform);
    Check(!Near(tangent[0]*wrong[0]+tangent[1]*wrong[1],0),"ordinary normal transform fails");
    Camera2D camera{{10,20},{400,300},2};
    const auto screen=camera.ToScreen({13,24});
    Check(Near(screen.x,406)&&Near(screen.y,292),"world up to screen down");
    const auto worldAgain=camera.ToWorld(screen);
    Check(Near(worldAgain.x,13)&&Near(worldAgain.y,24),"camera inverse");
    Throws([]{ Camera2D badCamera; badCamera.zoom=0; (void)badCamera.ToWorld({}); });
    std::cout<<"SRT(1,0,0,1) = ("<<p[0]<<','<<p[1]<<','<<p[2]<<','<<p[3]<<")\n";
    return Done("transforms");
}

````

## src/vector_motion.cpp

````cpp
#include "math.hpp"
#include "check.hpp"
using namespace study;
int main() {
    Check(Near(Dot(Vec3{1,2,3},Vec3{4,5,6}),32),"dot formula");
    const auto cross=Cross({1,0,0},{0,1,0});
    Check(Near(cross.z,1)&&Near(cross.x,0)&&Near(cross.y,0),"basis cross");
    const Vec3 a{2,-1,4},b{3,5,-2}; const Vec3 c=Cross(a,b);
    Check(Near(Dot(a,c),0)&&Near(Dot(b,c),0),"cross orthogonal");
    Check(Near(Cross(b,a).x,-c.x)&&Near(Cross(b,a).y,-c.y)&&Near(Cross(b,a).z,-c.z),"anti commutative");
    Check(!Normalize(Vec3{}),"zero rejected");
    Check(!Normalize(Vec3{std::numeric_limits<double>::infinity(),0,0}),"infinity rejected");
    Check(!Normalize(Vec2{std::numeric_limits<double>::quiet_NaN(),0}),"NaN rejected");
    const auto n=Normalize(Vec2{3,4});
    Check(n&&Near(n->x,.6)&&Near(n->y,.8),"3 4 normalization");
    const auto diagonal=Normalize(Vec2{1,1});
    Check(diagonal&&Near(std::hypot(diagonal->x,diagonal->y),1),"diagonal speed");
    const auto p=MoveTowards({0,0},{3,4},2,1);
    Check(Near(p.x,1.2)&&Near(p.y,1.6),"two units per second");
    const auto q=MoveTowards({0,0},{3,4},10,1);
    Check(Near(q.x,3)&&Near(q.y,4),"no overshoot");
    Check(Near(MoveTowards({1,2},{1,2},0,1).y,2),"same target");
    Throws([]{ (void)MoveTowards({},{1,1},-1,1); });
    Throws([]{ (void)MoveTowards({},{1,1},1,-1); });
    constexpr double pi=std::numbers::pi;
    Check(Near(WrapAngle(-pi),pi),"opposite convention");
    Check(Near(TurnTowards(170*pi/180,-170*pi/180,5*pi/180,1),175*pi/180),"shortest turn");
    Check(Near(JumpHeight(0,10,-10,1),5),"jump apex");
    Check(Near(JumpHeight(0,10,-10,2),0),"jump return");
    Check(Near(std::atan2(1.0,-1.0),3*pi/4),"atan2 quadrant");
    return Done("vector_motion");
}

````

## verify.py

````python
#!/usr/bin/env python3
"""Rebuild portable tests in a fresh temporary directory; preserve per-run logs."""
from __future__ import annotations
import argparse
import datetime as dt
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--sanitizers', action='store_true')
    args = parser.parse_args()
    compiler = shutil.which(args.compiler)
    if not compiler or not shutil.which('cmake') or not shutil.which('ctest'):
        print('The selected compiler, cmake and ctest must be available on PATH.', file=sys.stderr)
        return 2
    root = Path(__file__).resolve().parent
    stamp = dt.datetime.now(dt.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs = root / 'validation_runs' / stamp
    logs.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    if args.sanitizers:
        env['ASAN_OPTIONS'] = 'detect_leaks=1'
        env['UBSAN_OPTIONS'] = 'halt_on_error=1'
    try:
        with tempfile.TemporaryDirectory(prefix='study-math-build-') as temporary:
            commands = [
                ('configure', ['cmake', '-S', str(root), '-B', temporary,
                               '-DCMAKE_BUILD_TYPE=Debug', f'-DCMAKE_CXX_COMPILER={compiler}',
                               f'-DENABLE_SANITIZERS={"ON" if args.sanitizers else "OFF"}']),
                ('build', ['cmake', '--build', temporary, '--config', 'Debug', '-j', '2']),
                ('ctest', ['ctest', '--test-dir', temporary, '-C', 'Debug', '-V']),
            ]
            for name, command in commands:
                result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                                        stderr=subprocess.STDOUT, env=env, timeout=180)
                (logs / f'{name}.log').write_text(result.stdout, encoding='utf-8')
                print(f'{name}: exit {result.returncode}')
                if result.returncode:
                    print(result.stdout, file=sys.stderr)
                    return result.returncode
    except (OSError, subprocess.TimeoutExpired) as error:
        print(str(error), file=sys.stderr)
        return 1
    print(f'Logs saved to {logs}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

````

## windows/app.manifest

````xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <assemblyIdentity version="1.0.0.0" processorArchitecture="*" name="Study.GameMathWinApiLab" type="win32"/>
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security><requestedPrivileges><requestedExecutionLevel level="asInvoker" uiAccess="false"/></requestedPrivileges></security>
  </trustInfo>
  <application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings>
      <dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">PerMonitorV2</dpiAwareness>
    </windowsSettings>
  </application>
</assembly>

````

## windows/app.rc

````rc
1 24 "app.manifest"

````

## windows/winapi_demo.cpp

````cpp
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

````

## source_hashes.json

````json
{
  "CMakeLists.txt": "0d4ac2df5f5d72740e95ab14a448a97f87afd5cb82f3d07b2e5dd1114f6489bc",
  "README.md": "831ba4a32321665c3688c5c8f0e0d12f9832dc2f5cfb6996a6dc61121f91f317",
  "docs/validation.md": "cc9be22606f37f219903198f2ea2b68077e5582907a95124bf03e0ee0140be35",
  "include/check.hpp": "8640c131fcd59efeb0250da50b7014f7503db155d7e70aa6d061330c1dfbbf2a",
  "include/grid.hpp": "3ebe1bc309a4c071871e2674e1eb72cfe55710e2188c7eb25ac8795316a2acd1",
  "include/loop.hpp": "827cbbb08ce3e008a1c8c9ab85d8d27595a004e43372179fb4a30ff6a8a09ad7",
  "include/math.hpp": "1d5fecf1a62f79ba4010f12eda316e1e5d20cf80d1845e4c9427c2cc310dc612",
  "logs/clang-build.log": "44aa39f171003e1afa6686b9aeadc658cb2e9883932bb304e297dd0f956be790",
  "logs/clang-configure.log": "2c54c4953af93744f30ae4c5b978088bdfa92861ee1bec058652aa35c2a9ca67",
  "logs/clang-ctest.log": "6a2aad866c6af4e22348ce2befe42b54a568f0c5f3c4a7187ffe8d4f53512d25",
  "logs/gcc-build.log": "a786565e3f3ff373991f354b9ff21c9277cc5b4b515e867bee1688e8c8e2f1af",
  "logs/gcc-configure.log": "5688a0bd3708956421e2d37298ba192690cc68489715ffd3c2b5581fd7cc8f6c",
  "logs/gcc-ctest.log": "22ab860df88850d4d6819a90d891e922097ea6bfa61871f21d3fc8ec959e5136",
  "logs/header-checks.log": "15e9be71189f6a8a73f0a87e33b5a5a1371fd8045ec1de1c9eb99b8c02d02c19",
  "logs/sanitizer-build.log": "ec2963e977095a09708f6189163860690c3af08f07ae06db5acb5aced4817460",
  "logs/sanitizer-configure.log": "ff2eaa1f32ba370b6898bd6c388ff77c5eaa770122ea2730d1352cc0a11f3a98",
  "logs/sanitizer-ctest.log": "8095452169a775d3537c7ae358b58cea96b9edc7297134aae46e95051cdab7ec",
  "src/geometry.cpp": "771f45f5aac44d7af138a49041d9f82169b281aa0b718accb250e8db01cdbc0d",
  "src/loop_input.cpp": "af729b1f0b337cbb1beebdc3920c797aee8d584427bdf598730261ffed497733",
  "src/pathfinding.cpp": "d51e3cd7c94d27dfd0c753593c2db80e42a13341b9129b99c18bbbf3919407cc",
  "src/tile_document.cpp": "0fe520476ad9973ecf95d655bb1db720f8e3b5ffe550d70a570c09075e07b73b",
  "src/transforms.cpp": "7cb07b8d1686faee69675e9311f288b2730f465aea0b809104169aad909041df",
  "src/vector_motion.cpp": "9bd2387baa4fbc803fd1d4d0dae89367cf21f90ffe028d4698da8f7e03ace743",
  "verify.py": "8daad603eeed8b0733894de878902f952298290dd805fae1aeb9399832732847",
  "windows/app.manifest": "e53d17be3732a8f6d9407fd5133b992f425196301b306f1b9e1c355969b6ff7e",
  "windows/app.rc": "dee796c28bae15cf5c74ac1ece175de1b3f91771a9bd2fe9a116990d9d6eb7c7",
  "windows/winapi_demo.cpp": "1b847b552d96f79d0a26602d63b86c68a3b45af6296fbb754c2a87554808ee93"
}

````
