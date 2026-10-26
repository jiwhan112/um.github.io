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

using Graph = std::vector<std::vector<std::size_t>>;

void Validate(const Graph& graph, std::size_t start) {
    if (graph.empty() || graph.size() > 1000 || start >= graph.size()) {
        throw std::invalid_argument("graph range");
    }
    for (const auto& neighbors : graph) {
        for (auto vertex : neighbors) {
            if (vertex >= graph.size()) {
                throw std::invalid_argument("bad edge");
            }
        }
    }
}

std::vector<int> Bfs(const Graph& graph, std::size_t start) {
    Validate(graph, start);
    std::vector<int> distance(graph.size(), -1);
    std::queue<std::size_t> queue;
    distance[start] = 0;
    queue.push(start);
    while (!queue.empty()) {
        const auto current = queue.front();
        queue.pop();
        for (auto next : graph[current]) {
            if (distance[next] == -1) {
                // 큐에 넣는 순간 발견 표시를 하므로 중복 삽입하지 않습니다.
                distance[next] = distance[current] + 1;
                queue.push(next);
            }
        }
    }
    return distance;
}

std::vector<bool> Dfs(const Graph& graph, std::size_t start) {
    Validate(graph, start);
    std::vector<bool> seen(graph.size(), false);
    std::vector<std::size_t> stack{start};
    seen[start] = true;
    while (!stack.empty()) {
        const auto current = stack.back();
        stack.pop_back();
        for (auto next : graph[current]) {
            if (!seen[next]) {
                seen[next] = true;
                stack.push_back(next);
            }
        }
    }
    return seen;
}

int main() {
    Graph graph{{1, 2}, {3}, {3}, {0}, {4}};
    Check(Bfs(graph, 0) == std::vector<int>({0, 1, 1, 2, -1}), "cycle and disconnected");
    std::mt19937 rng(1000);
    for (int trial = 0; trial < 100; ++trial) {
        Graph random(8);
        std::array<std::array<int, 8>, 8> distance{};
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                distance[i][j] = i == j ? 0 : 100;
            }
        }
        for (std::size_t i = 0; i < 8; ++i) {
            for (std::size_t j = 0; j < 8; ++j) {
                if (rng() % 5 == 0) {
                    random[i].push_back(j);
                    distance[i][j] = std::min(distance[i][j], 1);
                }
            }
        }
        // 작은 그래프의 모든 쌍 최단 거리를 독립 기준 해법으로 계산합니다.
        for (int k = 0; k < 8; ++k) {
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    distance[i][j] = std::min(distance[i][j], distance[i][k] + distance[k][j]);
                }
            }
        }
        for (std::size_t start = 0; start < 8; ++start) {
            const auto breadth = Bfs(random, start);
            const auto depth = Dfs(random, start);
            for (std::size_t vertex = 0; vertex < 8; ++vertex) {
                const int expected = distance[start][vertex] >= 100 ? -1 : distance[start][vertex];
                Check(breadth[vertex] == expected, "all-pairs oracle");
                Check(depth[vertex] == (breadth[vertex] >= 0), "DFS reachability");
            }
        }
    }
    bool rejected = false;
    try {
        Bfs({{2}}, 0);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    Check(rejected, "edge validation");
    std::cout << "BFS shortest edge counts and DFS reachability match 100 graph oracles\n";
    std::cout << "PASS: " << checks << " contract checks\n";
}
