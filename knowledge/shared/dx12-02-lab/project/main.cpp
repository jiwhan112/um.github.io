#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// GPU 호출이 아닌, 이 단원의 명시적 계약을 검증하는 결정적 CPU 모델이다.
static unsigned checks = 0;
void Check(bool condition, const char* name) {
    ++checks;
    if (!condition) throw std::runtime_error(name); // Release에서도 검사를 유지한다.
}
template < class F > void Reject(F action, const char* name) {
    bool rejected = false;
    try {
        action();
    } catch (const std::exception&) {
        rejected = true;
    }
    Check(rejected, name);
}

// GPU가 참조하는 기록 메모리를 완료 전에 재사용하지 않는 보수적 상태 모델이다.
struct AllocatorModel {
    bool recording = false, closed = false;
    std::uint64_t lastUse = 0;
    void Begin(std::uint64_t completed) {
        if (recording || completed < lastUse) throw std::logic_error("allocator still in use");
        recording = true;
        closed = false;
    }
    void Close() {
        if (!recording) throw std::logic_error("not recording");
        recording = false;
        closed = true;
    }
    void Submit(std::uint64_t signal) {
        if (!closed || signal == 0 || signal <= lastUse) throw std::logic_error("bad submit");
        lastUse = signal;
        closed = false;
    }
};
int main() {
    try {
        AllocatorModel a;
        Reject([&] { a.Close(); }, "close without begin");
        a.Begin(0);
        Check(a.recording, "begin");
        Reject([&] { a.Begin(0); }, "parallel recording rejected");
        a.Close();
        a.Submit(4);
        Check(a.lastUse == 4, "retire value");
        Reject([&] { a.Begin(3); }, "pending GPU");
        Check(!a.recording && a.lastUse == 4, "failure preserves state");
        a.Begin(4);
        a.Close();
        Reject([&] { a.Submit(4); }, "monotonic signal");
        a.Submit(5);
        Check(a.lastUse == 5, "reuse after completion");
        Reject([&] { a.Submit(6); }, "closed list required");
        std::cout << "PASS DX12-02: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
