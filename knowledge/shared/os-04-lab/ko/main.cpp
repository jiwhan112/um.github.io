#include <cstddef>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// C++11-compatible CPU reference; C++20 is the default build.
// All jobs arrive at tick 0. Ticks are logical, NOT real OS clock ticks.
struct Slice { std::size_t job; unsigned begin; unsigned end; }; // 한국어: 한 구간은 [begin,end)이며 실제 밀리초가 아닌 논리 틱이다.
struct Schedule { std::vector<Slice> slices; std::vector<unsigned> completed; }; // 한국어: 완료 시각과 실행 구간을 분리해 결과를 보존한다.

static Schedule RoundRobin(const std::vector<unsigned>& burst, unsigned quantum) { // 한국어: 입력은 도착 시각이 모두 0인 양의 burst 목록이다.
    if (quantum == 0 || quantum > 1000000U || burst.empty() || burst.size() > 1000) // 한국어: quantum과 작업 수부터 검증하여 무한 순환 입력을 막는다.
        throw std::invalid_argument("invalid quantum or job count");
    std::deque<std::size_t> ready;
    unsigned total = 0; // 한국어: 총 작업량 상한으로 이후 clock 덧셈 범위를 제한한다.
    for (std::size_t i = 0; i < burst.size(); ++i) {
        if (burst[i] == 0 || burst[i] > 1000000U || total > 1000000U - burst[i]) // 한국어: burst 상한 확인 뒤 뺄셈 형태로 누적 상한을 검사한다.
            throw std::invalid_argument("invalid burst or total budget");
        total += burst[i]; ready.push_back(i); // 한국어: 입력 순서가 최초 준비 큐 순서다.
    }
    Schedule result;
    result.completed.assign(burst.size(), 0U);
    std::vector<unsigned> remain = burst; // 한국어: 원본 burst를 바꾸지 않고 남은 작업량을 따로 추적한다.
    unsigned clock = 0;
    while (!ready.empty()) {
        const std::size_t id = ready.front(); ready.pop_front(); // 한국어: 큐 맨 앞에서 하나를 선택한다.
        const unsigned used = remain[id] < quantum ? remain[id] : quantum; // 한국어: 남은 양이 작으면 quantum 전체를 채우지 않는다.
        result.slices.push_back(Slice{id, clock, clock + used}); // 한국어: 실행 구간을 기록한 뒤 논리 시각을 전진시킨다.
        clock += used; remain[id] -= used;
        if (remain[id] != 0) ready.push_back(id); // 한국어: 끝나지 않은 작업만 뒤로 보낸다.
        else result.completed[id] = clock; // 한국어: 남은 양이 0이 된 시각이 완료 시각이다.
    }
    if (clock != total) throw std::logic_error("internal accounting error"); // 한국어: 모든 입력 작업량이 정확히 소진되었는지 보존식을 확인한다.
    return result;
}

static unsigned checks = 0;
static void Check(bool valid, const std::string& label) {
    ++checks; if (!valid) throw std::runtime_error(label);
}
template<typename F> static void Reject(F fn) { // 한국어: 잘못된 입력은 정상 결과가 아니라 명시적 거부가 기대값이다.
    bool threw = false;
    try { fn(); } catch (const std::invalid_argument&) { threw = true; }
    Check(threw, "expected validation failure");
}
int main() {
    try {
        const Schedule s = RoundRobin(std::vector<unsigned>{5, 2, 1}, 2);
        Check(s.slices.size() == 5, "slice count");
        const std::size_t expectedId[] = {0, 1, 2, 0, 0};
        const unsigned expectedBegin[] = {0, 2, 4, 5, 7}; // 한국어: 이 시각들은 실행 측정이 아니라 손으로 재현 가능한 정답이다.
        const unsigned expectedEnd[] = {2, 4, 5, 7, 8};
        for (std::size_t i = 0; i < s.slices.size(); ++i) {
            Check(s.slices[i].job == expectedId[i], "round-robin order");
            Check(s.slices[i].begin == expectedBegin[i], "slice begins");
            Check(s.slices[i].end == expectedEnd[i], "slice ends");
        }
        Check(s.completed == std::vector<unsigned>({8, 4, 5}), "completion times");
        const Schedule shortJobs = RoundRobin(std::vector<unsigned>{1, 1}, 5); // 한국어: 짧은 작업의 남는 quantum을 대기로 채우지 않는 반례다.
        Check(shortJobs.slices.size() == 2, "short jobs not padded");
        Check(shortJobs.completed == std::vector<unsigned>({1, 2}), "short completion");
        const Schedule one = RoundRobin(std::vector<unsigned>{4}, 1);
        Check(one.slices.size() == 4 && one.completed[0] == 4, "single job");
        Reject([] { RoundRobin(std::vector<unsigned>{1}, 0); });
        Reject([] { RoundRobin(std::vector<unsigned>(), 1); });
        Reject([] { RoundRobin(std::vector<unsigned>{0}, 1); });
        Reject([] { RoundRobin(std::vector<unsigned>{1000000, 1}, 1); }); // 한국어: 합계 초과를 실행 전에 거부한다.
        Reject([] { RoundRobin(std::vector<unsigned>{1}, 1000001); });
        std::cout << "PASS OS04 checks=" << checks << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL OS04: " << e.what() << "\n";
        return 1;
    }
}
