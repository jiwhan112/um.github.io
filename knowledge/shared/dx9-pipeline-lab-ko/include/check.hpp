#ifndef STUDY_PIPELINE_CHECK_HPP
#define STUDY_PIPELINE_CHECK_HPP
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
// [한국어 해설] 조건·근사값·예외 검사를 모으는 학습용 도우미다.
struct Checks {
    int count{};
    void That(bool ok,const char* label) {
        ++count; if (!ok) throw std::runtime_error(std::string("FAIL: ")+label);
    }
// [한국어 해설] 유한성과 상대 스케일을 반영한 허용 오차로 기대값을 비교한다.
    void Near(double value,double expected,const char* label) {
        That(std::isfinite(value) && std::abs(value-expected)<=1e-9*(1+std::abs(expected)),label);
    }
// [한국어 해설] invalid_argument 거부 계약을 검사한다. 다른 예외까지 성공으로 간주하지 않는다.
    template<class F> void Rejects(F&& f,const char* label) {
        bool rejected=false;
        try { std::forward<F>(f)(); } catch(const std::invalid_argument&) { rejected=true; }
        That(rejected,label);
    }
};
// [한국어 해설] 예외를 실패 종료로 바꾸고 성공 시 실제 검사 수를 표시한다.
template<class F> int Run(const char* name,F f) {
    try { Checks c; f(c); std::cout<<"PASS "<<name<<" checks="<<c.count<<'\n'; return 0; }
    catch(const std::exception& e) { std::cerr<<name<<": "<<e.what()<<'\n'; return 1; }
}
#endif

