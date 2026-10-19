
<callout icon="📦" color="blue_bg">
	**수동 검수 13 · 04 확장 실습 보관 복구**
	<mention-date start="2026-10-16"/>. 실제 회수한 renderer_components.cpp 전체를 보존합니다. 기존 04 학습 본문은 <mention-page url="https://app.notion.com/p/bdcccf3ce6d34930827bf0a519b4fa78">쥬신 5–6개월차 — DirectX 9</mention-page>에 있습니다. 아래 코드는 Direct3D9·COM을 호출하지 않는 CPU 모델입니다. 기존 알고리즘을 중복 작성하지 않고 실행·보관 경로를 복구했습니다.
</callout>
## 파일과 실행 계약
아래 첫 블록을 renderer_components.cpp, 다음 블록을 CMakeLists.txt로 같은 폴더에 저장합니다. 외부 입력이나 에셋은 필요 없습니다. 학습 빌드 기본은 C++20, 같은 소스의 C++11 호환성도 확인했습니다. C++11에 도입된 unique_ptr, shared_ptr/weak_ptr, 이동, 람다와 enum class를 사용합니다. 회사 프로젝트 표준은 변경하지 않습니다.
이 모델의 TextureAsset은 이름만 가진 일반 C++ 객체입니다. shared_ptr의 소유 수를 D3D9의 COM 참조 수로 읽으면 안 됩니다. Flush는 weak_ptr를 lock하여 살아 있는 객체에 대해서만 처리하고, 임시 shared_ptr를 Flush 종료까지 유지합니다. Opaque 앞→뒤, Alpha 뒤→앞, UI 등록 순서는 이 예제의 정책이며 실제 반투명 삼각형 교차를 완벽히 해결하지 않습니다.
TransformPoint는 아핀 행렬을 전제로 이동까지 적용하며 원근 나눗셈은 없습니다. 유효한 Group 열거값·유한한 Transform 입력은 호출자 전제입니다. Add는 비유한 depth를 거부하지만 모든 게임 데이터의 유효성을 증명하는 검증 계층은 아닙니다.
## renderer_components.cpp — 전체
```c++
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// CPU-only educational model: this does NOT call Direct3D 9 or model COM AddRef.
struct Vec3 { double x, y, z; };
struct Mat4 {
    double a[4][4] = {};
    static Mat4 Identity() { Mat4 r; for (int i=0;i<4;++i) r.a[i][i]=1.0; return r; }
    static Mat4 Scale(double x,double y,double z) {
        Mat4 r=Identity(); r.a[0][0]=x; r.a[1][1]=y; r.a[2][2]=z; return r;
    }
    static Mat4 Yaw(double t) {
        Mat4 r=Identity(); double c=std::cos(t),s=std::sin(t);
        r.a[0][0]=c; r.a[0][2]=-s; r.a[2][0]=s; r.a[2][2]=c;
        return r;
    }
    static Mat4 Translate(Vec3 p) {
        Mat4 r=Identity(); r.a[3][0]=p.x; r.a[3][1]=p.y; r.a[3][2]=p.z; return r;
    }
};
Mat4 Mul(const Mat4& x,const Mat4& y) {
    Mat4 z{}; for(int i=0;i<4;++i) for(int j=0;j<4;++j)
        for(int k=0;k<4;++k) z.a[i][j]+=x.a[i][k]*y.a[k][j];
    return z;
}
Vec3 TransformPoint(Vec3 p, const Mat4& m) { // [x y z 1] * M (row-vector)
    return {p.x*m.a[0][0]+p.y*m.a[1][0]+p.z*m.a[2][0]+m.a[3][0],
            p.x*m.a[0][1]+p.y*m.a[1][1]+p.z*m.a[2][1]+m.a[3][1],
            p.x*m.a[0][2]+p.y*m.a[1][2]+p.z*m.a[2][2]+m.a[3][2]};
}
struct Transform {
    Vec3 position{0,0,0};
    Vec3 scale{1,1,1};
    double yaw=0;
    Mat4 Local() const {return Mul(Mul(Mat4::Scale(scale.x,scale.y,scale.z),Mat4::Yaw(yaw)), Mat4::Translate(position));}
    Mat4 World(const Mat4& parentWorld=Mat4::Identity()) const {return Mul(Local(),parentWorld);}
};
struct Component {
    virtual ~Component() = default;
    virtual std::unique_ptr<Component> Clone() const = 0;
};
struct TextureAsset { explicit TextureAsset(std::string n):name(std::move(n)) {} std::string name; };
struct TextureComponent : Component {
    explicit TextureComponent(std::shared_ptr<TextureAsset> p):asset(std::move(p)) {
        if(!asset) throw std::invalid_argument("null texture");
    }
    std::shared_ptr<TextureAsset> asset; // CPU handle only, not a COM reference.
    std::unique_ptr<Component> Clone() const override {
        return std::unique_ptr<Component>(new TextureComponent(asset));
    }
};
struct ComponentCatalog {
    std::map<std::string,std::unique_ptr<Component>> prototypes;
    void Add(const std::string& key,std::unique_ptr<Component> p) {
        if(key.empty()||!p||prototypes.count(key)) throw std::invalid_argument("invalid/duplicate prototype");
        prototypes.emplace(key,std::move(p));
    }
    std::unique_ptr<Component> Create(const std::string& key) const {
        auto p=prototypes.find(key);
        if(p==prototypes.end()) throw std::out_of_range("unknown component");
        return p->second->Clone();
    }
};
struct GameObject {
    explicit GameObject(std::string s):name(std::move(s)) {}
    std::string name;
    Transform transform;
    std::map<std::string,std::unique_ptr<Component>> components;
    void Attach(std::string key,std::unique_ptr<Component> p) {
        if(!p||components.count(key)) throw std::invalid_argument("duplicate/null component");
        components.emplace(std::move(key),std::move(p));
    }
    const TextureAsset* Texture() const {
        auto p=components.find("texture"); if(p==components.end()) return nullptr;
        auto t=dynamic_cast<TextureComponent*>(p->second.get());
        return t ? t->asset.get():nullptr;
    }
};
enum class Group { Opaque, Alpha, UI };
struct RenderState {
    bool depthTest,depthWrite,blend; std::string boundTexture;
    RenderState(bool test=true,bool write=true,bool blending=false,std::string texture="")
        :depthTest(test),depthWrite(write),blend(blending),boundTexture(std::move(texture)) {}
};
struct RenderEvent { std::string object; Group group; double depth; RenderState state; };
struct RenderQueue {
    struct Entry { std::weak_ptr<GameObject> object; Group group; double depth; std::size_t order; };
    std::vector<Entry> queue;
    std::size_t serial=0;
    RenderState state;
    void BeginFrame() {queue.clear();serial=0;state=RenderState{};}
    void Add(std::weak_ptr<GameObject> object, Group group, double depth) {
        if(!std::isfinite(depth)) throw std::invalid_argument("nonfinite depth");
        queue.push_back({object,group,depth,serial++});
    }
    static RenderState StateFor(Group group) {
        if(group==Group::Opaque) return {true,true,false,""};
        if(group==Group::Alpha) return {true,false,true,""};
        return {false,false,true,""}; // example UI: no depth test, per-design policy.
    }
    std::vector<RenderEvent> Flush() {
        struct Live {std::shared_ptr<GameObject> object; Group group; double depth; std::size_t order;};
        std::vector<Live> live;
        for(const auto& item:queue) if(auto p=item.object.lock())
            live.push_back({p,item.group,item.depth,item.order});
        queue.clear(); // no stale registrations after flushing
        std::stable_sort(live.begin(),live.end(),[](const Live& a,const Live& b){
            if(a.group!=b.group) return static_cast<int>(a.group)<static_cast<int>(b.group);
            if(a.group==Group::Alpha) return a.depth>b.depth; // back-to-front demo
            if(a.group==Group::Opaque) return a.depth<b.depth; // front-to-back demo
            return a.order<b.order;
        });
        std::vector<RenderEvent> events;
        for(const auto& e:live) {
            state=StateFor(e.group);  // initialize critical states on EACH draw/pass
            if(auto tex=e.object->Texture()) state.boundTexture=tex->name;
            events.push_back({e.object->name,e.group,e.depth,state});
        }
        state=RenderState{}; // explicit invariant for next frame in this model
        return events;
    }
};

int main() {
    int checks=0;
    const auto check=[&](bool b,const char* m){++checks;if(!b) throw std::runtime_error(m);};
    const auto near=[&](double a,double b){return std::abs(a-b)<1e-9;};
    Transform t; t.position={1,2,3};t.scale={2,2,2};t.yaw=std::acos(-1.0)/2;
    Vec3 local=TransformPoint({1,0,0},t.Local());
    check(near(local.x,1)&&near(local.y,2)&&near(local.z,1),"S*R*T row-vector");
    Vec3 world=TransformPoint({1,0,0},t.World(Mat4::Translate({10,0,0})));
    check(near(world.x,11)&&near(world.y,2)&&near(world.z,1),"child local * parent world");
    Vec3 origin=TransformPoint({0,0,0},t.Local());
    check(near(origin.x,1)&&near(origin.y,2)&&near(origin.z,3),"translation placement");

    auto texture=std::make_shared<TextureAsset>("smoke.dds");
    ComponentCatalog catalog;
    catalog.Add("smoke",std::unique_ptr<Component>(new TextureComponent(texture)));
    std::unique_ptr<Component> compA=catalog.Create("smoke");
    std::unique_ptr<Component> compB=catalog.Create("smoke");
    check(compA.get()!=compB.get(),"component instance clone independent");
    auto ta=dynamic_cast<TextureComponent*>(compA.get());
    auto tb=dynamic_cast<TextureComponent*>(compB.get());
    check(ta&&tb&&ta->asset==tb->asset,"asset handle shared");
    check(texture.use_count()==4,"texture owners: local+prototype+two clones");
    bool rejectedDuplicate=false;try {catalog.Add("smoke",std::unique_ptr<Component>(new TextureComponent(texture)));}
    catch(const std::invalid_argument&) {rejectedDuplicate=true;}
    check(rejectedDuplicate,"duplicate prototype rejected");
    bool missingRejected=false;try {catalog.Create("missing");}catch(const std::out_of_range&){missingRejected=true;}
    check(missingRejected,"missing prototype rejected");

    auto nearOpaque=std::make_shared<GameObject>("near-opaque");
    auto farOpaque=std::make_shared<GameObject>("far-opaque");
    auto farAlpha=std::make_shared<GameObject>("far-alpha");
    auto nearAlpha=std::make_shared<GameObject>("near-alpha");
    auto ui=std::make_shared<GameObject>("hud");
    farAlpha->Attach("texture",std::move(compA));
    nearAlpha->Attach("texture",std::move(compB));
    check(farAlpha->Texture()==nearAlpha->Texture(),"two owned components share one asset");
    RenderQueue queue;queue.BeginFrame();
    queue.Add(nearAlpha,Group::Alpha,3);
    queue.Add(farOpaque,Group::Opaque,8);
    queue.Add(ui,Group::UI,0);
    queue.Add(farAlpha,Group::Alpha,10);
    queue.Add(nearOpaque,Group::Opaque,2);
    auto dead=std::make_shared<GameObject>("despawned");
    queue.Add(dead,Group::Opaque,1);dead.reset(); // queue borrows; no use-after-free
    const auto events=queue.Flush();
    check(events.size()==5,"expired borrowed item skipped");
    const char* order[]={"near-opaque","far-opaque","far-alpha","near-alpha","hud"};
    for(std::size_t i=0;i<5;++i) check(events[i].object==order[i],"render order");
    check(events[0].state.depthWrite&&!events[0].state.blend,"opaque state");
    check(events[2].state.depthTest&&!events[2].state.depthWrite&&events[2].state.blend,"alpha state");
    check(!events[4].state.depthTest&&!events[4].state.depthWrite&&events[4].state.blend,"ui state");
    check(events[2].state.boundTexture=="smoke.dds","texture stage setup");
    check(events[0].state.boundTexture.empty(),"no leaked texture between passes");
    check(queue.queue.empty(),"per-frame queue cleared");
    check(queue.state.depthWrite&&!queue.state.blend&&queue.state.boundTexture.empty(),"state reset");
    queue.BeginFrame();check(queue.Flush().empty(),"empty frame");
    bool badDepth=false;try {queue.Add(ui,Group::UI,std::nan(""));}
    catch(const std::invalid_argument&) {badDepth=true;}
    check(badDepth,"nonfinite depth rejected");
    check(queue.queue.empty(),"bad item not queued");
    farAlpha.reset();nearAlpha.reset();catalog.prototypes.clear();
    check(texture.use_count()==1,"ownership released after owners cleared");
    std::cout<<"PASS: "<<checks<<" conditions; groups=opaque(front-back),alpha(back-front),ui; CPU-only\n";
}
```
## CMakeLists.txt — 이 확장 소스의 독립 빌드
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx9_renderer_recovered LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 or 11")
add_executable(renderer_components renderer_components.cpp)
set_target_properties(renderer_components PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
  target_compile_options(renderer_components PRIVATE /W4 /EHsc)
else()
  target_compile_options(renderer_components PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME renderer_components COMMAND renderer_components)
```
## 이번 검증 기록
아래 세 구성에서 동일한 확장 26조건을 통과했습니다. 같은 26조건을 78개의 서로 다른 검사로 합산하지 않습니다. 별도 기존 notion_example.cpp도 각 구성에서 실행했으며 성공 메시지를 확인했지만 해당 미니 프로그램의 조건 수를 새로 계수한 것은 아닙니다. 검수용 배치 CMake는 확장·미니 실행파일 둘을 등록하여 2/2 통과했고, 위 독립 CMake는 확장 하나만 등록합니다.
```bash
cmake -S . -B build-gcc20 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++
cmake --build build-gcc20 -j 2
ctest --test-dir build-gcc20 --output-on-failure -V
cmake -S . -B build-gcc11 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DLAB_CXX_STANDARD=11
cmake --build build-gcc11 -j 2
ctest --test-dir build-gcc11 --output-on-failure -V
cmake -S . -B build-san -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer" "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined"
cmake --build build-san -j 2
ctest --test-dir build-san --output-on-failure -V
```
```plain text
Linux x86-64 / GCC 14.2 C++20: PASS: 26 conditions
Linux x86-64 / GCC 14.2 C++11: PASS: 26 conditions
Linux x86-64 / Clang 17 ASan+UBSan C++20: PASS: 26 conditions
Each batch CTest: 2/2, exit=0; sanitizer diagnostic: none.
Windows/MSVC/COM/Direct3D9/GPU: NOT RUN.
```
## 이해 확인
<details>
<summary>weak_ptr가 큐에 있으므로 모든 객체가 Render까지 살아 있나요?</summary>
	아닙니다. weak_ptr 자체는 수명을 연장하지 않습니다. Flush에서 lock에 성공한 대상만 임시 shared_ptr로 보유합니다. 프레임 도중 소멸한 대상은 건너뜁니다. 실제 엔진의 핸들·삭제 지연 정책과 같은 이름으로 혼동하지 않습니다.
</details>
<details>
<summary>TransformPoint에 원근 투영 행렬을 넣어도 같은 결과인가요?</summary>
	아닙니다. 이 함수는 아핀 좌표 변환을 위한 세 성분 연산입니다. 동차 w 계산과 원근 나눗셈이 없으며, 투영에는 별도 계약이 필요합니다.
</details>
## 변경 이력·근거·잔여
이번 확장 C++ 소스는 원형을 보존했습니다. 네이티브 소스 보관과 CPU 재실행 완료를 GPU 상태 복원·COM·투명 렌더링 실행 검증으로 표현하지 않습니다. CMake의 학습 표준 선택 옵션과 위 재현 안내는 이번에 보완했습니다. 별도 미니 원형 notion_example.cpp도 아래 부록에 전체 파일로 보관했습니다. 확장 실습 26조건과 미니 프로그램의 실행 결과를 구분합니다. 기술 설명과 공식 API 근거는 대표 페이지 04 및 [Microsoft SetTexture](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-settexture), [D3DXVec3TransformCoord](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxvec3transformcoord)를 함께 봅니다.
## 부록 · 기존 미니 실습 전체 소스
아래는 같은 검수 배치에서 실행한 원형 notion_example.cpp입니다. 확장 파일과 별도 실행파일로 빌드하며 main이 둘이므로 함께 링크하지 않습니다. assert 기반이므로 NDEBUG를 정의하지 않습니다. 유한한 depth·Transform, 유효한 Pass, TextureComponent의 비어 있지 않은 resource가 입력 전제입니다. 확장 소스의 입력 검증과 같은 수준이라고 보지 않습니다.
```bash
g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror notion_example.cpp -o renderer_mini
./renderer_mini
```
```c++
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Standalone C++11 CPU model. No Direct3D calls; shared_ptr is NOT COM AddRef.
struct Texture { explicit Texture(const std::string& n) : name(n) {} std::string name; };
struct TextureComponent {
    explicit TextureComponent(std::shared_ptr<Texture> p) : resource(p) {}
    std::shared_ptr<Texture> resource;
    std::unique_ptr<TextureComponent> Clone() const {
        return std::unique_ptr<TextureComponent>(new TextureComponent(resource));
    }
};
struct Transform {
    double x = 0, z = 0, scale = 1, yaw = 0;
    // Row vector, LH +Z forward. Local S*R*T, parent translation only in this model.
    double WorldX(double localX, double localZ, double parentX) const {
        return scale * (localX * std::cos(yaw) + localZ * std::sin(yaw)) + x + parentX;
    }
    double WorldZ(double localX, double localZ) const {
        return scale * (-localX * std::sin(yaw) + localZ * std::cos(yaw)) + z;
    }
};
struct Object {
    explicit Object(const std::string& n) : name(n) {}
    std::string name;
    std::unique_ptr<TextureComponent> texture; // instance owner
};
enum class Pass { Opaque, Alpha, UI };
struct Job {
    std::weak_ptr<Object> ref; // not an owner
    Pass pass;
    double depth;
};
struct Event {
    std::string object;
    bool ztest, zwrite, blend;
    std::string texture;
};
struct Queue {
    std::vector<Job> jobs;
    void Add(const std::shared_ptr<Object>& obj, Pass pass, double depth) {
        jobs.push_back({obj, pass, depth});
    }
    std::vector<Event> Flush() {
        struct Live { std::shared_ptr<Object> obj; Pass pass; double depth; };
        std::vector<Live> live;
        for (const auto& job : jobs)
            if (auto obj = job.ref.lock()) live.push_back({obj, job.pass, job.depth});
        jobs.clear();
        std::stable_sort(live.begin(), live.end(), [](const Live& a, const Live& b) {
            if (a.pass != b.pass) return a.pass < b.pass;
            if (a.pass == Pass::Alpha) return a.depth > b.depth;
            if (a.pass == Pass::Opaque) return a.depth < b.depth;
            return false;
        });
        std::vector<Event> out;
        for (const auto& item : live) {
            const bool opaque = item.pass == Pass::Opaque;
            const bool ui = item.pass == Pass::UI;
            const std::string tex = item.obj->texture ? item.obj->texture->resource->name : "";
            out.push_back({item.obj->name, !ui, opaque, !opaque, tex});
        }
        return out;
    }
};
int main() {
    Transform t; t.x = 1; t.z = 3; t.scale = 2;
    t.yaw = std::acos(-1.0) / 2;
    assert(std::abs(t.WorldX(1, 0, 10) - 11) < 1e-9);
    assert(std::abs(t.WorldZ(1, 0) - 1) < 1e-9);
    auto asset = std::make_shared<Texture>("smoke.dds");
    TextureComponent prototype(asset);
    auto a = std::make_shared<Object>("alpha-near");
    auto b = std::make_shared<Object>("alpha-far");
    a->texture = prototype.Clone(); b->texture = prototype.Clone();
    assert(a->texture.get() != b->texture.get());
    assert(a->texture->resource == b->texture->resource);
    auto opaque = std::make_shared<Object>("opaque");
    auto ui = std::make_shared<Object>("hud");
    Queue q; q.Add(a, Pass::Alpha, 3); q.Add(b, Pass::Alpha, 10);
    q.Add(ui, Pass::UI, 0); q.Add(opaque, Pass::Opaque, 2);
    auto removed = std::make_shared<Object>("removed");
    q.Add(removed, Pass::Opaque, 1); removed.reset();
    const auto out = q.Flush();
    assert(out.size() == 4);
    assert(out[0].object == "opaque");
    assert(out[1].object == "alpha-far" && out[2].object == "alpha-near");
    assert(out[3].object == "hud");
    assert(out[0].ztest && out[0].zwrite && !out[0].blend);
    assert(out[1].ztest && !out[1].zwrite && out[1].blend);
    assert(!out[3].ztest && !out[3].zwrite && out[3].blend);
    assert(out[1].texture == "smoke.dds");
    assert(q.jobs.empty() && q.Flush().empty());
    std::cout << "PASS: row transform, separate instances/shared texture, "
                 "expired queue item, pass order and state\n";
}
```
GCC C++20·GCC C++11·Clang ASan/UBSan 배치에서 위 성공 메시지와 종료 코드 0을 확인했습니다. 이 미니는 독립적인 GPU 검사 또는 별도의 렌더링 엔진이 아닙니다.
