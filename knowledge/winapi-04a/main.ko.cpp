// WinAPI04A CPU-only fixed timestep model. C++11/C++20 compatible.
// No Windows headers or operating-system GUI calls are used.
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

// [한국어 해설] 수행 단계 수, 버린 시간, 남은 시간과 보간 비율을 함께 반환한다. 시간 단위는 정수 나노초다.
struct TickResult {
    std::int64_t updates;
    std::int64_t dropped_ns;
    std::int64_t remaining_ns;
    double alpha;
};

// [한국어 해설] 고정 간격과 프레임 제한, 최대 수행 횟수의 불변식을 생성자에서 검사한다.
class FixedStepper {
public:
    FixedStepper(std::int64_t step_ns, std::int64_t max_frame_ns,
                 std::int64_t max_updates)
        : step_ns_(step_ns), max_frame_ns_(max_frame_ns),
          max_updates_(max_updates), pending_ns_(0) {
        if (step_ns <= 0 || max_frame_ns < step_ns || max_updates <= 0 ||
            step_ns > std::numeric_limits<std::int64_t>::max() - max_frame_ns) {
            throw std::invalid_argument("invalid timestep configuration");
        }
    }

// [한국어 해설] 음수 경과 시간은 거부하고 프레임 제한과 Update 예산을 각각 적용한다.
    TickResult tick(std::int64_t measured_ns) {
        if (measured_ns < 0) {
            throw std::invalid_argument("negative elapsed time");
        }
        const std::int64_t accepted_ns =
            std::min(measured_ns, max_frame_ns_);
        // pending_ns_ is always [0, step_ns_); constructor prevents overflow.
// [한국어 해설] 남은 시간은 항상 한 단계 미만이다. 생성자 검사가 덧셈 오버플로를 방지한다.
        const std::int64_t available_ns = pending_ns_ + accepted_ns;
        const std::int64_t available_updates = available_ns / step_ns_;
        const std::int64_t updates = std::min(available_updates, max_updates_);
        const std::int64_t after_updates = available_ns - updates * step_ns_;
// [한국어 해설] 예산 후 남은 완전한 단계는 버리고 한 단계 미만의 잔여만 보존한다.
        const std::int64_t dropped_backlog =
            (after_updates / step_ns_) * step_ns_;
        pending_ns_ = after_updates - dropped_backlog;
// [한국어 해설] 프레임 입력 제한으로 버린 시간과 처리 예산으로 버린 시간을 합산한다.
        const std::int64_t dropped_ns =
            (measured_ns - accepted_ns) + dropped_backlog;
        return TickResult{updates, dropped_ns, pending_ns_,
                          static_cast<double>(pending_ns_) /
                              static_cast<double>(step_ns_)};
    }

private:
    std::int64_t step_ns_;
    std::int64_t max_frame_ns_;
    std::int64_t max_updates_;
    std::int64_t pending_ns_;
};

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// [한국어 해설] 잘못된 설정과 경과 시간이 invalid_argument로 거부되는지 검사한다.
template <typename Fn>
void must_reject(Fn fn, const char* message) {
    bool rejected = false;
    try {
        fn();
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, message);
}
}

// [한국어 해설] CPU 전용 모델이다. Windows 메시지 처리, QPC, GDI 또는 GPU 실행을 검사하지 않는다.
int main() {
    try {
        must_reject([] { FixedStepper s(0, 30, 3); (void)s; }, "zero step");
        must_reject([] { FixedStepper s(10, 9, 3); (void)s; }, "short clamp");
        must_reject([] { FixedStepper s(10, 30, 0); (void)s; }, "zero budget");
        must_reject([] {
            FixedStepper s(10, std::numeric_limits<std::int64_t>::max(), 3);
            (void)s;
        }, "overflow configuration");

// [한국어 해설] 부분 단계의 누적과 잔여 보존을 작은 정수로 확인한다.
        FixedStepper s(10, 30, 3);
        must_reject([&s] { (void)s.tick(-1); }, "negative delta");
        TickResult r = s.tick(0);
        check(r.updates == 0 && r.remaining_ns == 0, "zero delta");
        r = s.tick(6);
        check(r.updates == 0, "partial step");
        check(r.remaining_ns == 6, "partial preserved");
        check(r.alpha == 0.6, "interpolation ratio");
        r = s.tick(7);
        check(r.updates == 1, "accumulated full step");
        check(r.remaining_ns == 3, "remainder preserved");
        check(r.dropped_ns == 0, "no skipped time");
        r = s.tick(30);
        check(r.updates == 3, "update budget exactly used");
        check(r.remaining_ns == 3, "fraction remains after budget");
        check(r.dropped_ns == 0, "no full backlog dropped");

// [한국어 해설] 55 중 25는 입력 제한, 10은 처리 예산으로 버려 총 35가 되어야 한다.
        FixedStepper limited(10, 30, 2);
        r = limited.tick(55);
        check(r.updates == 2, "max updates");
        check(r.dropped_ns == 35, "25ns clamp plus 10ns backlog");
        check(r.remaining_ns == 0, "discard full backlog");
        r = limited.tick(5);
        check(r.updates == 0 && r.remaining_ns == 5, "resume after clamp");
        r = limited.tick(5);
        check(r.updates == 1 && r.remaining_ns == 0, "next update");

// [한국어 해설] 최대 정수 경과 시간에서도 부호 있는 정수 오버플로 없이 버린 시간을 계산하는지 확인한다.
        FixedStepper near_limit(10, 30, 2);
        r = near_limit.tick(std::numeric_limits<std::int64_t>::max());
        check(r.updates == 2, "huge delta budget");
        check(r.dropped_ns == std::numeric_limits<std::int64_t>::max() - 20,
              "huge delta no signed overflow");
        check(r.alpha == 0.0, "huge delta remainder");
        std::cout << "PASS " << checks
                  << " fixed-step CPU contract checks (C++11/20)\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
