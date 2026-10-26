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
    struct P {
        double x,y;
    };
    P operator+(P a,P b) {
        return {
            a.x+b.x,a.y+b.y
        };
    }
    P operator-(P a,P b) {
        return {
            a.x-b.x,a.y-b.y
        };
    }
    P operator*(P a,double s) {
        return {
            a.x*s,a.y*s
        };
    }
    double Norm(P a) {
        return std::hypot(a.x,a.y);
    }
    double Dot(P a,P b) {
        return a.x*b.x+a.y*b.y;
    }
    double Cross(P a,P b) {
        return a.x*b.y-a.y*b.x;
    }
    struct Pose {
        P joint,tip;
        bool clamped;
    };
    // 도달 가능 거리 [|a-b|, a+b]로 clamp한 뒤 삼각형을 복원한다.
    Pose TwoBone(double a, double b, P target, int bend) {
        for (double value : {a,b,target.x,target.y}) Finite(value);
        Require(a>=0.0001 && b>=0.0001 && a<=1000 && b<=1000,
                "bone lengths in [1e-4,1000]");
        Require(std::abs(target.x)<=1000000 && std::abs(target.y)<=1000000,
                "target coordinate magnitude <= 1e6");
        Require(bend==1 || bend==-1, "bend is +1 or -1");
        const double distance = Norm(target);
        Require(distance==0 || distance>=0.0001,
                "target distance is zero or at least 1e-4");
        const double inner = std::abs(a-b);
        const double outer = a+b;
        // root target를 epsilon으로 같은 길이라고 뭉개지 않는다.
        // 서로 다른 길이라면 +x쪽의 최소 도달점과 정확한 접힘을 반환한다.
        if (distance==0) {
            return {{a>=b ? a : -a, 0}, {inner,0}, a!=b};
        }
        const P axis{target.x/distance, target.y/distance};
        const double reach = std::clamp(distance,inner,outer);
        const bool clamped = distance<inner || distance>outer;
        // 삼각형이 일직선이 되는 양 끝에서는 sqrt의 cancellation을 피한다.
        if (reach==outer) return {axis*a,axis*reach,clamped};
        if (reach==inner) return {axis*(a>=b ? a : -a),axis*reach,clamped};
        // a²-b²를 (a-b)(a+b)로 계산해 비슷한 길이의 뺄셈 오차를 줄인다.
        const double along = std::clamp((a-b)*(a+b)/(2*reach)+reach/2,-a,a);
        const double height = std::sqrt(std::max(0.0,(a-along)*(a+along)));
        const P perpendicular{-axis.y,axis.x};
        return {axis*along+perpendicular*(height*bend),axis*reach,clamped};
    }
    // 수렴하지 않아도 최종 pose와 오차, 수행한 sweep 수를 반환한다.
    struct CcdResult {
        std::vector<P> joints;
        int sweeps;
        double error;
    };
    // 끝쪽 관절부터 root로 순회한다. 각 회전은 해당 관절 뒤 모든 점에 적용한다.
    CcdResult Ccd(std::vector<P> joints,P target,int maxSweeps,double tolerance) {
        Require(joints.size()>=2&&joints.size()<=32&&maxSweeps>=0&&maxSweeps<=1000&&tolerance>=0,"CCD limits");
        for(P p:joints) {
            Finite(p.x);
            Finite(p.y);
            Require(std::abs(p.x)<=1000000 && std::abs(p.y)<=1000000,
                    "CCD joint coordinate magnitude <= 1e6");
        }
        Finite(target.x);
        Finite(target.y);
        Finite(tolerance);
        Require(std::abs(target.x)<=1000000 && std::abs(target.y)<=1000000,
                "CCD target coordinate magnitude <= 1e6");
        for (std::size_t i=1; i<joints.size(); ++i) {
            const double length = Norm(joints[i]-joints[i-1]);
            Require(length>=0.0001 && length<=1000, "CCD bone length in [1e-4,1000]");
        }
        int done=0;
        for(;done<maxSweeps && Norm(joints.back()-target)>tolerance;++done) {
            for(int j=static_cast<int>(joints.size())-2;j>=0;--j) {
                P from=joints.back()-joints[j],to=target-joints[j];
                if(Norm(from)<1e-12||Norm(to)<1e-12)continue;
                double angle=std::atan2(Cross(from,to),Dot(from,to)),c=std::cos(angle),s=std::sin(angle);
                for(std::size_t k=static_cast<std::size_t>(j)+1;k<joints.size();++k) {
                    P v=joints[k]-joints[j];
                    joints[k]=joints[j]+P {
                        c*v.x-s*v.y,s*v.x+c*v.y
                    };
                }
            }
        }
        return {
            joints,done,Norm(joints.back()-target)
        };
    }
    // 두 팔꿈치 해, 안쪽/바깥쪽 도달 실패, 접힘과 반복 상한을 구분한다.
    void Run() {
        auto up=TwoBone(1,1, {
            1,1
        },1),down=TwoBone(1,1, {
            1,1
        },-1);
        Check(Near(Norm(up.joint),1)&&Near(Norm(up.tip-up.joint),1),"bone lengths");
        Check(Near(Norm(up.tip-P {
            1,1
        }),0)&&!up.clamped,"reachable endpoint");
        Check(Norm(up.joint-down.joint)>1,"two elbow branches");
        auto far=TwoBone(1,1, {
            10,0
        },1);
        Check(far.clamped&&Near(far.tip.x,2),"outer reach clamp");
        auto near=TwoBone(2,1, {
            0,0
        },1);
        Check(near.clamped&&Near(Norm(near.tip),1),"inner hole clamp");
        auto folded=TwoBone(1,1, {
            0,0
        },1);
        Check(Near(Norm(folded.tip),0)&&Near(Norm(folded.joint),1),"root target");
        auto c=Ccd( {
           {
                0,0
            }, {
                1,0
            }, {
                2,0
            }
        }, {
            1,1
        },100,1e-6);
        Check(c.error<1e-6,"CCD reachable");
        Check(Near(Norm(c.joints[1]-c.joints[0]),1)&&Near(Norm(c.joints[2]-c.joints[1]),1),"CCD preserves lengths");
        auto limited=Ccd( {
           {
                0,0
            }, {
                1,0
            }, {
                2,0
            }
        }, {
            10,1
        },3,1e-6);
        Check(limited.sweeps<=3&&limited.error>1,"bounded unreachable");
        Reject([] {
            TwoBone(0,1, {
                1,1
            },1);
        });
        Reject([] {
            TwoBone(1,1, {
                1,1
            },0);
        });
        // 리뷰에서 발견한 수치 정의역 회귀를 유지한다.
        Reject([] { TwoBone(1e-12,2e-12,{0,0},1); });
        Reject([] { TwoBone(1,1,{1.7e308,1.7e308},1); });
        Reject([] { TwoBone(1,1,{1e-8,0},1); });
        const auto small = TwoBone(0.0001,0.0002,{0,0},1);
        Check(small.clamped && Near(Norm(small.joint),0.0001,1e-12), "small supported root first bone");
        Check(Near(Norm(small.tip-small.joint),0.0002,1e-12), "small supported root second bone");
        const auto almostEqual = TwoBone(1,1+1e-12,{0,0},1);
        Check(almostEqual.clamped && Near(Norm(almostEqual.tip-almostEqual.joint),1+1e-12,1e-13), "near equal root lengths preserved");
        const auto extreme = TwoBone(1000,0.0001,{1000000,1000000},-1);
        Check(extreme.clamped && Near(Norm(extreme.joint),1000,1e-8), "bounded far target first bone");
        Check(Near(Norm(extreme.tip-extreme.joint),0.0001,1e-8), "bounded far target second bone");
        Reject([] { Ccd({{0,0},{1,0}}, {1.7e308,1.7e308}, 1, 1e-6); });
        std::cout<<"CCD sweeps="<<c.sweeps<<" error="<<c.error<<"\n";
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
