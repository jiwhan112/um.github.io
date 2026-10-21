#include "check.hpp"
#include <memory>
#include <string>
#include <vector>
struct State { std::vector<int> inventory; std::string scene; };
class GameState {
    std::unique_ptr<State> state_ = std::make_unique<State>(State{{1,2},"old"});
public:
    const State& Get() const noexcept { return *state_; }
    void Transition(std::string scene, bool fail_validation = false) {
        auto candidate = std::make_unique<State>(*state_);
        candidate->inventory.push_back(3);
        candidate->scene = std::move(scene);
        if (fail_validation || candidate->scene.empty()) throw std::runtime_error("validation");
        state_.swap(candidate); // Commit only owned state; no external I/O is performed.
    }
};
struct Resource { inline static int alive{}; Resource() { ++alive; } ~Resource() { --alive; } };
int FailPriority() { throw std::runtime_error("priority"); }
void Consume(std::unique_ptr<Resource>, int) {}
int main() { return Run("transaction", [](Checks& c) {
    GameState game;
    c.Throws<std::runtime_error>([&] { game.Transition("new", true); }, "late failure");
    c.Expect(game.Get().scene == "old" && game.Get().inventory == std::vector<int>({1,2}), "strong guarantee");
    game.Transition("new"); c.Expect(game.Get().scene == "new" && game.Get().inventory.size() == 3, "commit");
    c.Throws<std::runtime_error>([] { Consume(std::make_unique<Resource>(), FailPriority()); }, "argument failure");
    c.Expect(Resource::alive == 0, "RAII despite argument failure");
    auto owner = std::make_shared<Resource>(); std::weak_ptr<Resource> observer = owner;
    c.Expect(!observer.expired() && observer.lock() != nullptr, "weak lock");
    owner.reset(); c.Expect(observer.expired() && !observer.lock(), "weak not owning");
    c.Expect(Resource::alive == 0, "reclaimed");
}); }
