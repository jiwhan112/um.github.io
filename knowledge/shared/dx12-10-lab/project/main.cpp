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

std::vector < unsigned char > Pack(const std::vector < unsigned char > & rgba, unsigned width, unsigned height) {
    if (width == 0 || height == 0 || width > 4096 || height > 4096) throw std::invalid_argument("bounded image");
    const std::size_t row = static_cast < std::size_t > (width)*4;
    if (rgba.size() != row*height) throw std::invalid_argument("source byte count");
    const std::size_t pitch = (row+255)&~std::size_t(255); // RGBA8/mip0 제한 모델
    std::vector < unsigned char > upload(512+pitch*height, 0xCC);
    for (unsigned y = 0;y < height;++y)
        std::copy_n(rgba.begin()+static_cast < std::ptrdiff_t > (y*row), row,
                    upload.begin()+static_cast < std::ptrdiff_t > (512+y*pitch));
    return upload;
}
int main() {
    try {
        std::vector < unsigned char > rgba(24);
        for (unsigned i = 0;i < rgba.size();++i) rgba[i] = static_cast < unsigned char > (i);
        auto upload = Pack(rgba, 3, 2);
        Check(upload.size() == 1024, "two padded rows plus offset");
        Check(upload[511] == 0xCC, "placed offset prefix untouched");
        Check(upload[512] == 0 && upload[523] == 11, "first row");
        Check(upload[524] == 0xCC && upload[767] == 0xCC, "padding not pixel data");
        Check(upload[768] == 12 && upload[779] == 23, "second row stride");
        Reject([&] { Pack(rgba, 0, 2); }, "zero width");
        Reject([&] { Pack(rgba, 3, 1); }, "byte count mismatch");
        Reject([&] { Pack(rgba, 4097, 1); }, "bounded dimension");
        auto aligned = Pack(std::vector < unsigned char > (256, 7), 64, 1);
        Check(aligned.size() == 768 && aligned.back() == 7, "exact row alignment");
        std::cout << "PASS DX12-10: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
