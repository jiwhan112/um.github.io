#ifndef STUDY_LOOP_HPP
#define STUDY_LOOP_HPP
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
namespace study {
class Input {
    std::array<bool,256> held_{}, pressed_{};
public:
    void Key(std::size_t code,bool down) {
        if (code>=held_.size()) return;
        if (down&&!held_[code]) pressed_[code]=true;
        held_[code]=down;
    }
    bool Held(std::size_t code) const { return code<held_.size()&&held_[code]; }
    bool ConsumePressed(std::size_t code) {
        if (code>=pressed_.size()) return false;
        const bool result=pressed_[code]; pressed_[code]=false; return result;
    }
    void Clear() noexcept { held_.fill(false); pressed_.fill(false); }
};
struct StepResult { int steps{}; double alpha{}, discardedSeconds{}; };
class FixedStepper {
    double accumulator_{};
public:
    static constexpr double step=1.0/120.0;
    static constexpr double maxFrame=0.25;
    static constexpr int maxSteps=8;
    void Reset() noexcept { accumulator_=0; }
    template<class Update> StepResult Advance(double elapsed,Update&& update) {
        if (!std::isfinite(elapsed)||elapsed<0) throw std::invalid_argument("invalid elapsed time");
        StepResult result;
        const double admitted=std::min(elapsed,maxFrame);
        result.discardedSeconds=elapsed-admitted;
        accumulator_+=admitted;
        while (accumulator_+1e-12>=step&&result.steps<maxSteps) {
            update(step);
            accumulator_=std::max(0.0,accumulator_-step);
            ++result.steps;
        }
        if (accumulator_>=step) {
            const double remainder=std::fmod(accumulator_,step);
            result.discardedSeconds+=accumulator_-remainder;
            accumulator_=remainder;
        }
        result.alpha=accumulator_/step;
        return result;
    }
};
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
        const int offset=static_cast<int>(std::fmod(steps,static_cast<double>(frames_)));
        index_=static_cast<int>((static_cast<long long>(index_)+offset)%frames_);
    }
    int Index() const noexcept { return index_; }
};
}
#endif
