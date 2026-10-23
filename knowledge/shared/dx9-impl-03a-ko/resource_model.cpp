// v1.1: checked index arithmetic and relative range repair. CPU only.
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

// A CPU-only teaching model. These states are NOT Direct3D 9 API return values.
enum class Pool { Default, Managed, SystemMemory };
enum class State { Operational, Lost, NotReset };

struct Resource {
    Pool pool;
    bool alive;
    std::uint64_t generation;
};

class DeviceModel { // [한국어 해설] CPU 상태 기계이며 열거값은 실제 HRESULT가 아니다. LOST→NOTRESET→Reset 조건을 학습한다.
public:
    State state = State::Operational;
    std::uint64_t generation = 1;
    std::vector<Resource> resources;

    std::size_t Create(Pool pool, bool dynamic = false) {
        if (state != State::Operational)
            throw std::logic_error("model: creation outside operational state");
        if (pool == Pool::Managed && dynamic)
            throw std::invalid_argument("model: dynamic managed VB/IB forbidden");
        resources.push_back({pool, true, generation});
        return resources.size() - 1;
    }
    void Lose() {
        if (state != State::Operational) throw std::logic_error("model: not operational");
        state = State::Lost;
    }
    void CanReset() {
        if (state != State::Lost) throw std::logic_error("model: not lost");
        state = State::NotReset;
    }
    void ReleaseDefaultResources() {
        for (Resource& r : resources)
            if (r.pool == Pool::Default) r.alive = false;
    }
    bool Reset() { // [한국어 해설] 모든 살아 있는 DEFAULT 자원을 해제한 경우에만 세대를 올리고 정상 상태로 돌아간다.
        if (state != State::NotReset) return false;
        for (const Resource& r : resources)
            if (r.pool == Pool::Default && r.alive) return false;
        ++generation;
        state = State::Operational;
        return true;
    }
    bool Usable(std::size_t id) const {
        if (id >= resources.size() || state != State::Operational) return false;
        const Resource& r = resources[id];
        return r.alive && (r.pool != Pool::Default || r.generation == generation);
    }
};

struct Vertex { float x, y, z, u, v; };
static_assert(sizeof(Vertex) == 5 * sizeof(float), "Example assumes 5 floats");

// Check a mathematical base+relative index without signed addition overflow.
// uintmax_t can represent size_t; -(base+1) avoids negating INT64_MIN.
bool AdjustedVertexFits(std::size_t relative, std::int64_t base, // [한국어 해설] base+상대 인덱스를 직접 더하기 전에 범위를 확인해 부호 오버플로를 피한다.
                        std::size_t totalVertices) {
    const std::uintmax_t raw = relative, count = totalVertices;
    if (base >= 0) {
        const auto offset = static_cast<std::uintmax_t>(base);
        return offset < count && raw < count - offset;
    }
    const auto magnitude = static_cast<std::uintmax_t>(-(base + 1)) + 1;
    return raw >= magnitude && raw - magnitude < count;
}

bool ValidateTriangleList(const std::vector<std::uint16_t>& indices, // [한국어 해설] 선언 범위도 BaseVertexIndex 기준 상대 범위다. 정점 수와 삼각형 수, 인덱스 개수를 구분한다.
                          std::size_t totalVertices,
                          std::size_t startIndex,
                          std::size_t primitiveCount,
                          std::int64_t baseVertex,
                          std::size_t minVertexIndex,
                          std::size_t numVertices) {
    // Zero draws are rejected by this teaching model's policy, not an API claim.
    if (primitiveCount == 0 || numVertices == 0 || totalVertices == 0)
        return false;
    if (startIndex > indices.size() ||
        primitiveCount > (indices.size() - startIndex) / 3)
        return false;
    // Both declared range endpoints are relative to BaseVertexIndex.
    if (numVertices - 1 > std::numeric_limits<std::size_t>::max() - minVertexIndex)
        return false;
    const std::size_t lastVertexIndex = minVertexIndex + (numVertices - 1);
    if (!AdjustedVertexFits(minVertexIndex, baseVertex, totalVertices) ||
        !AdjustedVertexFits(lastVertexIndex, baseVertex, totalVertices))
        return false;
    const std::size_t endIndex = startIndex + primitiveCount * 3;
    for (std::size_t i = startIndex; i < endIndex; ++i) {
        const std::size_t index = indices[i];
        if (index < minVertexIndex || index > lastVertexIndex) return false;
    }
    return true;
}

struct RingReservation {
    std::size_t offset;
    bool discard;
};

class DynamicRingModel { // [한국어 해설] 링 버퍼의 DISCARD/NOOVERWRITE 예약 정책 모형이다. GPU 사용 완료를 실제로 조회하지 않는다.
    std::size_t capacity_;
    std::size_t cursor_ = 0;
public:
    explicit DynamicRingModel(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) throw std::invalid_argument("zero capacity");
    }
    RingReservation Reserve(std::size_t count) {
        if (count == 0 || count > capacity_) throw std::invalid_argument("invalid count");
        if (count > capacity_ - cursor_) {
            cursor_ = count;
            return {0, true}; // conceptual DISCARD, new storage region
        }
        const bool discard = cursor_ == 0;
        const auto at = cursor_;
        cursor_ += count;
        return {at, discard}; // conceptual NOOVERWRITE when false
    }
};

int main() { // [한국어 해설] 복구 순서, 음수 base의 유효 사례, 극단 정수 입력과 링 버퍼 경계를 검사한다.
    int checks = 0;
    auto expect = [&](bool cond, const char* message) {
        ++checks;
        if (!cond) throw std::runtime_error(message);
    };
    DeviceModel dev;
    const auto defaultVB = dev.Create(Pool::Default, true);
    const auto managedTexture = dev.Create(Pool::Managed);
    const auto systemCopy = dev.Create(Pool::SystemMemory);
    expect(dev.Usable(defaultVB), "default initial");
    expect(dev.Usable(managedTexture), "managed initial");
    expect(dev.Usable(systemCopy), "system initial");
    try { dev.Create(Pool::Managed, true); expect(false, "dynamic managed accepted"); }
    catch (const std::invalid_argument&) { expect(true, "dynamic managed rejected"); }
    dev.Lose();
    expect(!dev.Usable(defaultVB), "lost unusable");
    expect(!dev.Reset(), "reset before ready rejected");
    dev.CanReset();
    expect(!dev.Reset(), "default must be released");
    dev.ReleaseDefaultResources();
    expect(dev.Reset(), "reset after release");
    expect(!dev.Usable(defaultVB), "old default remains invalid");
    expect(dev.Usable(managedTexture), "managed survives reset");
    expect(dev.Usable(systemCopy), "system memory survives reset");
    const auto newDefaultVB = dev.Create(Pool::Default, true);
    expect(dev.Usable(newDefaultVB), "new default ready");
    expect(newDefaultVB != defaultVB, "not reusing old handle");

    const std::vector<std::uint16_t> indices = {0, 1, 2, 0, 2, 3};
    expect(ValidateTriangleList(indices, 4, 0, 2, 0, 0, 4), "two triangles");
    expect(ValidateTriangleList(indices, 4, 3, 1, 0, 0, 4), "second triangle");
    expect(!ValidateTriangleList(indices, 4, 0, 3, 0, 0, 4), "excess index range");
    expect(!ValidateTriangleList(indices, 4, 7, 1, 0, 0, 4), "bad start index");
    expect(!ValidateTriangleList(indices, 4, 0, 2, 0, 0, 3), "index outside declared range");
    expect(!ValidateTriangleList(indices, 4, 0, 2, -1, 0, 4), "negative adjusted index");
    expect(!ValidateTriangleList(indices, 4, 0, 2, 1, 0, 4), "adjusted index out of VB");
    expect(!ValidateTriangleList(indices, 4, 0, 0, 0, 0, 4), "zero primitives");

    // v1.1: six additional boundary/contract regression checks.
    const std::vector<std::uint16_t> shifted = {3, 4, 5};
    expect(ValidateTriangleList(shifted, 3, 0, 1, -3, 3, 3),
           "negative base with valid adjusted range");
    const std::vector<std::uint16_t> ones = {1, 1, 1};
    expect(!ValidateTriangleList(ones, 2, 0, 1,
        std::numeric_limits<std::int64_t>::max(), 0, 2), "INT64_MAX base rejected safely");
    expect(!ValidateTriangleList(ones, 2, 0, 1,
        std::numeric_limits<std::int64_t>::min(), 0, 2), "INT64_MIN base rejected safely");
    expect(!ValidateTriangleList(ones, 2, 0, 1, 0,
        std::numeric_limits<std::size_t>::max(), 2), "declared range overflow rejected");
    expect(!ValidateTriangleList(indices, 0, 0, 1, 0, 0, 1), "empty VB rejected");
    const std::vector<std::uint16_t> zeros = {0, 0, 0};
    expect(ValidateTriangleList(zeros, 1, 0, 1, 0, 0, 1),
           "repeated index is in range; geometry degeneracy is a separate question");

    DynamicRingModel ring(6);
    const auto a = ring.Reserve(4), b = ring.Reserve(2), c = ring.Reserve(3);
    expect(a.offset == 0 && a.discard, "first discard");
    expect(b.offset == 4 && !b.discard, "append nooverwrite");
    expect(c.offset == 0 && c.discard, "wrap discard");
    try { ring.Reserve(7); expect(false, "oversize accepted"); }
    catch (const std::invalid_argument&) { expect(true, "oversize rejected"); }
    try { ring.Reserve(0); expect(false, "zero accepted"); }
    catch (const std::invalid_argument&) { expect(true, "zero rejected"); }
    std::cout << "PASS: " << checks << " assertions (CPU model; not D3D9 GPU)\n";
}
