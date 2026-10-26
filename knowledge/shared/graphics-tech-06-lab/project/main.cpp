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
    using Image=std::array<double,25>;
    // hard threshold 대신 threshold를 뺀 양수 성분을 사용한다.
    Image Extract(const Image& src,double threshold) {
        Finite(threshold);
        Require(threshold>=0,"nonnegative threshold");
        Image out {
        };
        for(std::size_t i=0;i<src.size();++i) {
            Finite(src[i]);
            Require(src[i]>=0,"nonnegative radiance");
            out[i]=std::max(src[i]-threshold,0.0);
        }
        return out;
    }
    Image Blur(const Image& src,bool horizontal) {
        const double w[3]= {
            0.25,0.5,0.25
        };
        Image out {
        };
        for(int y=0;y<5;++y)for(int x=0;x<5;++x)for(int k=-1;k<=1;++k) {
            int sx=std::clamp(x+(horizontal?k:0),0,4),sy=std::clamp(y+(horizontal?0:k),0,4);
            out[static_cast<std::size_t>(y)*5+x]+=src[static_cast<std::size_t>(sy)*5+sx]*w[k+1];
        }
        return out;
        // 별도 출력으로 읽기/쓰기 피드백을 차단한다.
    }
    // 분리형 결과를 비교하기 위한 독립적인 2D convolution 기준 구현이다.
    Image BlurDirect(const Image& src) {
        const double w[3]= {
            0.25,0.5,0.25
        };
        Image out {
        };
        for(int y=0;y<5;++y)for(int x=0;x<5;++x)for(int j=-1;j<=1;++j)for(int i=-1;i<=1;++i) out[static_cast<std::size_t>(y)*5+x]+=src[static_cast<std::size_t>(std::clamp(y+j,0,4))*5+std::clamp(x+i,0,4)]*w[j+1]*w[i+1];
        return out;
    }
    // 단순 Reinhard. RGB 색역 관리나 HDR 디스플레이 출력 구현이 아니다.
    double Tone(double x) {
        Finite(x);
        Require(x>=0,"nonnegative HDR");
        return x/(1+x);
    }
    // 중앙 impulse와 상수 영상으로 필터·경계·비선형 합성 순서를 확인한다.
    void Run() {
        Image impulse {
        };
        impulse[12]=8;
        auto bright=Extract(impulse,1),blur=Blur(Blur(bright,true),false),direct=BlurDirect(bright);
        Check(Near(bright[12],7),"soft threshold difference");
        Check(Near(blur[12],1.75),"center weight");
        double sum=0;
        for(std::size_t i=0;i<25;++i) {
            Check(Near(blur[i],direct[i]),"separable equals 2d");
            sum+=blur[i];
        }
        Check(Near(sum,7),"interior impulse sum");
        Image constant {
        };
        constant.fill(3);
        auto smooth=Blur(Blur(constant,true),false);
        for(double x:smooth)Check(Near(x,3),"constant including border");
        Check(Near(Tone(0),0)&&Near(Tone(1),0.5),"tone reference");
        Check(Tone(8+blur[12])>Tone(8),"bloom before tone");
        Check(Tone(8+blur[12])<Tone(8)+Tone(blur[12]),"tone and add not commutative");
        Reject([] {
            Tone(-1);
        });
        Reject([&] {
            Extract(impulse,-1);
        });
        std::cout<<"center HDR="<<8+blur[12]<<"; SDR="<<Tone(8+blur[12])<<"\n";
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
