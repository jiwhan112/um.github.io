#ifndef STUDY_CHECK_HPP
#define STUDY_CHECK_HPP
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
inline int checks=0;
inline void Check(bool condition,const char* label) {
    ++checks; if (!condition) throw std::runtime_error(label);
}
inline bool Near(double a,double b,double eps=1e-9) {
    return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=eps*std::max({1.0,std::abs(a),std::abs(b)});
}
template<class F> void Throws(F&& f) {
    bool threw=false; try { f(); } catch(const std::exception&) { threw=true; }
    Check(threw,"expected exception");
}
inline int Done(const char* name) { std::cout<<name<<": "<<checks<<" checks passed\n"; return 0; }
#endif
