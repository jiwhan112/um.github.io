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
struct Transform { // [한국어 해설] 행 벡터에서 자식 Local*부모 World 순서로 결합한다. 행렬 저장 규약을 일관되게 유지한다.
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
struct TextureComponent : Component { // [한국어 해설] 컴포넌트 인스턴스는 독립 복제하지만 CPU 텍스처 자산 핸들은 공유한다. COM AddRef 모형은 아니다.
    explicit TextureComponent(std::shared_ptr<TextureAsset> p):asset(std::move(p)) {
        if(!asset) throw std::invalid_argument("null texture");
    }
    std::shared_ptr<TextureAsset> asset; // CPU handle only, not a COM reference.
    std::unique_ptr<Component> Clone() const override {
        return std::unique_ptr<Component>(new TextureComponent(asset));
    }
};
struct ComponentCatalog { // [한국어 해설] 프로토타입의 고유 소유권을 보관하고 Clone으로 새 컴포넌트를 생성한다. 중복 키는 거부한다.
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
struct RenderQueue { // [한국어 해설] 큐는 weak_ptr로 빌려 두고 Flush에서 살아 있는 항목만 잠시 소유한다.
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
        std::stable_sort(live.begin(),live.end(),[](const Live& a,const Live& b){ // [한국어 해설] 불투명은 앞→뒤, 알파는 뒤→앞, UI는 등록 순서다. 모든 투명도 문제를 해결하는 보편 정렬은 아니다.
            if(a.group!=b.group) return static_cast<int>(a.group)<static_cast<int>(b.group);
            if(a.group==Group::Alpha) return a.depth>b.depth; // back-to-front demo
            if(a.group==Group::Opaque) return a.depth<b.depth; // front-to-back demo
            return a.order<b.order;
        });
        std::vector<RenderEvent> events;
        for(const auto& e:live) {
            state=StateFor(e.group);  // initialize critical states on EACH draw/pass // [한국어 해설] 매 draw의 핵심 상태를 초기화하여 이전 패스의 텍스처·깊이·블렌드 상태 누출을 막는다.
            if(auto tex=e.object->Texture()) state.boundTexture=tex->name;
            events.push_back({e.object->name,e.group,e.depth,state});
        }
        state=RenderState{}; // explicit invariant for next frame in this model
        return events;
    }
};

int main() { // [한국어 해설] 변환, 독립 인스턴스/공유 자산, 만료된 큐, 순서·상태 및 소유권 해제를 CPU에서 검사한다.
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
