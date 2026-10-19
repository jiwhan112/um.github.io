#ifndef STUDY_PIPELINE_CHECK_HPP
#define STUDY_PIPELINE_CHECK_HPP
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
struct Checks {
    int count{};
    void That(bool ok,const char* label) {
        ++count; if (!ok) throw std::runtime_error(std::string("FAIL: ")+label);
    }
    void Near(double value,double expected,const char* label) {
        That(std::isfinite(value) && std::abs(value-expected)<=1e-9*(1+std::abs(expected)),label);
    }
    template<class F> void Rejects(F&& f,const char* label) {
        bool rejected=false;
        try { std::forward<F>(f)(); } catch(const std::invalid_argument&) { rejected=true; }
        That(rejected,label);
    }
};
template<class F> int Run(const char* name,F f) {
    try { Checks c; f(c); std::cout<<"PASS "<<name<<" checks="<<c.count<<'\n'; return 0; }
    catch(const std::exception& e) { std::cerr<<name<<": "<<e.what()<<'\n'; return 1; }
}
#endif

