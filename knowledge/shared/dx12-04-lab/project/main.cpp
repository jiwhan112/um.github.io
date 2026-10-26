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

// 정렬 덧셈과 범위 검사에 실패하면 원본 데이터를 변경하지 않는다.
std::uint64_t Align(std::uint64_t value, std::uint64_t alignment) {
    if (alignment == 0 || (alignment&(alignment-1)) != 0) throw std::invalid_argument("power of two");
    const auto extra = alignment-1;
    if (value > std::numeric_limits < std::uint64_t > ::max()-extra) throw std::overflow_error("alignment");
    return (value+extra)&~extra;
}
void Copy(std::vector < unsigned char > & dst, std::size_t offset, const std::vector < unsigned char > & src) {
    if (offset > dst.size() || src.size() > dst.size()-offset) throw std::out_of_range("buffer range");
    std::copy(src.begin(), src.end(), dst.begin()+static_cast < std::ptrdiff_t > (offset));
}
int main() {
    try {
        Check(Align(1, 256) == 256, "CBV alignment");
        Check(Align(256, 256) == 256, "already aligned");
        Check(Align(257, 256) == 512, "next slice");
        Check(Align(0, 256) == 0, "empty size");
        Reject([] { Align(7, 3); }, "not power of two");
        Reject([] { Align(7, 0); }, "zero align");
        Reject([] { Align(UINT64_MAX, 256); }, "overflow");
        std::vector < unsigned char > b(8, 0);
        Copy(b, 2, {7, 8});
        Check(b[2] == 7 && b[3] == 8, "copy bytes");
        auto old = b;
        Reject([&] { Copy(b, 7, {1, 2}); }, "range before mutation");
        Check(b == old, "preserve bytes");
        Copy(b, 8, {});
        Check(b == old, "empty end valid");
        std::cout << "PASS DX12-04: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
