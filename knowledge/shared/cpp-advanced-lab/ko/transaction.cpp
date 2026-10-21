#include "check.hpp"
#include <memory>
#include <string>
#include <vector>

struct State {
    std::vector<int> inventory;
    std::string scene;
};

class GameState {
    std::unique_ptr<State> state_ = std::make_unique<State>(State{{1,2},"old"});
public:
    const State& Get() const noexcept { return *state_; }
    void Transition(std::string scene, bool fail_validation = false) {
        // 별도 후보에 변경을 준비한다. 실패 전에는 현재 소유 상태를 건드리지 않는다.
        auto candidate = std::make_unique<State>(*state_);
        candidate->inventory.push_back(3);
        candidate->scene = std::move(scene);
        if (fail_validation || candidate->scene.empty()) throw std::runtime_error("validation");
        // 준비·검증 뒤 noexcept인 소유권 교체를 한다. 이전 상태는 candidate가 정리한다.
        // 파일·네트워크 롤백이나 스레드 사이의 원자적 공개를 구현한 것은 아니다.
        state_.swap(candidate);
    }
};

struct Resource {
    inline static int alive{};
    Resource() { ++alive; }
    ~Resource() { --alive; }
};
int FailPriority() { throw std::runtime_error("priority"); }
void Consume(std::unique_ptr<Resource>, int) {}

int main() {
    return Run("transaction", [](Checks& c) {
        GameState game;
        c.Throws<std::runtime_error>([&] { game.Transition("new", true); }, "late failure");
        c.Expect(game.Get().scene == "old" && game.Get().inventory == std::vector<int>({1,2}), "strong guarantee");
        game.Transition("new");
        c.Expect(game.Get().scene == "new" && game.Get().inventory.size() == 3, "commit");

        // 인수 평가 순서는 고정하지 않는다. Resource가 생성되지 않을 수도 있다.
        // 먼저 생성되었다면 unique_ptr 임시 객체가 예외 전파 중 자원을 정리한다.
        c.Throws<std::runtime_error>([] { Consume(std::make_unique<Resource>(), FailPriority()); }, "argument failure");
        c.Expect(Resource::alive == 0, "RAII despite argument failure");

        auto owner = std::make_shared<Resource>();
        std::weak_ptr<Resource> observer = owner;
        // weak_ptr 자체는 소유 수를 늘리지 않는다. lock으로 얻은 shared_ptr은 소유한다.
        c.Expect(!observer.expired() && observer.lock() != nullptr, "weak lock");
        owner.reset();
        c.Expect(observer.expired() && !observer.lock(), "weak not owning");
        c.Expect(Resource::alive == 0, "reclaimed");
    });
}
