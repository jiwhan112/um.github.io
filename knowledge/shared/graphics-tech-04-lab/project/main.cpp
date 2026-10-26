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
    struct Shadow {
        int width,height;
        std::vector<double> depth;
    };
    // 교육용 경계 검사다. 실제 렌더러에서는 맵 업로드 시 검증할 수 있다.
    void Validate(const Shadow& s) {
        Require(s.width>0&&s.height>0&&s.width<=4096&&s.height<=4096,"map dimensions");
        Require(s.depth.size()==static_cast<std::size_t>(s.width)*s.height,"map length");
        for(double d:s.depth) {
            Finite(d);
            Require(d>=0&&d<=1,"depth in [0,1]");
        }
    }
    // 반경 r의 커널은 (2r+1)^2개 비교다. border는 밝음으로 명시한다.
    double Pcf(const Shadow& s,double u,double v,double receiver,double bias,int radius) {
        Validate(s);
        for(double x: {
            u,v,receiver,bias
        })Finite(x);
        Require(receiver>=0&&receiver<=1&&bias>=0&&bias<=1,"depth/bias domain");
        Require(radius>=0&&radius<=8,"bounded filter radius");
        // 표준 Z: 가까운 깊이 0. light frustum 밖은 이 예제에서 밝음으로 처리한다.
        if(u<0||u>=1||v<0||v>=1)return 1;
        int cx=static_cast<int>(u*s.width),cy=static_cast<int>(v*s.height),lit=0,count=0;
        for(int y=-radius;y<=radius;++y)for(int x=-radius;x<=radius;++x) {
            int px=cx+x,py=cy+y;
            ++count;
            if(px<0||py<0||px>=s.width||py>=s.height) {
                ++lit;
                continue;
            }
            // 깊이를 평균하는 대신 비교 결과 0/1을 평균한다.
            if(receiver-bias<=s.depth[static_cast<std::size_t>(py)*s.width+px])++lit;
        }
        return static_cast<double>(lit)/count;
    }
    // 평균 순서 반례, equality, 작은/큰 bias와 border 결과를 고정한다.
    void Run() {
        Shadow s {
            3,3, {
                0.2,0.2,0.8, 0.2,0.2,0.8, 0.2,0.8,0.8
            }
        };
        Check(Near(Pcf(s,0.5,0.5,0.5,0,1),4.0/9),"compare before average");
        Check(Near(Pcf(s,0.5,0.5,0.2,0,0),1),"equality passes");
        Check(Near(Pcf(s,0.5,0.5,0.2001,0,0),0),"acne example");
        Check(Near(Pcf(s,0.5,0.5,0.2001,0.001,0),1),"bias removes local error");
        Check(Near(Pcf(s,0.5,0.5,0.7,0.6,0),1),"too much bias leaks light");
        Check(Near(Pcf(s,0.5,0.5,0.1,0,1),1),"all lit");
        Check(Near(Pcf(s,0.5,0.5,0.9,0,1),0),"all blocked");
        Check(Near(Pcf(s,1,0.5,0.5,0,1),1),"outside policy");
        Check(Near(Pcf(s,0,0,0.9,0,1),5.0/9),"bright border policy");
        Reject([&] {
            Pcf(s,0.5,0.5,0.5,-1,0);
        });
        Reject([&] {
            Pcf(s,0.5,0.5,0.5,0,9);
        });
        Reject([] {
            Pcf( {
                1,2, {
                    0.5
                }
            },0,0,0.5,0,0);
        });
        std::cout<<"3x3 PCF visibility="<<Pcf(s,0.5,0.5,0.5,0,1)<<"\n";
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
