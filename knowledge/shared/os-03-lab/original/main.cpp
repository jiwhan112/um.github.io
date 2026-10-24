#include <algorithm>
#include <array>
#include <cstddef>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

struct Checks {
    unsigned long long n;
    Checks() : n(0) {}
    void expect(bool good, const char* why) {
        ++n;
        if (!good) throw std::runtime_error(why);
    }
};

// Models lock-order acquisition constraints, not operating system waiting.
class LockOrderGraph {
    std::vector<std::vector<bool> > edge_;
public:
    explicit LockOrderGraph(std::size_t count)
        : edge_(count, std::vector<bool>(count, false)) {}

    bool reachable(std::size_t from, std::size_t to) const {
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
    bool allow(std::size_t held, std::size_t next) {
        if (held >= edge_.size() || next >= edge_.size())
            throw std::out_of_range("lock ID");
        if (held == next) return false; // non-recursive mutex self-acquisition
        if (edge_[held][next]) return true; // idempotent constraint
        if (reachable(next, held)) return false; // would create a cycle
        edge_[held][next] = true;
        return true;
    }
    bool has(std::size_t held, std::size_t next) const { return edge_.at(held).at(next); }
    std::size_t size() const { return edge_.size(); }
};

// Independent transitive closure oracle for deterministic small graphs.
static bool oracle_reachable(const std::vector<std::vector<bool> >& edge,
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
struct Account {
    std::mutex mutex;
    long long coins;
    Account() : coins(10000) {}
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;
};

static void transfer(Account& from, Account& to, long long amount) {
    if (&from == &to || amount <= 0) throw std::invalid_argument("distinct account and positive amount required");
    std::unique_lock<std::mutex> lock1(from.mutex, std::defer_lock);
    std::unique_lock<std::mutex> lock2(to.mutex, std::defer_lock);
    std::lock(lock1, lock2); // deadlock-avoiding acquisition of both mutexes
    if (from.coins >= amount) {
        from.coins -= amount;
        to.coins += amount;
    }
}

static unsigned int xorshift32(unsigned int& state) {
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

const std::size_t n = 7;
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
        for (std::size_t v = 0; v < n; ++v)
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

std::vector<std::thread> workers;
for (std::size_t i = 0; i < accounts.size(); ++i) {
    workers.push_back(std::thread([&accounts, i]() {
        for (int j = 0; j < 1500; ++j) {
            transfer(accounts[i], accounts[(i + 1) % accounts.size()], 1);
            transfer(accounts[(i + 1) % accounts.size()], accounts[i], 1);
        }
    }));
}
        for (std::size_t i = 0; i < workers.size(); ++i) workers[i].join();
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
