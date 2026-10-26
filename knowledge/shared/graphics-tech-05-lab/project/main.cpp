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
    // near/far는 양수 view-space 거리다. normalized depth 값이 아니다.
    std::vector<double> Splits(double nearZ,double farZ,int count,double lambda) {
        for(double x: {
            nearZ,farZ,lambda
        })Finite(x);
        Require(nearZ>=0.0001&&farZ<=1000000&&farZ>nearZ&&count>=1&&count<=16&&lambda>=0&&lambda<=1,"split contract");
        std::vector<double> out;
        out.push_back(nearZ);
        for(int i=1;i<=count;++i) {
            double t=static_cast<double>(i)/count;
            out.push_back((1-lambda)*(nearZ+(farZ-nearZ)*t)+lambda*nearZ*std::pow(farZ/nearZ,t));
        }
        out.back()=farZ;
        return out;
    }
    // 구간 끝과 같은 값은 더 가까운 cascade에 배정한다.
    int Cascade(const std::vector<double>& split,double z) {
        if(z<split.front()||z>split.back())return -1;
        for(std::size_t i=1;i<split.size();++i)if(z<=split[i])return static_cast<int>(i)-1;
        return -1;
    }
    // extent는 반폭이다. 크기를 바꾸면 격자 간격도 바뀌므로 이 실습에서는 고정한다.
    double Snap(double center,double extent,int resolution) {
        for(double x: {
            center,extent
        })Finite(x);
        // 단위가 제한된 교육 모델: 큰 유한값의 곱셈 overflow도 입력 단계에서 거절한다.
        Require(std::abs(center)<=1000000 && extent>=0.0001 && extent<=1000000,
                "center/extent bounded world domain");
        Require(resolution>=1 && resolution<=16384, "resolution in [1,16384]");
        const double texel=2*extent/resolution;
        return std::floor(center/texel+0.5)*texel;
        // 가장 가까운 texel. 동률은 +방향.
    }
    // 선형/로그 끝값, 단조성, 경계 선택과 반 texel 오차 한계를 확인한다.
    void Run() {
        auto linear=Splits(1,101,4,0),log=Splits(1,81,4,1),mixed=Splits(1,101,4,0.5);
        Check(Near(linear[1],26)&&Near(linear[4],101),"linear partition");
        Check(Near(log[1],3)&&Near(log[2],9)&&Near(log[3],27),"log partition");
        for(std::size_t i=1;i<mixed.size();++i)Check(mixed[i]>mixed[i-1],"monotonic split");
        Check(Cascade(linear,26)==0,"split equality belongs to nearer cascade");
        Check(Cascade(linear,26.001)==1,"cross split");
        Check(Cascade(linear,0)==-1&&Cascade(linear,102)==-1,"out of range");
        Check(Near(Snap(0.01,10,100),0)&&Near(Snap(0.09,10,100),0),"subtexel stable");
        Check(Near(Snap(0.11,10,100),0.2),"one texel step");
        Check(Near(Snap(-0.11,10,100),-0.2),"negative coordinates");
        for(int i=-100;i<=100;++i) {
            double c=i*0.013;
            Check(std::abs(Snap(c,10,100)-c)<=0.10000001,"bounded snap displacement");
        }
        Reject([] {
            Splits(0,100,4,0.5);
        });
        Reject([] {
            Splits(1,100,0,0.5);
        });
        Reject([] {
            Snap(0,1,0);
        });
        Reject([] { Snap(1,std::numeric_limits<double>::max(),1); });
        Reject([] { Snap(0,1e-300,1024); });
        Reject([] { Snap(1000001,1,1024); });
        Reject([] { Snap(0,1,16385); });
        Check(std::isfinite(Snap(1000000,0.0001,16384)), "bounded extreme stays finite");
        Check(std::isfinite(Snap(-1000000,1000000,1)), "largest extent stays finite");
        std::cout<<"mixed splits:";
        for(double z:mixed)std::cout<<' '<<z;
        std::cout<<"\n";
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
