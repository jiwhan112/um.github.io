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
    double Saturate(double x) {
        return std::clamp(x,0.0,1.0);
    }
    double Fresnel(double cosine,double f0) {
        return f0+(1-f0)*std::pow(1-Saturate(cosine),5);
    }
    // alpha는 지각적 roughness r의 제곱이며 a2는 alpha의 제곱이다.
    double Distribution(double nh,double alpha) {
        const double a2=alpha*alpha, d=nh*nh*(a2-1)+1;
        return a2/(PI*d*d);
    }
    // 한 방향의 Smith masking. 수평선 아래에서는 기여를 0으로 둔다.
    double Mask(double nx,double alpha) {
        if(nx<=0)return 0;
        return 2*nx/(nx+std::sqrt(alpha*alpha+(1-alpha*alpha)*nx*nx));
    }
    // BRDF 항과 입사각을 곱한 응답을 구분한다. 화면 인코딩 값이 아니다.
    struct Terms {
        double diffuse,specular,total;
    };
    Terms Shade(double base,double metallic,double perceptualRoughness,double nl,double nv,double nh,double vh) {
        for(double x: {
            base,metallic,perceptualRoughness,nl,nv,nh,vh
        }) {
            Finite(x);
            Require(x>=0&&x<=1,"unit domain");
        }
        if(nl==0 || nv==0) return {
            0,0,0
        };
        // 0.05는 이 교육 모델의 수치 안정화 정책이다. 모든 엔진의 규격이 아니다.
        const double r=std::max(perceptualRoughness,0.05), alpha=r*r;
        const double f0=0.04*(1-metallic)+base*metallic;
        const double f=Fresnel(vh,f0);
        const double diffuse=(1-metallic)*(1-f)*base/PI;
        // G1(n)=2n/(n+sqrt(...))의 nl·nv를 분모와 대수적으로 먼저 소거한다.
        // 매우 작은 양수 내적에서 (nl*nv)/(nl*nv)가 0/0이 되는 것을 피한다.
        const double a2 = alpha * alpha;
        const double denominatorL = nl + std::sqrt(a2 + (1-a2)*nl*nl);
        const double denominatorV = nv + std::sqrt(a2 + (1-a2)*nv*nv);
        const double visibility = 1.0 / (denominatorL * denominatorV);
        const double specular = Distribution(nh,alpha) * f * visibility;
        return {
            diffuse,specular,(diffuse+specular)*nl
        };
        // 광원 radiance=1인 스칼라 채널
    }
    // 기준 각도와 roughness sweep을 검사한다. 반구 에너지 적분 검사는 아니다.
    void Run() {
        Check(Near(Fresnel(1,0.04),0.04),"normal incidence");
        Check(Near(Fresnel(0,0.04),1),"grazing Fresnel");
        Check(Near(Distribution(1,1),1/PI),"rough distribution");
        Check(Near(Mask(1,0.2),1),"normal mask");
        Check(Near(Mask(0,0.2),0),"horizon mask");
        auto plastic=Shade(0.5,0,0.5,1,1,1,1), metal=Shade(0.5,1,0.5,1,1,1,1);
        Check(plastic.diffuse>0,"dielectric diffuse");
        Check(Near(metal.diffuse,0),"metal has no diffuse in this model");
        Check(Shade(0.5,0,0.5,0,1,0.5,0.5).total==0,"unlit horizon");
        Check(std::isfinite(Shade(0.5,0,0,1,1,1,1).total),"zero roughness clamped");
        for(int i=1;i<=100;++i) {
            auto x=Shade(0.8,0.3,i/100.0,0.7,0.8,0.9,0.9);
            Check(x.total>=0&&std::isfinite(x.total),"finite material sweep");
        }
        // 양수지만 곱이 underflow되는 horizon 회귀 입력도 유한해야 한다.
        const auto tiny = Shade(0.5,0,0.5,1e-300,1e-300,0.5,0.5);
        Check(std::isfinite(tiny.specular) && std::isfinite(tiny.total), "tiny horizon stays finite");
        Check(tiny.specular >= 0 && tiny.total >= 0, "tiny horizon nonnegative");
        const auto subnormal = Shade(0.5,0,0.5,std::numeric_limits<double>::denorm_min(),1e-300,0.5,0.5);
        Check(std::isfinite(subnormal.specular) && std::isfinite(subnormal.total), "subnormal horizon stays finite");
        Reject([] {
            Shade(0.5,2,0.5,1,1,1,1);
        });
        std::cout<<"plastic="<<plastic.total<<" metal="<<metal.total<<" (BRDF response, not display signal)\n";
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
