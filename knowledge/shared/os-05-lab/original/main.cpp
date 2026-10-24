// OS05: Deterministic CPU reference for cache line mapping and LRU replacement.
// Portable C++11 / C++20. NOT a hardware cache, virtual-memory, or latency test.
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace os05 {
struct Stats {
    std::uint64_t hits;
    std::uint64_t misses;
    std::uint64_t evictions;
    Stats() : hits(0), misses(0), evictions(0) {}
};

struct Line {
    bool valid;
    std::uint64_t block;
    std::uint64_t recently_used;
    Line() : valid(false), block(0), recently_used(0) {}
};

class CacheModel {
public:
    CacheModel(std::size_t line_count, std::size_t associativity)
        : ways_(validate(line_count, associativity)),
          sets_(line_count / ways_, std::vector<Line>(ways_)), clock_(0) {}

    bool touch(std::uint64_t memory_block) {
        // A real CPU first computes the block address from byte address/line size.
        // Here the input IS an already-computed memory block number.
        std::vector<Line>& bucket = sets_[memory_block % sets_.size()];
        ++clock_; // Finite educational traces, not a wraparound-safe production clock.
        for (std::size_t i = 0; i < bucket.size(); ++i) {
            if (bucket[i].valid && bucket[i].block == memory_block) {
                ++stats_.hits;
                bucket[i].recently_used = clock_;
                return true;
            }
        }
        ++stats_.misses;
        std::size_t replacement = 0;
        for (std::size_t i = 0; i < bucket.size(); ++i) {
            if (!bucket[i].valid) {
                replacement = i;
                install(bucket[replacement], memory_block);
                return false;
            }
            if (bucket[i].recently_used < bucket[replacement].recently_used)
                replacement = i;
        }
        ++stats_.evictions;
        install(bucket[replacement], memory_block);
        return false;
    }

    Stats stats() const { return stats_; }
    std::size_t set_count() const { return sets_.size(); }
    std::size_t associativity() const { return ways_; }

private:
    static std::size_t validate(std::size_t lines, std::size_t ways) {
        if (lines == 0 || ways == 0 || ways > lines || lines % ways != 0)
            throw std::invalid_argument("line_count and associativity must be positive and divisible");
        return ways;
    }
    void install(Line& line, std::uint64_t block) {
        line.valid = true;
        line.block = block;
        line.recently_used = clock_;
    }
    std::size_t ways_;
    std::vector<std::vector<Line> > sets_;
    std::uint64_t clock_;
    Stats stats_;
};
}  // namespace os05

int main() {
    unsigned checks = 0;
    unsigned failed = 0;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failed; std::cerr << "FAIL " << #expr << " at line " << __LINE__ << '\n'; } } while (0)
    using os05::CacheModel;
    {
        bool rejected = false;
        try { CacheModel bad(0, 1); (void)bad; }
        catch (const std::invalid_argument&) { rejected = true; }
        CHECK(rejected);
        rejected = false;
        try { CacheModel bad(3, 2); (void)bad; }
        catch (const std::invalid_argument&) { rejected = true; }
        CHECK(rejected);
        rejected = false;
        try { CacheModel bad(4, 0); (void)bad; }
        catch (const std::invalid_argument&) { rejected = true; }
        CHECK(rejected);
    }
    {
        CacheModel direct(4, 1);
        CHECK(direct.set_count() == 4);
        CHECK(direct.associativity() == 1);
        CHECK(!direct.touch(0));
        CHECK(!direct.touch(4));
        CHECK(!direct.touch(0));
        CHECK(!direct.touch(4));
        CHECK(direct.stats().hits == 0);
        CHECK(direct.stats().misses == 4);
        CHECK(direct.stats().evictions == 3);
    }
    {
        CacheModel two_way(4, 2);
        CHECK(two_way.set_count() == 2);
        CHECK(!two_way.touch(0));
        CHECK(!two_way.touch(4));
        CHECK(two_way.touch(0));
        CHECK(two_way.touch(4));
        CHECK(two_way.stats().hits == 2);
        CHECK(two_way.stats().misses == 2);
        CHECK(two_way.stats().evictions == 0);
    }
    {
        CacheModel lru(2, 2);
        CHECK(!lru.touch(10));
        CHECK(!lru.touch(11));
        CHECK(lru.touch(10));
        CHECK(!lru.touch(12)); // 11 was least recently used.
        CHECK(!lru.touch(11)); // 10 is evicted now.
        CHECK(lru.stats().hits == 1);
        CHECK(lru.stats().misses == 4);
        CHECK(lru.stats().evictions == 2);
        CHECK(lru.stats().hits + lru.stats().misses == 5);
    }
    {
        CacheModel large(8, 4);
        const std::uint64_t max_block = std::numeric_limits<std::uint64_t>::max();
        CHECK(!large.touch(max_block));
        CHECK(large.touch(max_block));
        CHECK(large.stats().hits == 1);
        CHECK(large.stats().misses == 1);
        CHECK(large.stats().evictions == 0);
    }
#undef CHECK
    std::cout << "OS05 CPU cache mapping: " << checks << " distinct logical assertions; failed=" << failed << '\n';
    return failed == 0 ? 0 : 1;
}
