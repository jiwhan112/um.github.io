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
