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
    struct V {
        double x,y,z;
    };
    V operator+(V a,V b) {
        return {
            a.x+b.x,a.y+b.y,a.z+b.z
        };
    }
    V operator-(V a,V b) {
        return {
            a.x-b.x,a.y-b.y,a.z-b.z
        };
    }
    V operator*(V a,double s) {
        return {
            a.x*s,a.y*s,a.z*s
        };
    }
    double Dot(V a,V b) {
        return a.x*b.x+a.y*b.y+a.z*b.z;
    }
    V Cross(V a,V b) {
        return {
            a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x
        };
    }
    V Unit(V a) {
        Finite(a.x);
        Finite(a.y);
        Finite(a.z);
        const double l=std::sqrt(Dot(a,a));
        Require(std::isfinite(l)&&l>1e-12,"nonzero finite vector");
        return a*(1/l);
    }
    struct Frame {
        V t,b,n;
    };
    // N을 기준으로 T의 법선 성분을 제거하고 UV 방향성으로 B를 정한다.
    Frame MakeFrame(V normal,V tangent,double handedness) {
        Require(handedness==1 || handedness==-1,"handedness is +1 or -1");
        V n=Unit(normal);
        V t=Unit(tangent-n*Dot(tangent,n));
        // 접선을 법선 평면에 투영한다.
        return {
            t,Cross(n,t)*handedness,n
        };
    }
    // sampled의 성분은 월드 좌표가 아니라 T/B/N 기저의 계수다.
    V ToWorld(Frame f,V sampled) {
        V n=Unit(sampled);
        return Unit(f.t*n.x+f.b*n.y+f.n*n.z);
    }
    V ScaleDirection(V v,V scale) {
        return {
            v.x*scale.x,v.y*scale.y,v.z*scale.z
        };
    }
    V ScaleNormal(V n,V scale) {
        Finite(scale.x);
        Finite(scale.y);
        Finite(scale.z);
        Require(std::abs(scale.x)>1e-12&&std::abs(scale.y)>1e-12&&std::abs(scale.z)>1e-12,"invertible diagonal scale");
        // 이 실습은 대각 scale만 다룬다. 일반 행렬 역전치 구현이 아니다.
        return Unit( {
            n.x/scale.x,n.y/scale.y,n.z/scale.z
        });
    }
    // 비균일 scale의 반례와 중립 normal map, 미러 UV를 비교한다.
    void Run() {
        const V n=Unit( {
            1,1,0
        }), t=Unit( {
            1,-1,0
        }), s= {
            2,1,3
        };
        const V tn=ScaleNormal(n,s), tt=ScaleDirection(t,s);
        Check(Near(Dot(tn,tt),0),"normal remains perpendicular");
        Check(std::abs(Dot(Unit(ScaleDirection(n,s)),tt))>0.5,"wrong transform counterexample");
        const Frame f=MakeFrame( {
            0,0,2
        }, {
            2,0,1
        },1);
        Check(Near(Dot(f.t,f.n),0),"orthogonal tangent");
        Check(Near(Dot(f.b,f.n),0),"orthogonal bitangent");
        Check(Near(Dot(f.t,f.t),1)&&Near(Dot(f.b,f.b),1),"unit axes");
        const V neutral=ToWorld(f, {
            0,0,1
        });
        Check(Near(neutral.z,1),"neutral normal map");
        Check(Near(ToWorld(f, {
            0,1,0
        }).y,1),"positive green");
        Check(Near(ToWorld(MakeFrame( {
            0,0,1
        }, {
            1,0,0
        },-1), {
            0,1,0
        }).y,-1),"mirrored UV parity");
        Reject([] {
            MakeFrame( {
                0,0,1
            }, {
                0,0,4
            },1);
        });
        Reject([] {
            ScaleNormal( {
                1,0,0
            }, {
                0,1,1
            });
        });
        Reject([] {
            MakeFrame( {
                0,0,0
            }, {
                1,0,0
            },1);
        });
        Reject([] {
            MakeFrame( {
                0,0,1
            }, {
                1,0,0
            },0);
        });
        std::cout<<"correct transformed normal dot tangent="<<Dot(tn,tt)<<"\n";
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
