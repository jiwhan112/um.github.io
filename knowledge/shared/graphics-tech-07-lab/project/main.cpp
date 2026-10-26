#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
    int checks = 0;
    void Check(bool ok, const char* why) {
        ++checks;
        if (!ok) throw std::runtime_error(why);
        // Release에서도 검사가 사라지지 않는다.
    }
    bool Near(double a, double b, double e=1e-9) {
        return std::abs(a-b) <= e;
    }
    void Require(bool ok, const char* why) {
        if (!ok) throw std::invalid_argument(why);
    }
    void Finite(double x) {
        Require(std::isfinite(x), "finite input required");
    }
    template<class F> void Reject(F fn) {
        bool rejected=false;
        try {
            fn();
        } catch(const std::invalid_argument&) {
            rejected=true;
        }
        Check(rejected, "invalid input was not rejected");
    }
    constexpr double PI=3.14159265358979323846;
    struct History {
        double u,v,depth,color;
        bool cameraCut;
    };
    struct Result {
        double value;
        bool reused;
    };
    // expectedPreviousDepth와 h.depth는 동일한 이전 view 공간이어야 한다.
    Result Resolve(double current,const std::array<double,3>& neighbors,double expectedPreviousDepth, History h,double weight,double depthTolerance) {
        for(double x: {
            current,expectedPreviousDepth,h.u,h.v,h.depth,h.color,weight,depthTolerance
        })Finite(x);
        for(double x:neighbors)Finite(x);
        Require(expectedPreviousDepth>=0 && h.depth>=0, "nonnegative view depths");
        Require(weight>=0&&weight<=1&&depthTolerance>=0,"weight/tolerance");
        if(h.cameraCut||h.u<0||h.u>=1||h.v<0||h.v>=1||std::abs(h.depth-expectedPreviousDepth)>depthTolerance) return {
            current,false
        };
        const auto bounds=std::minmax_element(neighbors.begin(),neighbors.end());
        const double clipped=std::clamp(h.color,*bounds.first,*bounds.second);
        return {
            current*(1-weight)+clipped*weight,true
        };
    }
    // motion 단위는 pixel이며 history 조회 전에 UV로 정규화한다.
    double PreviousUV(double currentUV,double motionPixels,int size) {
        Require(size>0,"positive image size");
        Finite(currentUV);
        Finite(motionPixels);
        // motionPixels = previousPosition - currentPosition, 둘 다 같은 비지터 pixel 좌표.
        return currentUV+motionPixels/size;
    }
    // 정상 누적, clip, disocclusion, camera cut, 화면 밖과 단위를 검사한다.
    void Run() {
        History h {
            0.5,0.5,10,0.6,false
        };
        std::array<double,3> neighborhood {
            0.4,0.5,0.6
        };
        auto r=Resolve(0.5,neighborhood,10,h,0.9,0.01);
        Check(r.reused&&Near(r.value,0.59),"valid history");
        h.color=100;
        Check(Near(Resolve(0.5,neighborhood,10,h,0.9,0.01).value,0.59),"bright history clipped");
        h.depth=9;
        Check(!Resolve(0.5,neighborhood,10,h,0.9,0.01).reused,"disocclusion rejected");
        h.depth=10;
        h.cameraCut=true;
        Check(!Resolve(0.5,neighborhood,10,h,0.9,0.01).reused,"camera cut reset");
        h.cameraCut=false;
        h.u=1;
        Check(!Resolve(0.5,neighborhood,10,h,0.9,0.01).reused,"right edge outside");
        h.u=-0.01;
        Check(!Resolve(0.5,neighborhood,10,h,0.9,0.01).reused,"left edge outside");
        h.u=0.5;
        Check(Near(Resolve(0.5,neighborhood,10,h,0,0.01).value,0.5),"zero weight");
        Check(Near(PreviousUV(0.5,-4,100),0.46),"motion direction and units");
        Check(Near(PreviousUV(0.5,4,200),0.52),"resolution matters");
        Reject([&] {
            Resolve(0.5,neighborhood,10,h,1.1,0.01);
        });
        Reject([] {
            PreviousUV(0,1,0);
        });
        std::cout<<"accepted history="<<r.value<<"; camera cuts and disocclusions reset\n";
    }
}
// namespace
int main() {
    try {
        Run();
        std::cout<<"PASS: "<<checks<<" contract checks\n";
        return 0;
    }catch(const std::exception& e) {
        std::cerr<<"FAIL: "<<e.what()<<"\n";
        return 1;
    }
}
