#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

// C++11 compatible. 64 bytes is a lab assumption, NOT detected cache-line size.
struct NearCell { std::atomic<unsigned> n; NearCell() : n(0) {} };
struct alignas(64) FarCell { std::atomic<unsigned> n; FarCell() : n(0) {} };
struct Near { NearCell a,b; };
struct Far { FarCell a,b; };

template<class T>
std::pair<unsigned,unsigned> count(T& v, unsigned repetitions, long long& ns) {
    const auto start = std::chrono::steady_clock::now();
    std::thread a([&] {
        for (unsigned i = 0; i < repetitions; ++i)
            v.a.n.fetch_add(1, std::memory_order_relaxed);
    });
    std::thread b([&] {
        for (unsigned i = 0; i < repetitions; ++i)
            v.b.n.fetch_add(1, std::memory_order_relaxed);
    });
    a.join();
    b.join();
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - start).count();
    return std::make_pair(v.a.n.load(std::memory_order_relaxed),
                          v.b.n.load(std::memory_order_relaxed));
}

int main(int argc, char** argv) {
    if (argc != 2 || std::string(argv[1]) != "--test") return 2;
    int checks = 0;
    int failed = 0;
    const auto verify = [&](bool ok) {
        ++checks;
        if (!ok) ++failed;
    };
    verify(alignof(FarCell) >= 64);
    verify(sizeof(FarCell) % 64 == 0);
    verify(sizeof(Far) >= 2 * sizeof(FarCell));
    const unsigned cases[] = {0, 1, 7, 10000};
    for (unsigned n : cases) {
        Near nearby;
        Far separated;
        long long near_ns = 0, far_ns = 0;
        const auto x = count(nearby, n, near_ns);
        const auto y = count(separated, n, far_ns);
        verify(x.first == n && x.second == n);
        verify(y.first == n && y.second == n);
        verify(x == y);
        if (n == 10000)
            std::cout << "OBSERVE ONLY near_ns=" << near_ns
                      << " far_ns=" << far_ns << '\n';
    }
    std::cout << (failed ? "FAIL " : "PASS ") << checks
              << " OS06 deterministic checks\n";
    return failed ? 1 : 0;
}
