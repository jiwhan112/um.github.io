#ifndef STUDY_LOOP_HPP
#define STUDY_LOOP_HPP
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
namespace study {
// [한국어 해설] held는 현재 눌림, pressed는 아직 소비하지 않은 눌림 에지다. 짧게 눌렀다 떼어도 에지는 남긴다.
class Input {
    std::array<bool,256> held_{}, pressed_{};
public:
    void Key(std::size_t code,bool down) {
        if (code>=held_.size()) return;
        if (down&&!held_[code]) pressed_[code]=true;
        held_[code]=down;
    }
    bool Held(std::size_t code) const { return code<held_.size()&&held_[code]; }
// [한국어 해설] 눌림 에지는 Update에서 한 번만 소비한다. 키 반복 메시지를 여러 번의 새 입력으로 세지 않는다.
    bool ConsumePressed(std::size_t code) {
        if (code>=pressed_.size()) return false;
        const bool result=pressed_[code]; pressed_[code]=false; return result;
    }
// [한국어 해설] 포커스를 잃으면 입력을 비워 키가 계속 눌린 것처럼 남는 문제를 막는다.
    void Clear() noexcept { held_.fill(false); pressed_.fill(false); }
};
// [한국어 해설] steps는 수행 횟수, alpha는 다음 단계까지 남은 비율, discardedSeconds는 정책상 버린 시간이다.
struct StepResult { int steps{}; double alpha{}, discardedSeconds{}; };
// [한국어 해설] 120Hz 고정 Update 모델이다. 화면 주사율을 보장하지 않으며 밀린 시간을 일부 버리는 정책을 쓴다.
class FixedStepper {
    double accumulator_{};
public:
    static constexpr double step=1.0/120.0;
    static constexpr double maxFrame=0.25;
    static constexpr int maxSteps=8;
    void Reset() noexcept { accumulator_=0; }
// [한국어 해설] 경과 시간을 최대 0.25초로 제한하고 한 호출에서 최대 8단계까지만 따라잡는다.
    template<class Update> StepResult Advance(double elapsed,Update&& update) {
        if (!std::isfinite(elapsed)||elapsed<0) throw std::invalid_argument("invalid elapsed time");
        StepResult result;
        const double admitted=std::min(elapsed,maxFrame);
        result.discardedSeconds=elapsed-admitted;
        accumulator_+=admitted;
// [한국어 해설] update 성공 뒤 누적 시간을 줄인다. 콜백 예외에 대한 트랜잭션 보장은 별도로 구현하지 않는다.
        while (accumulator_+1e-12>=step&&result.steps<maxSteps) {
            update(step);
            accumulator_=std::max(0.0,accumulator_-step);
            ++result.steps;
        }
// [한국어 해설] 예산 이후 남은 완전한 단계는 버리고 소수 단계만 남겨 무한 따라잡기를 피한다.
        if (accumulator_>=step) {
            const double remainder=std::fmod(accumulator_,step);
            result.discardedSeconds+=accumulator_-remainder;
            accumulator_=remainder;
        }
        result.alpha=accumulator_/step;
        return result;
    }
};
// [한국어 해설] 애니메이션 프레임 수와 간격을 검증하고, 지나간 여러 프레임과 남은 시간을 함께 반영한다.
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
// [한국어 해설] 전체 단계 수를 프레임 수로 나눈 나머지만 인덱스에 더해 반복 애니메이션을 만든다.
        const int offset=static_cast<int>(std::fmod(steps,static_cast<double>(frames_)));
        index_=static_cast<int>((static_cast<long long>(index_)+offset)%frames_);
    }
    int Index() const noexcept { return index_; }
};
}
#endif
