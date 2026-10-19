#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace study {
using Id = std::uint64_t;
struct Handle { std::uint64_t generation; Id id; };
class Engine;
class World;

struct FrameContext {
    World& world;
    Engine& engine;
};

class Actor {
public:
    virtual ~Actor() {}
    Id id() const { return id_; }
    virtual void FixedTick(FrameContext&, double) {}
    virtual void LateTick(FrameContext&) {}
    virtual void Render(std::vector<std::string>&) const {}
private:
    friend class World;
    Id id_ = 0;
};

class World {
public:
    explicit World(std::string name) : name_(std::move(name)) {}
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    Id Spawn(std::string layer, std::unique_ptr<Actor> object) {
        if (!object) throw std::invalid_argument("null actor");
        if (object->id_ != 0) throw std::logic_error("actor already owned");
        const Id id = nextId_++;
        object->id_ = id;
        pendingAdds_.push_back({std::move(layer), std::move(object)});
        return id;
    }
    void Destroy(Id id) { pendingRemoves_.insert(id); }
    void QueueDraw(Id id) {
        if (Find(id) && !pendingRemoves_.count(id)) drawQueue_.push_back({generation_, id});
    }
    Actor* Find(Id id) {
        auto it = objects_.find(id);
        return it == objects_.end() ? nullptr : it->second.get();
    }
    const Actor* Resolve(Handle h) const {
        if (h.generation != generation_) return nullptr;
        auto it = objects_.find(h.id);
        return it == objects_.end() ? nullptr : it->second.get();
    }
    Handle MakeHandle(Id id) const { return {generation_, id}; }
    std::size_t Size() const { return objects_.size(); }
    const std::string& Name() const { return name_; }
    std::uint64_t Generation() const { return generation_; }
    void Commit() {
        for (Id id : pendingRemoves_) {
            objects_.erase(id);
            for (auto& layer : layers_) {
                auto& ids = layer.second;
                ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
            }
        }
        for (auto& pending : pendingAdds_) {
            const Id id = pending.object->id_;
            if (pendingRemoves_.count(id)) continue; // Spawn + Destroy in same step.
            if (!layers_.count(pending.layer)) layerOrder_.push_back(pending.layer);
            layers_[pending.layer].push_back(id);
            objects_.emplace(id, std::move(pending.object));
        }
        pendingAdds_.clear();
        pendingRemoves_.clear();
    }
    void FixedStep(Engine& engine, double dt);
    std::size_t LateAndDraw(Engine& engine, std::vector<std::string>& events);
private:
    friend class Engine;
    struct PendingAdd { std::string layer; std::unique_ptr<Actor> object; };
    void SetGeneration(std::uint64_t gen) { generation_ = gen; Commit(); }
    std::string name_;
    std::uint64_t generation_ = 0;
    Id nextId_ = 1;
    std::map<Id, std::unique_ptr<Actor>> objects_;
    std::map<std::string, std::vector<Id>> layers_;
    std::vector<std::string> layerOrder_;
    std::vector<PendingAdd> pendingAdds_;
    std::set<Id> pendingRemoves_;
    std::vector<Handle> drawQueue_;
};

struct FrameStats {
    unsigned fixedUpdates = 0;
    std::size_t rendered = 0;
    bool sceneChanged = false;
    bool backlogDropped = false;
    double alpha = 0.0;
};

class Engine {
public:
    static constexpr double FixedDt() { return 1.0 / 60.0; }
    void OpenScene(std::unique_ptr<World> next) {
        if (!next) throw std::invalid_argument("null scene");
        pending_ = std::move(next);
    }
    bool HasPendingScene() const { return static_cast<bool>(pending_); }
    const World* Current() const { return active_.get(); }
    World* Current() { return active_.get(); }
    void ApplyPending() {
        if (!pending_) return;
        pending_->SetGeneration(++generation_);
        active_ = std::move(pending_); // Releases previous world and its actors.
        accumulator_ = 0.0;              // Do not mix simulation time across scenes.
    }
    FrameStats RunFrame(double measuredDt, std::vector<std::string>& events) {
        if (!std::isfinite(measuredDt) || measuredDt < 0.0)
            throw std::invalid_argument("invalid frame delta");
        FrameStats result;
        if (pending_) { ApplyPending(); result.sceneChanged = true; }
        if (!active_) return result;
        accumulator_ += std::min(measuredDt, 0.25);  // Stalls must not generate unlimited ticks.
        const double step = FixedDt();
        while (accumulator_ + 1e-12 >= step && result.fixedUpdates < 5) {
            active_->FixedStep(*this, step);
            ++result.fixedUpdates;
            accumulator_ = std::max(0.0, accumulator_ - step); // v1.1: epsilon must not yield negative alpha
            if (pending_) {
                ApplyPending();
                result.sceneChanged = true;
                result.alpha = 0.0;
                return result; // Old scene must not LateTick/Render this frame.
            }
        }
        if (accumulator_ >= step) {
            accumulator_ = std::fmod(accumulator_, step);  // Drop excess backlog.
            result.backlogDropped = true;
        }
        result.rendered = active_->LateAndDraw(*this, events);
        if (pending_) {
            ApplyPending();
            result.sceneChanged = true;
            result.rendered = 0;
            result.alpha = 0.0;
            return result;
        }
        result.alpha = accumulator_ / step;
        return result;
    }
private:
    std::unique_ptr<World> active_;
    std::unique_ptr<World> pending_;
    std::uint64_t generation_ = 0;
    double accumulator_ = 0.0;
};

inline void World::FixedStep(Engine& engine, double dt) {
    FrameContext ctx{*this, engine};
    for (const auto& layer : layerOrder_) {
        const auto& ids = layers_.at(layer);
        for (Id id : ids) {
            if (pendingRemoves_.count(id)) continue;
            Actor* actor = Find(id);
            if (actor) actor->FixedTick(ctx, dt);
            if (engine.HasPendingScene()) return;
        }
    }
    Commit();  // The new actors participate in the NEXT fixed step.
}

inline std::size_t World::LateAndDraw(Engine& engine, std::vector<std::string>& events) {
    FrameContext ctx{*this, engine};
    drawQueue_.clear();
    for (const auto& layer : layerOrder_) {
        const auto& ids = layers_.at(layer);
        for (Id id : ids) {
            if (pendingRemoves_.count(id)) continue;
            Actor* actor = Find(id);
            if (actor) actor->LateTick(ctx);
            if (engine.HasPendingScene()) { drawQueue_.clear(); return 0; }
        }
    }
    Commit();  // Destroy between LateTick and Render invalidates queued handles.
    std::size_t drawn = 0;
    for (Handle h : drawQueue_) {
        const Actor* actor = Resolve(h);
        if (actor) { actor->Render(events); ++drawn; }
    }
    drawQueue_.clear();
    return drawn;
}
}  // namespace study
