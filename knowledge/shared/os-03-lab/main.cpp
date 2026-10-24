#include <algorithm>
#include <array>
#include <cstddef>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

struct Checks { // [한국어 해설] expect 호출 횟수에는 반복 오라클 비교가 포함된다. 독립 요구사항 수와 다르다.
    unsigned long long n;
    Checks() : n(0) {}
    void expect(bool good, const char* why) {
        ++n;
        if (!good) throw std::runtime_error(why);
    }
};

// Models lock-order acquisition constraints, not operating system waiting.
class LockOrderGraph { // [한국어 해설] 잠금을 잡은 상태에서 다음 잠금을 요청하는 순서 제약을 저장한다. 실시간 대기 그래프가 아니다.
    std::vector<std::vector<bool> > edge_;
public:
    explicit LockOrderGraph(std::size_t count)
        : edge_(count, std::vector<bool>(count, false)) {}

    bool reachable(std::size_t from, std::size_t to) const { // [한국어 해설] 반복 탐색으로 경로 존재 여부를 구한다. from==to이면 길이 0 경로도 참이다.
        if (from >= edge_.size() || to >= edge_.size())
            throw std::out_of_range("lock ID");
        std::vector<bool> seen(edge_.size(), false);
        std::vector<std::size_t> pending(1, from);
        while (!pending.empty()) {
            const std::size_t v = pending.back();
            pending.pop_back();
            if (v == to) return true;
            if (seen[v]) continue;
            seen[v] = true;
            for (std::size_t w = 0; w < edge_.size(); ++w)
                if (edge_[v][w] && !seen[w]) pending.push_back(w);
        }
        return false;
    }
    bool allow(std::size_t held, std::size_t next) { // [한국어 해설] 자기 잠금과 순환을 만드는 새 간선을 거부한다. 거부 전에 그래프를 변경하지 않는다.
        if (held >= edge_.size() || next >= edge_.size())
            throw std::out_of_range("lock ID");
        if (held == next) return false; // non-recursive mutex self-acquisition
        if (edge_[held][next]) return true; // idempotent constraint
        if (reachable(next, held)) return false; // would create a cycle // [한국어 해설] 기존 next→held 경로에 held→next를 추가하면 사이클이 생긴다.
        edge_[held][next] = true;
        return true;
    }
    bool has(std::size_t held, std::size_t next) const { return edge_.at(held).at(next); }
    std::size_t size() const { return edge_.size(); }
};

// Independent transitive closure oracle for deterministic small graphs.
static bool oracle_reachable(const std::vector<std::vector<bool> >& edge, // [한국어 해설] 별도의 Floyd–Warshall 전이 폐쇄를 비교 기준으로 사용한다. 실제 OS 스케줄러를 호출하지 않는다.
                             std::size_t source, std::size_t target) {
    std::vector<std::vector<bool> > reach = edge;
    const std::size_t n = edge.size();
    for (std::size_t i = 0; i < n; ++i) reach[i][i] = true;
    for (std::size_t k = 0; k < n; ++k)
        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                reach[i][j] = reach[i][j] || (reach[i][k] && reach[k][j]);
    return reach[source][target];
}
struct Account { // [한국어 해설] 계정 하나의 잔액과 mutex를 묶는다. 복사할 수 없으며 초기 잔액은 10000이다.
    std::mutex mutex;
    long long coins;
    Account() : coins(10000) {}
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;
};

static void transfer(Account& from, Account& to, long long amount) { // [한국어 해설] 서로 다른 계정과 양수 금액만 허용한다. 이 테스트의 총액 40000 범위에서 산술한다.
    if (&from == &to || amount <= 0) throw std::invalid_argument("distinct account and positive amount required");
    std::unique_lock<std::mutex> lock1(from.mutex, std::defer_lock); // [한국어 해설] defer_lock은 아직 잠그지 않는다는 뜻이다. 두 락의 취득을 std::lock에 맡긴다.
    std::unique_lock<std::mutex> lock2(to.mutex, std::defer_lock);
    std::lock(lock1, lock2); // deadlock-avoiding acquisition of both mutexes // [한국어 해설] 동시에 필요한 두 락을 교착 회피 알고리즘으로 얻는다. 공정성이나 완료 시한은 보장하지 않는다.
    if (from.coins >= amount) { // [한국어 해설] 잔액이 부족하면 아무 변화 없이 반환한다. 범용 은행 거래의 오버플로/오류 반환 API는 아니다.
        from.coins -= amount;
        to.coins += amount;
    }
}

static unsigned int xorshift32(unsigned int& state) { // [한국어 해설] 고정 시드의 테스트 입력 생성기다. 암호학적 난수도 OS 실행 순서 제어도 아니다.
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

int main() {
    try {
        Checks test;
        LockOrderGraph graph(4);
        test.expect(graph.allow(0, 1), "0 -> 1");
        test.expect(graph.allow(1, 2), "1 -> 2");
        test.expect(!graph.allow(2, 0), "reject 2 -> 0 cycle");
        test.expect(!graph.has(2, 0), "no rejected edge persisted");
        test.expect(!graph.allow(2, 2), "self-loop must be rejected");
        test.expect(graph.allow(0, 1), "duplicate is harmless");
        test.expect(graph.allow(2, 3), "chain extension");
        test.expect(graph.reachable(0, 3), "chain reachability");
        test.expect(!graph.reachable(3, 0), "no reverse reachability");
        bool invalid = false;
        try { (void)graph.allow(4, 0); }
        catch (const std::out_of_range&) { invalid = true; }
        test.expect(invalid, "out of bounds ID");
        invalid = false;
        try { (void)graph.reachable(0, 4); }
        catch (const std::out_of_range&) { invalid = true; }
        test.expect(invalid, "out of bounds reach query");

const std::size_t n = 7; // [한국어 해설] 7노드에서 12시드×256회=3072개의 간선 삽입 결정을 비교한다.
for (unsigned int seed = 1; seed <= 12; ++seed) {
    LockOrderGraph actual(n);
    std::vector<std::vector<bool> > oracle(n, std::vector<bool>(n, false));
    unsigned int state = seed * 747796405u + 2891336453u;
    for (int step = 0; step < 256; ++step) {
        const std::size_t from = static_cast<std::size_t>(xorshift32(state)) % n;
        const std::size_t to = static_cast<std::size_t>(xorshift32(state)) % n;
        const bool expected = from != to && (oracle[from][to] || !oracle_reachable(oracle, to, from));
        const bool accepted = actual.allow(from, to);
        test.expect(accepted == expected, "reference cycle oracle mismatch");
        if (accepted) oracle[from][to] = true;
        for (std::size_t v = 0; v < n; ++v) // [한국어 해설] 각 결정 뒤 7×7 간선 상태까지 대조하므로 expect 호출 수가 크게 늘어난다.
            for (std::size_t w = 0; w < n; ++w)
                test.expect(actual.has(v, w) == oracle[v][w], "edge persistence mismatch");
    }
}
std::array<Account, 4> accounts;
bool wrong_transfer = false;
try { transfer(accounts[0], accounts[0], 1); }
catch (const std::invalid_argument&) { wrong_transfer = true; }
test.expect(wrong_transfer, "self-transfer rejected");
wrong_transfer = false;
try { transfer(accounts[0], accounts[1], 0); }
catch (const std::invalid_argument&) { wrong_transfer = true; }
test.expect(wrong_transfer, "zero-transfer rejected");

std::vector<std::thread> workers; // [한국어 해설] 4개 실제 표준 스레드가 왕복 전송한다. 잠금 그래프 모델을 런타임에 자동 적용하지는 않는다.
for (std::size_t i = 0; i < accounts.size(); ++i) {
    workers.push_back(std::thread([&accounts, i]() {
        for (int j = 0; j < 1500; ++j) {
            transfer(accounts[i], accounts[(i + 1) % accounts.size()], 1);
            transfer(accounts[(i + 1) % accounts.size()], accounts[i], 1);
        }
    }));
}
        for (std::size_t i = 0; i < workers.size(); ++i) workers[i].join(); // [한국어 해설] 모든 작업이 끝난 뒤 잔액을 읽는다. join 전에 잠금 없이 읽으면 같은 계약이 아니다.
        long long total = 0;
        for (std::size_t i = 0; i < accounts.size(); ++i) {
            test.expect(accounts[i].coins >= 0, "balance must be nonnegative");
            total += accounts[i].coins;
        }
        test.expect(total == 40000, "concurrent transfers preserve total");
        std::cout << "PASS OS03 checks=" << test.n
                  << " random_order_attempts=" << (12 * 256)
                  << " concurrent_transfers=" << (4 * 1500 * 2) << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL OS03: " << e.what() << '\n';
        return 1;
    }
}
