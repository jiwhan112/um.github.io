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
    // RGB 신호 값만 변환한다. alpha는 빛의 세기가 아니라 합성 가중치다.
    // 어두운 구간의 선형 toe와 나머지 거듭제곱 구간을 구분한다.
    double Decode(double s) {
        Finite(s);
        Require(s>=0 && s<=1, "sRGB signal in [0,1]");
        return s<=0.04045 ? s/12.92 : std::pow((s+0.055)/1.055,2.4);
    }
    double Encode(double l) {
        Finite(l);
        Require(l>=0 && l<=1, "SDR linear input in [0,1]");
        return l<=0.0031308 ? 12.92*l : 1.055*std::pow(l,1.0/2.4)-0.055;
    }
    struct Color {
        double r,g,b,a;
    };
    Color Premultiply(Color c) {
        Finite(c.a);
        Require(c.a>=0&&c.a<=1,"alpha in [0,1]");
        return {
            Decode(c.r)*c.a,Decode(c.g)*c.a,Decode(c.b)*c.a,c.a
        };
    }
    // RGB는 이미 alpha가 곱해져 있으므로 source alpha를 다시 곱하지 않는다.
    Color Over(Color front, Color back) {
        // 두 입력은 이미 선형·premultiplied 상태라는 내부 계약이다.
        const double remain=1-front.a;
        return {
            front.r+back.r*remain,front.g+back.g*remain, front.b+back.b*remain,front.a+back.a*remain
        };
    }
    // 경계값, 왕복 오차, 잘못된 신호 평균과 합성 불변식을 독립적으로 확인한다.
    void Run() {
        Check(Near(Decode(0),0),"black");
        Check(Near(Decode(1),1),"white");
        Check(Near(Decode(0.04045),0.04045/12.92),"toe branch");
        Check(Near(Encode(0.0031308),0.0031308*12.92),"encode toe");
        for(int i=0;i<=1000;++i) {
            const double s=i/1000.0;
            Check(Near(Encode(Decode(s)),s,3e-7),"round trip");
        }
        const double correct=Encode((Decode(0)+Decode(1))*0.5);
        Check(correct>0.73 && correct<0.74,"linear white-black average");
        Check(Decode(0.5)<0.22,"signal half is not energy half");
        const auto red=Premultiply( {
            1,0,0,0.5
        });
        const auto blue=Premultiply( {
            0,0,1,1
        });
        auto mixed=Over(red,blue);
        Check(Near(mixed.r,0.5)&&Near(mixed.b,0.5)&&Near(mixed.a,1),"over");
        auto invisible=Over(Premultiply( {
            1,1,1,0
        }),blue);
        Check(Near(invisible.r,0)&&Near(invisible.b,1),"transparent color");
        Reject([] {
            Premultiply( {
                1,1,1,2
            });
        });
        Reject([] {
            Decode(-0.1);
        });
        Reject([] {
            Encode(2);
        });
        Reject([] {
            Decode(std::numeric_limits<double>::quiet_NaN());
        });
        std::cout<<"linear blend encoded="<<correct<<"; encoded-space blend=0.5\n";
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
