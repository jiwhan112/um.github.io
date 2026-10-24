// OS05: Deterministic CPU reference for cache line mapping and LRU replacement.
// Portable C++11 / C++20. NOT a hardware cache, virtual-memory, or latency test.
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace os05 {
struct Stats { // 한국어: hit·miss·교체를 별도로 집계한다. miss가 항상 교체인 것은 아니다.
    std::uint64_t hits;
    std::uint64_t misses;
    std::uint64_t evictions;
    Stats() : hits(0), misses(0), evictions(0) {}
};

struct Line { // 한국어: valid가 false인 슬롯에는 아직 유효한 블록이 없다.
    bool valid;
    std::uint64_t block;
    std::uint64_t recently_used;
    Line() : valid(false), block(0), recently_used(0) {}
};

class CacheModel {
public:
    CacheModel(std::size_t line_count, std::size_t associativity) // 한국어: ways 검증이 먼저 수행되어 0으로 나누는 구성을 막는다.
        : ways_(validate(line_count, associativity)),
          sets_(line_count / ways_, std::vector<Line>(ways_)), clock_(0) {} // 한국어: 집합 수 × 웨이 수가 전체 라인 수가 된다.

    bool touch(std::uint64_t memory_block) { // 한국어: 주소 바이트가 아니라 이미 계산된 메모리 블록 번호를 받는다.
        // A real CPU first computes the block address from byte address/line size.
        // Here the input IS an already-computed memory block number.
        std::vector<Line>& bucket = sets_[memory_block % sets_.size()]; // 한국어: 나머지 연산으로 한 집합만 선택한다.
        ++clock_; // Finite educational traces, not a wraparound-safe production clock. // 한국어: 교육용 유한 접근열만 다루며 시계 래핑은 해결하지 않는다.
        for (std::size_t i = 0; i < bucket.size(); ++i) {
            if (bucket[i].valid && bucket[i].block == memory_block) { // 한국어: 같은 집합에서 유효한 동일 블록을 찾아야 hit다.
                ++stats_.hits;
                bucket[i].recently_used = clock_; // 한국어: hit에서도 최근 사용 시각을 갱신해야 진짜 LRU가 된다.
                return true;
            }
        }
        ++stats_.misses; // 한국어: 없었던 블록의 접근은 miss로 기록한다.
        std::size_t replacement = 0;
        for (std::size_t i = 0; i < bucket.size(); ++i) {
            if (!bucket[i].valid) { // 한국어: 빈 웨이가 있으면 퇴출 없이 먼저 사용한다.
                replacement = i;
                install(bucket[replacement], memory_block);
                return false;
            }
            if (bucket[i].recently_used < bucket[replacement].recently_used) // 한국어: 가장 오래 사용하지 않은 유효 웨이를 후보로 고른다.
                replacement = i;
        }
        ++stats_.evictions; // 한국어: 모든 웨이가 찬 miss만 기존 블록을 퇴출한다.
        install(bucket[replacement], memory_block);
        return false;
    }

    Stats stats() const { return stats_; }
    std::size_t set_count() const { return sets_.size(); }
    std::size_t associativity() const { return ways_; }

private:
    static std::size_t validate(std::size_t lines, std::size_t ways) { // 한국어: 양수·나눗셈 가능한 배치가 입력 계약이다.
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
        CacheModel direct(4, 1); // 한국어: 전체4라인·1way면4집합이며 0과4는 같은 집합이다.
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
        CacheModel two_way(4, 2); // 한국어: 전체4라인·2way면2집합이지만 0과4를 함께 보관할 수 있다.
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
        CacheModel lru(2, 2); // 한국어: 2라인·2way의 한 집합으로 최근 사용 갱신을 확인한다.
        CHECK(!lru.touch(10));
        CHECK(!lru.touch(11));
        CHECK(lru.touch(10)); // 한국어: 10을 다시 읽어 11이 가장 오래된 블록이 된다.
        CHECK(!lru.touch(12)); // 11 was least recently used.
        CHECK(!lru.touch(11)); // 10 is evicted now.
        CHECK(lru.stats().hits == 1);
        CHECK(lru.stats().misses == 4);
        CHECK(lru.stats().evictions == 2);
        CHECK(lru.stats().hits + lru.stats().misses == 5);
    }
    {
        CacheModel large(8, 4);
        const std::uint64_t max_block = std::numeric_limits<std::uint64_t>::max(); // 한국어: 최대 블록 번호도 유효하며 바이트주소로 곱하지 않는다.
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
