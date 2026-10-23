#include "scene_loop.hpp"
#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
using namespace study;

class Probe : public Actor { // [한국어 해설] 테스트 배우는 갱신·렌더 이벤트와 파괴 횟수를 관찰한다.
public:
    Probe(std::string label, int* destruct = nullptr)
        : name_(std::move(label)), destruct_(destruct) {}
    ~Probe() override { if (destruct_) ++*destruct_; }
    void FixedTick(FrameContext&, double) override { ++ticks_; }
    void LateTick(FrameContext& context) override { context.world.QueueDraw(id()); }
    void Render(std::vector<std::string>& events) const override { events.push_back(name_); }
    unsigned Ticks() const { return ticks_; }
private:
    std::string name_;
    int* destruct_;
    unsigned ticks_ = 0;
};

class SpawnAndRemove : public Actor { // [한국어 해설] 갱신 중 자식을 생성하고 자신을 삭제하여 지연 반영 계약을 검사한다.
public:
    explicit SpawnAndRemove(Id* child) : child_(child) {}
    void FixedTick(FrameContext& c, double) override {
        *child_ = c.world.Spawn("actors", std::unique_ptr<Actor>(new Probe("child")));
        c.world.Destroy(id());
    }
    void LateTick(FrameContext& c) override { c.world.QueueDraw(id()); }
    void Render(std::vector<std::string>& events) const override { events.push_back("should-not-draw"); }
private: Id* child_;
};

class SpawnAndCancel : public Actor {
public:
    void FixedTick(FrameContext& c, double) override {
        Id x = c.world.Spawn("actors", std::unique_ptr<Actor>(new Probe("cancelled")));
        c.world.Destroy(x);
        c.world.Destroy(id());
    }
};

class SwitchScene : public Actor { // [한국어 해설] 고정 갱신 중 장면을 예약할 때 이전 장면의 LateTick/Render가 중단되는지 확인한다.
public:
    explicit SwitchScene(int* oldDestruct) : oldDestruct_(oldDestruct) {}
    ~SwitchScene() override { ++*oldDestruct_; }
    void FixedTick(FrameContext& c, double) override {
        auto next = std::unique_ptr<World>(new World("Game"));
        next->Spawn("actors", std::unique_ptr<Actor>(new Probe("new-scene")));
        c.engine.OpenScene(std::move(next));
    }
    void LateTick(FrameContext& c) override { c.world.QueueDraw(id()); }
    void Render(std::vector<std::string>& e) const override { e.push_back("old-scene"); }
private: int* oldDestruct_;
};

int main() { // [한국어 해설] __LINE__ 진단을 보존하려고 이 파일은 새 줄 대신 줄 끝 한국어 주석만 추가했다.
    unsigned tests = 0;
#define CHECK(expr) do { ++tests; if (!(expr)) { std::cerr << "FAILED: " << #expr << " line " << __LINE__ << "\n"; return 1; } } while (0)
    Engine engine;
    std::vector<std::string> events;
    auto world = std::unique_ptr<World>(new World("Logo"));
    Id child = 0;
    world->Spawn("actors", std::unique_ptr<Actor>(new SpawnAndRemove(&child)));
    world->Spawn("actors", std::unique_ptr<Actor>(new SpawnAndCancel));
    Id staticId = world->Spawn("actors", std::unique_ptr<Actor>(new Probe("static")));
    engine.OpenScene(std::move(world));
    FrameStats zero = engine.RunFrame(0.0, events);
    CHECK(zero.sceneChanged); CHECK(zero.fixedUpdates == 0); CHECK(zero.rendered == 2);
    CHECK(engine.Current()->Size() == 3);
    Handle oldStaticHandle = engine.Current()->MakeHandle(staticId);
    events.clear();
    FrameStats one = engine.RunFrame(Engine::FixedDt(), events);
    CHECK(one.fixedUpdates == 1); CHECK(one.rendered == 2);
    CHECK(engine.Current()->Size() == 2); CHECK(child != 0); CHECK(engine.Current()->Find(child) != nullptr);
    CHECK(events.size() == 2); CHECK(events[0] == "static"); CHECK(events[1] == "child");
    CHECK(engine.Current()->Resolve(oldStaticHandle) != nullptr);
    CHECK(static_cast<Probe*>(engine.Current()->Find(child))->Ticks() == 0); // New actor missed this FixedTick.
    events.clear();
    FrameStats half = engine.RunFrame(0.5 * Engine::FixedDt(), events);
    CHECK(half.fixedUpdates == 0); CHECK(half.rendered == 2); CHECK(half.alpha > 0 && half.alpha < 1);
    CHECK(static_cast<Probe*>(engine.Current()->Find(child))->Ticks() == 0);
    engine.RunFrame(0.5 * Engine::FixedDt(), events);
    CHECK(static_cast<Probe*>(engine.Current()->Find(child))->Ticks() == 1);
    FrameStats stall = engine.RunFrame(0.5, events);
    CHECK(stall.fixedUpdates == 5); CHECK(stall.backlogDropped);
    CHECK(stall.alpha >= 0 && stall.alpha < 1);
    CHECK(engine.Current()->Size() == 2);
    bool caught = false;
    try { engine.RunFrame(-0.01, events); } catch (const std::invalid_argument&) { caught = true; }
    CHECK(caught);
    caught = false;
    try { engine.RunFrame(std::numeric_limits<double>::quiet_NaN(), events); }
    catch (const std::invalid_argument&) { caught = true; }
    CHECK(caught);
    caught = false;
    try { engine.Current()->Spawn("bad", std::unique_ptr<Actor>()); }
    catch (const std::invalid_argument&) { caught = true; }
    CHECK(caught);
    int oldDestruct = 0;
    auto logo2 = std::unique_ptr<World>(new World("Logo2"));
    logo2->Spawn("actors", std::unique_ptr<Actor>(new SwitchScene(&oldDestruct)));
    engine.OpenScene(std::move(logo2));
    events.clear();
    FrameStats switched = engine.RunFrame(Engine::FixedDt(), events);
    CHECK(switched.sceneChanged); CHECK(switched.fixedUpdates == 1);
    CHECK(switched.rendered == 0); CHECK(events.empty()); CHECK(oldDestruct == 1);
    CHECK(engine.Current()->Name() == "Game"); CHECK(engine.Current()->Resolve(oldStaticHandle) == nullptr);
    CHECK(engine.Current()->Generation() != oldStaticHandle.generation);
    events.clear();
    FrameStats after = engine.RunFrame(Engine::FixedDt(), events);
    CHECK(!after.sceneChanged); CHECK(after.fixedUpdates == 1); CHECK(after.rendered == 1);
    CHECK(events.size() == 1 && events[0] == "new-scene");
    CHECK(engine.Current()->Size() == 1);
    Engine boundary;
    boundary.OpenScene(std::unique_ptr<World>(new World("Boundary")));
    std::vector<std::string> boundaryEvents;
    FrameStats tolerance = boundary.RunFrame(Engine::FixedDt() - 5e-13, boundaryEvents);
    CHECK(tolerance.fixedUpdates == 1);
    CHECK(tolerance.alpha == 0.0);
    FrameStats repeatedZero = boundary.RunFrame(0.0, boundaryEvents);
    CHECK(repeatedZero.fixedUpdates == 0 && repeatedZero.alpha == 0.0);
    std::cout << "PASS " << tests << " independent assertions\n";
}
