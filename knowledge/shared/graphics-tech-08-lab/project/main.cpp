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
    // 수학적 xy 평면에서 반시계 방향이 양수인 부호 면적이다.
    double Area(P a,P b,P c) {
        return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
    }
    // 이 작은 월드 좌표 실습의 epsilon. 범용 강건 기하 predicate가 아니다.
    bool Same(P a,P b) {
        return std::abs(a.x-b.x)<1e-9&&std::abs(a.y-b.y)<1e-9;
    }
    struct Portal {
        P left,right;
    };
    std::vector<P> Pull(P start,P end,const std::vector<Portal>& gates) {
        // y가 위로 증가하는 평면. 각 portal은 진행 방향에서 왼쪽/오른쪽 순서다.
        std::vector<Portal> all;
        all.push_back( {
            start,start
        });
        all.insert(all.end(),gates.begin(),gates.end());
        all.push_back( {
            end,end
        });
        Require(all.size()<=10000,"bounded portal count");
        for(const auto& g:all)for(double v: {
            g.left.x,g.left.y,g.right.x,g.right.y
        }) {
            Finite(v);
            Require(std::abs(v)<=1000000,"coordinate bound");
        }
        P apex=start,left=start,right=start;
        std::size_t leftIndex=0,rightIndex=0;
        std::vector<P> result {
            start
        };
        std::size_t budget=all.size()*all.size()+1;
        for(std::size_t i=1;i<all.size();++i) {
            Require(budget--!=0,"invalid corridor iteration budget");
            P nextLeft=all[i].left,nextRight=all[i].right;
            // 오른쪽 경계는 반시계 방향으로 좁혀진다.
            if(Area(apex,right,nextRight)>=0) {
                if(Same(apex,right)||Area(apex,left,nextRight)<0) {
                    right=nextRight;
                    rightIndex=i;
                } else {
                    result.push_back(left);
                    apex=left;
                    const auto restart=leftIndex;
                    left=right=apex;
                    leftIndex=rightIndex=restart;
                    i=restart;
                    continue;
                }
            }
            // 왼쪽 경계는 시계 방향으로 좁혀진다.
            if(Area(apex,left,nextLeft)<=0) {
                if(Same(apex,left)||Area(apex,right,nextLeft)>0) {
                    left=nextLeft;
                    leftIndex=i;
                } else {
                    result.push_back(right);
                    apex=right;
                    const auto restart=rightIndex;
                    left=right=apex;
                    leftIndex=rightIndex=restart;
                    i=restart;
                    continue;
                }
            }
        }
        if(!Same(result.back(),end)) result.push_back(end);
        return result;
    }
    // 경로 길이는 코너 사이 선분 길이의 합이다. 이동 시간이나 지형 cost가 아니다.
    double Length(const std::vector<P>& p) {
        double d=0;
        for(std::size_t i=1;i<p.size();++i)d+=std::hypot(p[i].x-p[i-1].x,p[i].y-p[i-1].y);
        return d;
    }
    // 직선, L자, 폭 0 portal, 같은 끝점과 폭 sweep을 검증한다.
    void Run() {
        auto line=Pull( {
            0,0
        }, {
            4,0
        }, {
           {
               {
                    1,1
                }, {
                    1,-1
                }
            }, {
               {
                    3,1
                }, {
                    3,-1
                }
            }
        });
        Check(line.size()==2&&Near(Length(line),4),"straight corridor");
        auto bend=Pull( {
            0,0
        }, {
            4,4
        }, {
           {
               {
                    1,1
                }, {
                    1,-1
                }
            }, {
               {
                    2,3
                }, {
                    3,1
                }
            }, {
               {
                    3,4
                }, {
                    4,3
                }
            }
        });
        Check(Same(bend.front(), {
            0,0
        })&&Same(bend.back(), {
            4,4
        }),"endpoints");
        Check(bend.size()>=2&&Length(bend)>=std::sqrt(32.0)-1e-8,"length lower bound");
        auto narrow=Pull( {
            0,0
        }, {
            4,0
        }, {
           {
               {
                    2,1
                }, {
                    2,1
                }
            }
        });
        Check(narrow.size()==3&&Same(narrow[1], {
            2,1
        }),"zero-width portal forced corner");
        Check(Near(Length(narrow),2*std::sqrt(5.0)),"known forced path length");
        auto turn=Pull( {
            0,0
        }, {
            4,4
        }, {
           {
               {
                    2,1
                }, {
                    2,-1
                }
            }, {
               {
                    2,3
                }, {
                    4,3
                }
            }
        });
        Check(turn.size()==3&&Same(turn[1], {
            2,1
        }),"L corridor inner corner");
        Check(Near(Length(turn),std::sqrt(5.0)+std::sqrt(13.0)),"L corridor length");
        Check(Pull( {
            1,1
        }, {
            1,1
        }, {
        }).size()==1,"identical endpoints");
        Check(Pull( {
            0,0
        }, {
            2,0
        }, {
        }).size()==2,"empty portal list");
        for(int width=1;width<=100;++width) {
            const double w=width/10.0;
            auto straight=Pull( {
                0,0
            }, {
                10,0
            }, {
               {
                   {
                        2,w
                    }, {
                        2,-w
                    }
                }, {
                   {
                        5,w
                    }, {
                        5,-w
                    }
                }, {
                   {
                        8,w
                    }, {
                        8,-w
                    }
                }
            });
            Check(straight.size()==2 && Near(Length(straight),10),"straight corridor width sweep");
        }
        Reject([] {
            Pull( {
                0,0
            }, {
                std::numeric_limits<double>::quiet_NaN(),0
            }, {
            });
        });
        std::cout<<"L-corridor corners="<<turn.size()<<" length="<<Length(turn)<<"\n";
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
