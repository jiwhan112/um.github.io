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

// 패딩 바이트 대신 의미 있는 상태 필드로만 PSO 캐시 키를 비교한다.
struct Key {
    unsigned root, shader, format, samples;
    bool depth;
    bool operator < (const Key& b) const {
        // 객체 패딩을 해시하거나 memcmp 하지 않는다. 의미 있는 필드를 비교한다.
        const auto aFields = std::array < unsigned, 5 > {{root, shader, format, samples, depth?1u:0u}};
        const auto bFields = std::array < unsigned, 5 > {{b.root, b.shader, b.format, b.samples, b.depth?1u:0u}};
        return aFields < bFields;
    }
};
void Validate(const Key& k, unsigned targetFormat, unsigned targetSamples) {
    if (k.samples == 0 || k.format != targetFormat || k.samples != targetSamples)
        throw std::invalid_argument("PSO target contract");
}
int main() {
    try {
        Key k{1, 2, 28, 1, false};
        std::map < Key, unsigned > cache;
        cache[k] = 7;
        Check(cache.at(Key{1, 2, 28, 1, false}) == 7, "same semantic key");
        cache[Key{1, 3, 28, 1, false}] = 8;
        Check(cache.size() == 2, "shader changes key");
        cache[Key{1, 2, 28, 4, false}] = 9;
        Check(cache.size() == 3, "MSAA changes key");
        cache[Key{1, 2, 28, 1, true}] = 10;
        Check(cache.size() == 4, "depth changes key");
        Validate(k, 28, 1);
        Check(true, "target matches");
        Reject([&] { Validate(k, 87, 1); }, "format mismatch");
        Reject([&] { Validate(k, 28, 4); }, "sample mismatch");
        Reject([] { Validate(Key{1, 2, 28, 0, false}, 28, 0); }, "invalid samples");
        Check(!(k < k), "strict comparator irreflexive");
        std::cout << "PASS DX12-07: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
