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

enum class Result { Running, Finished, DeviceLost, Timeout };
// 성공과 장치 오류·시간 초과를 구분하고 종료 결과를 덮지 않는다.
struct Diagnostic {
    Result result = Result::Running;
    std::vector < std::string > recent;
    void Mark(const std::string& operation) {
        if (result != Result::Running) throw std::logic_error("already stopped");
        if (recent.size() == 4) recent.erase(recent.begin()); // DRED 구현이 아닌 작은 로그 모델
        recent.push_back(operation);
    }
    void Complete(std::uint64_t completed, std::uint64_t expected, bool timeout) {
        if (result != Result::Running) throw std::logic_error("terminal state");
        if (completed == UINT64_MAX) result = Result::DeviceLost;
        else if (timeout || completed < expected) result = Result::Timeout;
        else result = Result::Finished;
    }
};
int main() {
    try {
        Diagnostic d;
        for (const char* op:{"reset", "copy", "barrier", "draw", "present"}) d.Mark(op);
        Check(d.recent.size() == 4, "bounded history");
        Check(d.recent.front() == "copy", "drop oldest");
        d.Complete(5, 5, false);
        Check(d.result == Result::Finished, "completed");
        Reject([&] { d.Mark("again"); }, "no work after terminal");
        Diagnostic lost;
        lost.Complete(UINT64_MAX, 1, false);
        Check(lost.result == Result::DeviceLost, "UINT64MAX not success");
        Diagnostic pending;
        pending.Complete(2, 3, false);
        Check(pending.result == Result::Timeout, "not complete");
        Diagnostic timeout;
        timeout.Complete(3, 3, true);
        Check(timeout.result == Result::Timeout, "timeout not hidden");
        Reject([&] { timeout.Complete(4, 3, false); }, "cannot overwrite failure");
        Check(lost.recent.empty(), "diagnostic does not invent breadcrumb");
        std::cout << "PASS DX12-12: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
