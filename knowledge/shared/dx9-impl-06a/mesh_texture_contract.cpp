// DX9 VB/IB and texture upload contract - CPU-only executable test (C++11).
// This is NOT a Direct3D9 implementation or an emulated GPU.
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

struct VertexPT { float x, y, z, u, v; };
static_assert(sizeof(VertexPT) == 20, "VertexPT must be 20 bytes");

bool ValidateTriangleList(const std::vector<VertexPT>& vertices,
                          const std::vector<std::uint16_t>& indices,
                          std::size_t startIndex, std::size_t primitiveCount,
                          std::size_t minIndex, std::size_t numVertices,
                          std::int32_t baseVertexIndex) {
    if (vertices.empty() || numVertices == 0 || indices.empty()) return false;
    if (startIndex > indices.size()) return false;
    if (primitiveCount > (indices.size() - startIndex) / 3) return false;
    if (minIndex > std::numeric_limits<std::size_t>::max() - numVertices) return false;
    const std::size_t endIndex = minIndex + numVertices; // exclusive, raw index range
    const auto adjustedFits = [&](std::size_t raw) {
        const std::uintmax_t value = raw, count = vertices.size();
        const std::int64_t base = baseVertexIndex;
        if (base >= 0) {
            const auto offset = static_cast<std::uintmax_t>(base);
            return offset < count && value < count - offset;
        }
        const auto magnitude = static_cast<std::uintmax_t>(-base); // base is int32
        return value >= magnitude && value - magnitude < count;
    };
    if (!adjustedFits(minIndex) || !adjustedFits(endIndex - 1)) return false;

    for (std::size_t n = startIndex; n < startIndex + primitiveCount * 3; ++n) {
        const std::size_t raw = indices[n];
        if (raw < minIndex || raw >= endIndex) return false;
        const std::int64_t effective = static_cast<std::int64_t>(baseVertexIndex) +
            static_cast<std::int64_t>(raw); // raw is a uint16 index
        if (effective < 0 || static_cast<std::uint64_t>(effective) >= vertices.size()) return false;
    }
    return primitiveCount != 0;
}

enum class Pool { Default, Managed, SystemMem };
struct BufferSpec { bool dynamic; Pool pool; std::size_t bytes; };
bool ValidVertexBufferSpec(const BufferSpec& spec) {
    if (!spec.bytes) return false;
    // This models the documented D3D9 dynamic VB/IB + MANAGED prohibition.
    return !(spec.dynamic && spec.pool == Pool::Managed);
}

bool CopyImageRows(const std::vector<std::uint8_t>& src,
                   std::size_t width, std::size_t height,
                   std::size_t bytesPerPixel, std::size_t destinationPitch,
                   std::vector<std::uint8_t>& destination) {
    // In-place expansion can overwrite a later source row; reject aliasing first.
    if (&src == &destination) return false;
    if (!width || !height || !bytesPerPixel) return false;
    if (width > std::numeric_limits<std::size_t>::max() / bytesPerPixel) return false;
    const std::size_t rowBytes = width * bytesPerPixel;
    if (destinationPitch < rowBytes) return false;
    if (height > std::numeric_limits<std::size_t>::max() / rowBytes) return false;
    if (src.size() < rowBytes * height) return false;
    if (height > std::numeric_limits<std::size_t>::max() / destinationPitch) return false;
    if (destination.size() < destinationPitch * height) return false;
    for (std::size_t row = 0; row < height; ++row)
        std::memcpy(destination.data() + row * destinationPitch,
                    src.data() + row * rowBytes, rowBytes);
    return true;
}

int checks = 0;
void Check(bool condition, const char* label) {
    ++checks;
    if (!condition) throw std::runtime_error(label);
}

int main() {
    try {
        const std::vector<VertexPT> vertices(4);
        const std::vector<std::uint16_t> indices = {0, 1, 2, 2, 1, 3};
        Check(sizeof(VertexPT) * vertices.size() == 80, "vertex byte size");
        Check(sizeof(std::uint16_t) * indices.size() == 12, "index byte size");
        Check(ValidateTriangleList(vertices, indices, 0, 2, 0, 4, 0), "valid indexed rect");
        Check(!ValidateTriangleList(vertices, indices, 0, 3, 0, 4, 0), "out of index range");
        Check(!ValidateTriangleList(vertices, indices, 4, 1, 0, 4, 0), "start too late");
        Check(!ValidateTriangleList(vertices, indices, 0, 2, 0, 3, 0), "wrong NumVertices");
        Check(!ValidateTriangleList(vertices, indices, 0, 2, 1, 3, 0), "wrong MinIndex");
        Check(!ValidateTriangleList(vertices, indices, 0, 2, 0, 4, -1), "negative effective index");
        Check(!ValidateTriangleList(vertices, indices, 0, 2, 0, 4, 1), "effective index beyond VB");
        Check(!ValidateTriangleList(vertices, indices, 0, 0, 0, 4, 0), "zero primitives");
        Check(!ValidateTriangleList(vertices, indices, 0, 2,
              std::numeric_limits<std::size_t>::max(), 4, 0), "index range overflow");
        Check(ValidVertexBufferSpec({false, Pool::Managed, 80}), "managed static allowed");
        Check(ValidVertexBufferSpec({true, Pool::Default, 80}), "default dynamic allowed");
        Check(!ValidVertexBufferSpec({true, Pool::Managed, 80}), "managed dynamic rejected");
        Check(!ValidVertexBufferSpec({false, Pool::Default, 0}), "zero byte buffer rejected");
        std::vector<std::uint8_t> source = {1,2,3,4, 5,6,7,8};
        std::vector<std::uint8_t> dest(16, 0xcc);  // 2 rows; pitch 8, row content 4
        Check(CopyImageRows(source, 1, 2, 4, 8, dest), "pitched texture copy");
        Check(dest[0] == 1 && dest[3] == 4 && dest[8] == 5 && dest[11] == 8,
              "both rows placed at destination pitch");
        Check(dest[4] == 0xcc && dest[12] == 0xcc, "destination padding preserved");
        Check(!CopyImageRows(source, 1, 2, 4, 3, dest), "pitch smaller than row rejected");
        Check(!CopyImageRows(source, 1, 3, 4, 8, dest), "undersized source rejected");
        std::vector<std::uint8_t> smallDest(12, 0);
        Check(!CopyImageRows(source, 1, 2, 4, 8, smallDest), "undersized destination rejected");
        Check(!CopyImageRows(source, std::numeric_limits<std::size_t>::max(),
                            2, 4, 8, dest), "width overflow rejected");
        const std::vector<std::uint16_t> shifted = {3,4,5};
        Check(ValidateTriangleList(std::vector<VertexPT>(3), shifted, 0,1,3,3,-3),
              "negative base valid range");
        Check(!ValidateTriangleList(vertices, {0,0,0}, 0,1,0,5,0),
              "full declared range beyond VB rejected");
        std::vector<std::uint8_t> alias(24);
        for (unsigned i=0;i<alias.size();++i) alias[i]=static_cast<std::uint8_t>(i);
        const auto beforeAlias=alias;
        Check(!CopyImageRows(alias,1,3,4,8,alias), "in-place pitched copy rejected");
        Check(alias==beforeAlias, "alias rejection leaves storage unchanged");
        const auto beforeDest=dest;
        Check(!CopyImageRows(source,0,2,4,8,dest), "zero width rejected");
        Check(dest==beforeDest, "invalid width leaves destination unchanged");
        std::cout << "PASS " << checks << " CPU contract assertions\n";
    } catch (const std::exception& err) {
        std::cerr << "FAIL " << checks << ": " << err.what() << '\n';
        return 1;
    }
}
