// DX11 13B: CPU-only input-layout bounds and overlap validator.
// C++11 and C++20 compatible; no Direct3D or Windows API calls.
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

enum class Result { Ok, InvalidLayout, InvalidElement, Overlap };
struct Element {
    std::uint32_t slot, offset, bytes, stepRate;
    bool perInstance;
};
struct Layout {
    std::uint32_t vertexStride, instanceStride;
    std::vector<Element> elements;
};

Result validate(const Layout& x) {
    if (x.vertexStride == 0 || x.instanceStride == 0 || x.elements.empty()) {
        return Result::InvalidLayout;
    }
    // First pass: reject malformed elements before doing interval arithmetic.
    for (std::size_t i = 0; i < x.elements.size(); ++i) {
        const Element& e = x.elements[i];
        if (e.slot > 1) return Result::InvalidElement;
        const std::uint32_t stride = e.slot == 0 ? x.vertexStride : x.instanceStride;
        if (e.bytes == 0 || e.offset > stride || e.bytes > stride - e.offset) {
            return Result::InvalidElement;
        }
        if (e.perInstance != (e.slot == 1)) return Result::InvalidElement;
        // Pedagogical contract: every per-instance element advances once.
        // Real D3D11 permits nonzero step rates greater than one.
        if (e.stepRate != (e.perInstance ? 1u : 0u)) return Result::InvalidElement;
    }
    // Second pass: end <= stride is now guaranteed. Wide ends add clarity.
    for (std::size_t i = 0; i < x.elements.size(); ++i) {
        for (std::size_t j = i + 1; j < x.elements.size(); ++j) {
            const Element& a = x.elements[i];
            const Element& b = x.elements[j];
            if (a.slot != b.slot) continue;
            const std::uint64_t endA = std::uint64_t(a.offset) + a.bytes;
            const std::uint64_t endB = std::uint64_t(b.offset) + b.bytes;
            if (std::uint64_t(a.offset) < endB && std::uint64_t(b.offset) < endA) {
                return Result::Overlap;
            }
        }
    }
    return Result::Ok;
}

// Small model of the old single-pass branch: unsafe ordering of diagnostics,
// not undefined behavior (unsigned wrap itself is defined by C++).
Result oldOrder(const Layout& x) {
    if (!x.vertexStride || !x.instanceStride || x.elements.empty()) return Result::InvalidLayout;
    for (std::size_t i = 0; i < x.elements.size(); ++i) {
        const Element a = x.elements[i];
        const std::uint32_t stride = a.slot == 0 ? x.vertexStride : a.slot == 1 ? x.instanceStride : 0;
        if (!stride || !a.bytes || a.offset > stride || a.bytes > stride-a.offset ||
            a.perInstance != (a.slot == 1) || a.stepRate != (a.perInstance ? 1u : 0u)) return Result::InvalidElement;
        for (std::size_t j = i+1; j < x.elements.size(); ++j) {
            const Element b = x.elements[j]; // b has not been bounds-checked.
            if (a.slot == b.slot && a.offset < b.offset+b.bytes &&
                b.offset < a.offset+a.bytes) return Result::Overlap;
        }
    }
    return Result::Ok;
}

int main() {
    unsigned checked = 0;
    const auto expect = [&](const Layout& layout, Result result, const char* name) {
        ++checked;
        if (validate(layout) != result) throw std::runtime_error(name);
    };
    const Element v0 = {0, 0, 12, 0, false};
    const Element v1 = {0, 12, 8, 0, false};
    const Element i0 = {1, 0, 16, 1, true};
    expect({20, 64, {v0, v1, i0}}, Result::Ok, "standard valid");
    expect({20, 64, {v0, v1}}, Result::Ok, "adjacent intervals");
    expect({20, 64, {v0, {0, 11, 8, 0, false}}}, Result::Overlap, "real overlap");
    expect({20, 64, {v0, {1, 0, 8, 1, true}}}, Result::Ok, "different slots");
    expect({0, 64, {v0}}, Result::InvalidLayout, "zero vertex stride");
    expect({20, 0, {v0}}, Result::InvalidLayout, "zero instance stride");
    expect({20, 64, {}}, Result::InvalidLayout, "empty layout");
    expect({20, 64, {{2, 0, 4, 0, false}}}, Result::InvalidElement, "slot not in model");
    expect({20, 64, {{0, 0, 0, 0, false}}}, Result::InvalidElement, "zero byte width");
    expect({20, 64, {{0, 21, 1, 0, false}}}, Result::InvalidElement, "offset beyond stride");
    expect({20, 64, {{0, 20, 1, 0, false}}}, Result::InvalidElement, "at end plus one");
    expect({20, 64, {{0, 19, 2, 0, false}}}, Result::InvalidElement, "width beyond rest");
    expect({20, 64, {{0, 0, 4, 0, true}}}, Result::InvalidElement, "wrong input class");
    expect({20, 64, {{0, 0, 4, 1, false}}}, Result::InvalidElement, "vertex step rate");
    expect({20, 64, {{1, 0, 4, 2, true}}}, Result::InvalidElement, "model expects instance step rate 1");
    const std::uint32_t m = (std::numeric_limits<std::uint32_t>::max)();
    expect({m, m, {{0, m-1, 1, 0, false}}}, Result::Ok, "valid at extreme stride");
    expect({m, m, {{0, 0, m, 0, false}, {0, m-2, 10, 0, false}}},
           Result::InvalidElement, "bad second element classified before overlap");
    expect({m, m, {{0, m-2, 10, 0, false}, {0, 0, m, 0, false}}},
           Result::InvalidElement, "bad first element rejected");
    expect({m, m, {{0, 0, m, 0, false}}}, Result::Ok, "maximal single range");
    expect({20, 64, {{0, 18, 2, 0, false}, {0, 19, 1, 0, false}}},
           Result::Overlap, "overlap at last byte");
    ++checked;
    const Layout adversarial = {m, m, {{0, 0, m, 0, false}, {0, m-2, 10, 0, false}}};
    if (oldOrder(adversarial) != Result::Overlap || validate(adversarial) != Result::InvalidElement)
        throw std::runtime_error("old/new classification counterexample");
    std::cout << "PASS " << checked << " unique layout contract checks\n";
}
