#include <limits>
#include "loop.hpp"
#include "check.hpp"
using namespace study;
// [한국어 해설] 렌더 빈도를 30/60/144로 바꿔도 보통 1초 동안 120 Update와 같은 이동량을 얻는지 검사한다.
int main() {
    for (int fps:{30,60,144}) {
        FixedStepper stepper; int count{}; double position{};
        for (int frame=0;frame<fps;++frame) {
            const auto result=stepper.Advance(1.0/fps,[&](double dt){++count;position+=120*dt;});
            Check(result.alpha>=0&&result.alpha<1.00000001,"alpha range");
            Check(Near(result.discardedSeconds,0),"ordinary frame not dropped");
        }
        Check(count==120&&Near(position,120),"same second different render rate");
    }
// [한국어 해설] 긴 정지 이후 최대 따라잡기 횟수와 버리는 시간, 초기화 및 잘못된 시간 입력을 확인한다.
    FixedStepper stepper;
    const auto stalled=stepper.Advance(1.0,[](double){});
    Check(stalled.steps==8&&stalled.discardedSeconds>.9,"catch-up cap and discarded time");
    stepper.Reset(); Check(stepper.Advance(0,[](double){}).steps==0,"pause reset");
    Throws([&]{ (void)stepper.Advance(-1,[](double){}); });
    Throws([&]{ (void)stepper.Advance(std::numeric_limits<double>::quiet_NaN(),[](double){}); });
// [한국어 해설] 키 반복·짧은 탭·포커스 상실을 분리해 상태와 에지가 서로 다른 개념임을 검사한다.
    Input input; input.Key(32,true); input.Key(32,true);
    Check(input.Held(32)&&input.ConsumePressed(32),"press edge");
    Check(!input.ConsumePressed(32),"edge consumed once");
    input.Key(32,false); input.Key(32,true); input.Key(32,false);
    Check(!input.Held(32)&&input.ConsumePressed(32),"short tap preserved");
    input.Key(32,true); input.Clear(); Check(!input.Held(32)&&!input.ConsumePressed(32),"focus loss clears input");
    input.Key(999,true); Check(!input.Held(999),"out of range key ignored");
    AnimationClock animation(4,.1); animation.Advance(.25);
    Check(animation.Index()==2,"animation handles multiple frames");
    animation.Advance(.05); Check(animation.Index()==3,"animation keeps remainder");
    animation.Advance(.2); Check(animation.Index()==1,"animation wraps");
    Throws([]{ AnimationClock bad(0,.1); });
    return Done("loop_input");
}
