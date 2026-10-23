#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Standalone C++11 CPU model. No Direct3D calls; shared_ptr is NOT COM AddRef.
struct Texture { explicit Texture(const std::string& n) : name(n) {} std::string name; };
struct TextureComponent { // [한국어 해설] 간단한 원문 CPU 예제다. Clone은 컴포넌트를 새로 만들고 내부 자산 shared_ptr만 공유한다.
    explicit TextureComponent(std::shared_ptr<Texture> p) : resource(p) {}
    std::shared_ptr<Texture> resource;
    std::unique_ptr<TextureComponent> Clone() const {
        return std::unique_ptr<TextureComponent>(new TextureComponent(resource));
    }
};
struct Transform { // [한국어 해설] 행 벡터·왼손 +Z 전방이며 이 축약 예제의 부모 변환은 이동만 지원한다.
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
struct Queue { // [한국어 해설] weak_ptr 등록으로 큐가 게임 객체 수명을 불필요하게 늘리지 않는다.
    std::vector<Job> jobs;
    void Add(const std::shared_ptr<Object>& obj, Pass pass, double depth) {
        jobs.push_back({obj, pass, depth});
    }
    std::vector<Event> Flush() { // [한국어 해설] 유효 객체만 모아 패스별 순서를 정하고 이번 프레임 등록을 비운다.
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
int main() { // [한국어 해설] 원문 사례를 별도 실행한다. 더 상세한 실패·계약 검사는 renderer_components.cpp에 있다.
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
