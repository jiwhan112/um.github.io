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
        double x,y,z;
    };
    struct Plane {
        P normal;
        double d;
    };
    struct Sphere {
        P center;
        double radius;
    };
    double Dot(P a,P b) {
        return a.x*b.x+a.y*b.y+a.z*b.z;
    }
    // 법선 길이를 반지름에도 반영해 동일 평면의 스케일에 독립적인 판정을 한다.
    bool Visible(const Sphere& s,const std::array<Plane,6>& planes,int& tested) {
        Finite(s.radius);
        Require(s.radius>=0,"nonnegative radius");
        for(double v: {
            s.center.x,s.center.y,s.center.z
        })Finite(v);
        for(const auto& p:planes) {
            ++tested;
            for(double v: {
                p.normal.x,p.normal.y,p.normal.z,p.d
            })Finite(v);
            const double length=std::sqrt(Dot(p.normal,p.normal));
            Require(length>0,"nonzero plane");
            if(Dot(p.normal,s.center)+p.d < -s.radius*length)return false;
        }
        return true;
        // 경계/교차는 보수적으로 보인다.
    }
    // 작은 정면 원판 근사다. near plane을 가로지르는 큰 구에는 쓰지 않는다.
    double RadiusPixels(double radius,double z,double fovY,int height) {
        for(double x: {
            radius,z,fovY
        })Finite(x);
        Require(radius>=0&&z>0&&fovY>0&&fovY<PI&&height>0,"projection domain");
        return radius*height/(2*z*std::tan(fovY/2));
        // 작은 정면 원판 근사, 구 실루엣의 정확식은 아님
    }
    // 들어갈 때와 나올 때의 임계값을 분리한다. 중간 영역은 이전 상태를 유지한다.
    int ChooseLod(double pixels,int old) {
        Require(std::isfinite(pixels)&&pixels>=0&&(old==0||old==1),"LOD contract");
        if(old==0&&pixels<45)return 1;
        if(old==1&&pixels>55)return 0;
        return old;
    }
    // 후보 수와 비교 횟수는 실제 GPU 프레임 시간과 다른 지표다.
    void Run() {
        std::array<Plane,6> box= {
            Plane {
               {
                    1,0,0
                },1
            },Plane {
               {
                    -1,0,0
                },1
            },Plane {
               {
                    0,1,0
                },1
            },Plane {
               {
                    0,-1,0
                },1
            },Plane {
               {
                    0,0,1
                },1
            },Plane {
               {
                    0,0,-1
                },1
            }
        };
        int tested=0;
        Check(Visible( {
           {
                0,0,0
            },0.1
        },box,tested),"inside");
        Check(tested==6,"six planes tested");
        Check(!Visible( {
           {
                3,0,0
            },0.5
        },box,tested),"outside");
        Check(Visible( {
           {
                1.5,0,0
            },0.5
        },box,tested),"touching is visible");
        auto scaled=box;
        for(auto& p:scaled) {
            p.normal.x*=3;
            p.normal.y*=3;
            p.normal.z*=3;
            p.d*=3;
        }
        int dummy=0;
        Check(Visible( {
           {
                1.5,0,0
            },0.5
        },scaled,dummy),"nonunit plane supported");
        Check(Near(RadiusPixels(1,10,PI/2,1000),50),"projection scale");
        Check(Near(RadiusPixels(1,20,PI/2,1000),25),"distance halves radius");
        int old=0,changes=0;
        for(double p: {
            51.,49.,51.,49.,44.,46.,54.,56.
        }) {
            int next=ChooseLod(p,old);
            changes+=next!=old;
            old=next;
        }
        Check(changes==2,"hysteresis prevents threshold chatter");
        int candidates=0,planeTests=0;
        for(int x=-10;x<=10;++x)if(Visible( {
           {
                x*0.5,0,0
            },0.2
        },box,planeTests))++candidates;
        Check(candidates==5,"deterministic candidate count");
        Reject([] {
            RadiusPixels(1,0,1,100);
        });
        Reject([] {
            ChooseLod(-1,0);
        });
        std::cout<<"candidates="<<candidates<<" of 21; CPU plane tests="<<planeTests<<"; no GPU timing measured\n";
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
