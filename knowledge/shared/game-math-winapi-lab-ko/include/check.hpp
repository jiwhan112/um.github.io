#ifndef STUDY_CHECK_HPP
#define STUDY_CHECK_HPP
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
// [한국어 해설] 학습용 검사 카운터다. 병렬 검사 실행용 원자 카운터는 아니다.
inline int checks=0;
inline void Check(bool condition,const char* label) {
    ++checks; if (!condition) throw std::runtime_error(label);
}
// [한국어 해설] 상대 크기와 절대 오차를 함께 고려한다. NaN과 무한대는 근사 일치로 처리하지 않는다.
inline bool Near(double a,double b,double eps=1e-9) {
    return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=eps*std::max({1.0,std::abs(a),std::abs(b)});
}
// [한국어 해설] 예외 발생 여부를 검사하지만 정확한 예외 종류까지 검증하지는 않는다.
template<class F> void Throws(F&& f) {
    bool threw=false; try { f(); } catch(const std::exception&) { threw=true; }
    Check(threw,"expected exception");
}
inline int Done(const char* name) { std::cout<<name<<": "<<checks<<" checks passed\n"; return 0; }
#endif
