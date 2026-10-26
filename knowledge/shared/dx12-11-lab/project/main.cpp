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

bool Acyclic(unsigned nodes, const std::vector < std::pair < unsigned, unsigned >> & edges) {
    std::vector < unsigned > indegree(nodes, 0);
    std::vector < std::vector < unsigned >> out(nodes);
    for (auto edge:edges) {
        if (edge.first >= nodes || edge.second >= nodes) throw std::out_of_range("node");
        out[edge.first].push_back(edge.second);
        ++indegree[edge.second];
    }
    std::vector < unsigned > ready;
    for (unsigned i = 0;i < nodes;++i) if (indegree[i] == 0) ready.push_back(i);
    std::size_t done = 0;
    while (done < ready.size()) { auto n = ready[done++];
    for (auto v:out[n]) if (--indegree[v] == 0) ready.push_back(v);
    }
    return done == nodes;
}
unsigned Groups(unsigned count, unsigned threads) {
    if (threads == 0) throw std::invalid_argument("zero threads");
    return count/threads+(count%threads != 0); // count+threads-1 오버플로 회피
}
int main() {
    try {
        Check(Acyclic(3, {{0, 1}, {1, 2}}), "compute copy CPU chain");
        Check(!Acyclic(2, {{0, 1}, {1, 0}}), "two queue wait cycle");
        Check(!Acyclic(1, {{0, 0}}), "self wait");
        Check(Acyclic(0, {}), "empty graph");
        Reject([] { Acyclic(2, {{0, 2}}); }, "bad edge");
        Check(Groups(64, 32) == 2, "two dispatch groups");
        Check(Groups(65, 32) == 3, "partial group needs shader bound");
        Check(Groups(0, 32) == 0, "empty dispatch planning");
        Check(Groups(UINT32_MAX, 32) == 134217728u, "ceil without addition overflow");
        Reject([] { Groups(1, 0); }, "invalid group size");
        std::cout << "PASS DX12-11: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
