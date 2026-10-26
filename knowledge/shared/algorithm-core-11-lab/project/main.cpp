#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Release 빌드에서도 사라지지 않는 계약 검사입니다.
static std::size_t checks = 0;
void Check(bool condition, const char* label) {
    ++checks;
    if (!condition) throw std::runtime_error(label);
}

struct Edge {
    std::size_t to;
    int cost;
};
using WeightedGraph = std::vector<std::vector<Edge>>;
constexpr std::int64_t Inf = std::numeric_limits<std::int64_t>::max() / 4;

std::int64_t Search(const WeightedGraph& graph, std::size_t start, std::size_t goal,
                    const std::vector<std::int64_t>& heuristic) {
    if (graph.empty() || graph.size() > 1000 || start >= graph.size() ||
        goal >= graph.size() || heuristic.size() != graph.size() || heuristic[goal] != 0) {
        throw std::invalid_argument("graph or heuristic");
    }
    // 모든 인덱스와 수치 범위를 먼저 확인한 뒤 휴리스틱을 참조합니다.
    for (std::size_t from = 0; from < graph.size(); ++from) {
        if (heuristic[from] < 0 || heuristic[from] > 1000000000) {
            throw std::invalid_argument("heuristic range");
        }
        for (auto edge : graph[from]) {
            if (edge.to >= graph.size() || edge.cost < 0 || edge.cost > 1000000) {
                throw std::invalid_argument("edge range");
            }
        }
    }
    // 이 교재는 일관적 휴리스틱만 지원합니다. 모든 간선에서 검사합니다.
    for (std::size_t from = 0; from < graph.size(); ++from) {
        for (auto edge : graph[from]) {
            if (heuristic[from] > edge.cost + heuristic[edge.to]) {
                throw std::invalid_argument("inconsistent heuristic");
            }
        }
    }
    using Candidate = std::pair<std::int64_t, std::size_t>;
    std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> open;
    std::vector<std::int64_t> distance(graph.size(), Inf);
    distance[start] = 0;
    open.push({heuristic[start], start});
    while (!open.empty()) {
        const auto [priority, current] = open.top();
        open.pop();
        // 이전 거리로 넣어 두었던 오래된 후보는 확장하지 않습니다.
        if (priority != distance[current] + heuristic[current]) {
            continue;
        }
        if (current == goal) {
            return distance[current];
        }
        for (auto edge : graph[current]) {
            const auto candidate = distance[current] + edge.cost;
            if (candidate < distance[edge.to]) {
                distance[edge.to] = candidate;
                open.push({candidate + heuristic[edge.to], edge.to});
            }
        }
    }
    return Inf;
}

int main() {
    WeightedGraph graph{{{1, 8}, {2, 2}}, {{3, 1}}, {{1, 2}, {3, 10}}, {}};
    Check(Search(graph, 0, 3, {0, 0, 0, 0}) == 5, "stale candidate improves");
    Check(Search(graph, 0, 3, {5, 1, 3, 0}) == 5, "consistent heuristic");
    Check(Search(graph, 3, 0, {0, 0, 0, 0}) == Inf, "unreachable");
    std::mt19937 rng(1100);
    for (int trial = 0; trial < 80; ++trial) {
        WeightedGraph random(7);
        std::array<std::array<std::int64_t, 7>, 7> distance{};
        for (int i = 0; i < 7; ++i) {
            for (int j = 0; j < 7; ++j) {
                distance[i][j] = i == j ? 0 : Inf;
            }
        }
        for (std::size_t i = 0; i < 7; ++i) {
            for (std::size_t j = 0; j < 7; ++j) {
                if (rng() % 4 == 0) {
                    const int weight = static_cast<int>(rng() % 10);
                    random[i].push_back({j, weight});
                    distance[i][j] = std::min(distance[i][j], static_cast<std::int64_t>(weight));
                }
            }
        }
        // INF끼리 더하지 않고, 존재하는 두 경로만 연결합니다.
        for (int k = 0; k < 7; ++k) {
            for (int i = 0; i < 7; ++i) {
                for (int j = 0; j < 7; ++j) {
                    if (distance[i][k] != Inf && distance[k][j] != Inf) {
                        distance[i][j] = std::min(distance[i][j], distance[i][k] + distance[k][j]);
                    }
                }
            }
        }
        for (std::size_t start = 0; start < 7; ++start) {
            for (std::size_t goal = 0; goal < 7; ++goal) {
                Check(Search(random, start, goal, std::vector<std::int64_t>(7, 0)) ==
                      distance[start][goal], "weighted oracle");
            }
        }
    }
    bool rejected = false;
    try {
        Search({{{1, -1}}, {}}, 0, 1, {0, 0});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    Check(rejected, "negative edge rejected");
    rejected = false;
    try {
        Search(graph, 0, 3, {99, 1, 3, 0});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    Check(rejected, "inconsistent heuristic rejected");
    std::cout << "Dijkstra and consistent A*: cost=5; 80 weighted graph oracles\n";
    std::cout << "PASS: " << checks << " contract checks\n";
}
