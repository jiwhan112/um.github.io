#include <cstddef>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// C++11-compatible CPU reference; C++20 is the default build.
// All jobs arrive at tick 0. Ticks are logical, NOT real OS clock ticks.
struct Slice { std::size_t job; unsigned begin; unsigned end; };
struct Schedule { std::vector<Slice> slices; std::vector<unsigned> completed; };

static Schedule RoundRobin(const std::vector<unsigned>& burst, unsigned quantum) {
    if (quantum == 0 || quantum > 1000000U || burst.empty() || burst.size() > 1000)
        throw std::invalid_argument("invalid quantum or job count");
    std::deque<std::size_t> ready;
    unsigned total = 0;
    for (std::size_t i = 0; i < burst.size(); ++i) {
        if (burst[i] == 0 || burst[i] > 1000000U || total > 1000000U - burst[i])
            throw std::invalid_argument("invalid burst or total budget");
        total += burst[i]; ready.push_back(i);
    }
    Schedule result;
    result.completed.assign(burst.size(), 0U);
    std::vector<unsigned> remain = burst;
    unsigned clock = 0;
    while (!ready.empty()) {
        const std::size_t id = ready.front(); ready.pop_front();
        const unsigned used = remain[id] < quantum ? remain[id] : quantum;
        result.slices.push_back(Slice{id, clock, clock + used});
        clock += used; remain[id] -= used;
        if (remain[id] != 0) ready.push_back(id);
        else result.completed[id] = clock;
    }
    if (clock != total) throw std::logic_error("internal accounting error");
    return result;
}

static unsigned checks = 0;
static void Check(bool valid, const std::string& label) {
    ++checks; if (!valid) throw std::runtime_error(label);
}
template<typename F> static void Reject(F fn) {
    bool threw = false;
    try { fn(); } catch (const std::invalid_argument&) { threw = true; }
    Check(threw, "expected validation failure");
}
int main() {
    try {
        const Schedule s = RoundRobin(std::vector<unsigned>{5, 2, 1}, 2);
        Check(s.slices.size() == 5, "slice count");
        const std::size_t expectedId[] = {0, 1, 2, 0, 0};
        const unsigned expectedBegin[] = {0, 2, 4, 5, 7};
        const unsigned expectedEnd[] = {2, 4, 5, 7, 8};
        for (std::size_t i = 0; i < s.slices.size(); ++i) {
            Check(s.slices[i].job == expectedId[i], "round-robin order");
            Check(s.slices[i].begin == expectedBegin[i], "slice begins");
            Check(s.slices[i].end == expectedEnd[i], "slice ends");
        }
        Check(s.completed == std::vector<unsigned>({8, 4, 5}), "completion times");
        const Schedule shortJobs = RoundRobin(std::vector<unsigned>{1, 1}, 5);
        Check(shortJobs.slices.size() == 2, "short jobs not padded");
        Check(shortJobs.completed == std::vector<unsigned>({1, 2}), "short completion");
        const Schedule one = RoundRobin(std::vector<unsigned>{4}, 1);
        Check(one.slices.size() == 4 && one.completed[0] == 4, "single job");
        Reject([] { RoundRobin(std::vector<unsigned>{1}, 0); });
        Reject([] { RoundRobin(std::vector<unsigned>(), 1); });
        Reject([] { RoundRobin(std::vector<unsigned>{0}, 1); });
        Reject([] { RoundRobin(std::vector<unsigned>{1000000, 1}, 1); });
        Reject([] { RoundRobin(std::vector<unsigned>{1}, 1000001); });
        std::cout << "PASS OS04 checks=" << checks << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL OS04: " << e.what() << "\n";
        return 1;
    }
}
