// DX11 13B: educational CPU-only InputLayout contract; not GPU/API execution.
// C++11 source, tested with C++11 and C++20.
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

struct Element {
    std::uint32_t slot, offset, bytes, step;
    bool per_instance;
};
struct Layout {
    std::uint32_t vertex_stride, instance_stride;
    std::vector<Element> elements;
};

void validate(const Layout& l) { // [한국어 해설] 모든 원소 범위를 먼저 검증하고 안전한 끝점끼리 겹침을 비교한다.
    if (!l.vertex_stride || !l.instance_stride || l.elements.empty())
        throw std::invalid_argument("missing stream");
    // Stage 1: validate ALL elements before calculating any end offsets.
    for (std::size_t i = 0; i < l.elements.size(); ++i) {
        const Element& e = l.elements[i];
        if (e.slot > 1 || !e.bytes || e.per_instance != (e.slot == 1) ||
            (e.per_instance ? e.step == 0 : e.step != 0))
            throw std::invalid_argument("slot or class");
        const std::uint32_t stride = e.slot == 0 ? l.vertex_stride : l.instance_stride;
        if (e.offset > stride || e.bytes > stride - e.offset)
            throw std::out_of_range("element beyond stride");
    }
    // Stage 2: safe sums, because Stage 1 proved offset+bytes <= stride. // [한국어 해설] 이 축약판은 per-instance step이 양수면 허용한다. 13B 상세판의 step=1 정책과 구별한다.
    for (std::size_t i = 0; i < l.elements.size(); ++i)
        for (std::size_t j = i + 1; j < l.elements.size(); ++j) {
            const Element& a = l.elements[i];
            const Element& b = l.elements[j];
            if (a.slot == b.slot && a.offset < b.offset + b.bytes &&
                b.offset < a.offset + a.bytes)
                throw std::invalid_argument("overlapping elements");
        }
}

bool valid(const Layout& l) { // [한국어 해설] 검증 예외를 bool로 바꾼 편의 함수다. 메모리 할당 등 모든 예외를 성공/실패 bool로 숨기지 않는다.
    try { validate(l); return true; }
    catch (const std::invalid_argument&) { return false; }
    catch (const std::out_of_range&) { return false; }
}
int main() { // [한국어 해설] 극단 오프셋·겹침·서로 다른 슬롯과 stepRate2를 포함하는13개 CPU 검사다.
    unsigned checks = 0;
    const auto expect = [&](bool result) {
        ++checks;
        if (!result) throw std::runtime_error("layout assertion failed");
    };
    expect(valid({20,64,{{0,0,12,0,false},{0,12,8,0,false},
         {1,0,16,2,true},{1,16,16,2,true},{1,32,16,2,true},{1,48,16,2,true}}}));
    expect(!valid({0,64,{{0,0,4,0,false}}}));
    expect(!valid({20,64,{}}));
    expect(!valid({20,64,{{2,0,4,0,false}}}));
    expect(!valid({20,64,{{0,0,0,0,false}}}));
    expect(!valid({20,64,{{0,0,4,1,false}}}));
    expect(!valid({20,64,{{1,0,4,0,true}}}));
    expect(!valid({20,64,{{0,20,1,0,false}}}));
    expect(!valid({20,64,{{0,4294967292U,16,0,false}}}));
    expect(!valid({20,64,{{0,0,12,0,false},{0,8,8,0,false}}}));
    expect(valid({20,64,{{0,0,12,0,false},{0,12,8,0,false}}}));
    expect(valid({20,64,{{0,0,12,0,false},{1,0,12,1,true}}}));
    expect(!valid({20,64,{{0,0,12,0,false},{1,63,2,1,true}}}));
    std::cout << "PASS " << checks << " CPU InputLayout checks\n";
}
