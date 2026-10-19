#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

enum class Usage { Default, Immutable, Dynamic, Staging };
enum Bind : unsigned { Vertex = 1, Index = 2, Constant = 4, ShaderResource = 8, RenderTarget = 16 };
enum Cpu : unsigned { Read = 1, Write = 2 };
enum class Map { Read, Write, ReadWrite, WriteDiscard, WriteNoOverwrite };
struct BufferDesc {
    Usage usage;
    unsigned bind;
    unsigned cpu;
    std::uint32_t bytes;
    bool initial;
    BufferDesc(Usage u, unsigned b, unsigned c, std::uint32_t n, bool init)
        : usage(u), bind(b), cpu(c), bytes(n), initial(init) {}
};
bool IsPowerOfTwo(unsigned v) { return v && (v & (v-1)) == 0; }
// Deliberately limited teaching subset: no MiscFlags, structured/indirect buffers or 11.1 extended CB.
bool Valid(const BufferDesc& d) {
    if (!d.bytes) return false;
    if (d.bind & Constant) {
        if (d.bind != Constant || d.bytes % 16 != 0 || d.bytes > 65536) return false;
    }
    switch(d.usage) {
        case Usage::Default: return d.cpu == 0;
        case Usage::Immutable: return d.cpu == 0 && d.bind != 0 && d.initial;
        case Usage::Dynamic: return d.cpu == Write && d.bind != 0;
        case Usage::Staging: return d.bind == 0 && d.cpu != 0;
    }
    return false;
}
bool CanMap(const BufferDesc& d, Map m, bool runtime11_1=false) {
    if (!Valid(d)) return false;
    if (d.usage == Usage::Staging) {
        if(m == Map::Read) return (d.cpu & Read) != 0;
        if(m == Map::Write) return (d.cpu & Write) != 0;
        if(m == Map::ReadWrite) return d.cpu == (Read|Write);
        return false;
    }
    if (d.usage != Usage::Dynamic) return false;
    if (m == Map::WriteDiscard) return true;
    if (m == Map::WriteNoOverwrite) return (d.bind & (Vertex|Index)) != 0 ||
        (runtime11_1 && (d.bind & Constant) != 0);
    return false;
}
bool CanUpdateSubresource(const BufferDesc& d) {return Valid(d) && d.usage == Usage::Default;}
bool CanCreateView(unsigned resourceBind, unsigned requestedViewBind) {
    if (!IsPowerOfTwo(requestedViewBind) ||
        (requestedViewBind != ShaderResource && requestedViewBind != RenderTarget)) return false;
    return (resourceBind & requestedViewBind) != 0;
}
struct ByteRange {std::uint32_t offset, length;};
bool Fits(std::uint32_t capacity, ByteRange r) {
    return r.offset <= capacity && r.length <= capacity-r.offset;
}
struct DynamicRing {
    std::uint32_t size;
    std::uint32_t cursor = 0;
    explicit DynamicRing(std::uint32_t n) : size(n) {}
    bool DiscardReserve(std::uint32_t n, ByteRange& out) {
        if (!n || n > size) return false;
        cursor=n; out={0,n}; return true; // DISCARD: old contents undefined.
    }
    bool NoOverwriteReserve(std::uint32_t n, ByteRange& out) {
        if (!n || n>size-cursor) return false;
        out={cursor,n}; cursor += n; return true;
    }
};
int main() {
    unsigned checks=0;
    const auto check=[&](bool b, const char* why) {++checks;if(!b) throw std::runtime_error(why);};
    try {
        const BufferDesc def{Usage::Default,Vertex,0,80,false};
        const BufferDesc imm{Usage::Immutable,Index,0,12,true};
        const BufferDesc dyn{Usage::Dynamic,Vertex,Write,96,false};
        const BufferDesc cb{Usage::Dynamic,Constant,Write,64,false};
        const BufferDesc staging{Usage::Staging,0,Read|Write,64,false};
        check(Valid(def),"default VB");
        check(Valid(imm),"immutable IB initialized");
        check(!Valid({Usage::Immutable,Index,0,12,false}),"immutable initial required");
        check(!Valid({Usage::Immutable,Index,Write,12,true}),"immutable CPU access forbidden");
        check(Valid(dyn),"dynamic writable VB");
        check(!Valid({Usage::Dynamic,Vertex,Read,96,false}),"dynamic read forbidden");
        check(!Valid({Usage::Dynamic,Vertex,0,96,false}),"dynamic missing CPU write");
        check(!Valid({Usage::Default,Vertex,Write,96,false}),"default cannot CPU write");
        check(Valid(staging),"staging readwrite");
        check(!Valid({Usage::Staging,Vertex,Read,64,false}),"staging cannot bind");
        check(!Valid({Usage::Staging,0,0,64,false}),"staging needs CPU access");
        check(!Valid({Usage::Default,Vertex,0,0,false}),"zero allocation");
        check(Valid(cb),"dynamic constant buffer");
        check(!Valid({Usage::Default,Constant,0,20,false}),"CB 16 byte multiple");
        check(!Valid({Usage::Default,Constant|Vertex,0,32,false}),"CB exclusive bind");
        check(!Valid({Usage::Default,Constant,0,65552,false}),"legacy CB >64KiB");
        check(Valid({Usage::Default,Constant,0,65536,false}),"legacy CB max");
        check(CanMap(dyn,Map::WriteDiscard),"dynamic discard");
        check(CanMap(dyn,Map::WriteNoOverwrite),"dynamic VB no-overwrite");
        check(!CanMap(dyn,Map::Read),"dynamic can't CPU read");
        check(!CanMap(def,Map::Write),"default no map");
        check(CanUpdateSubresource(def),"default UpdateSubresource");
        check(!CanUpdateSubresource(dyn),"dynamic not UpdateSubresource in this model");
        check(CanMap(staging,Map::ReadWrite),"staging readwrite map");
        check(!CanMap(staging,Map::WriteDiscard),"staging not discard");
        check(!CanMap(cb,Map::WriteNoOverwrite,false),"11.0 CB nooverwrite unavailable");
        check(CanMap(cb,Map::WriteNoOverwrite,true),"11.1 CB nooverwrite capability assumed");
        check(CanCreateView(ShaderResource|RenderTarget,ShaderResource),"SRV bind flags");
        check(CanCreateView(ShaderResource|RenderTarget,RenderTarget),"RTV bind flags");
        check(!CanCreateView(ShaderResource,RenderTarget),"RTV missing bind flag");
        check(!CanCreateView(ShaderResource|RenderTarget,ShaderResource|RenderTarget),"one view kind per descriptor");
        check(Fits(80,{76,4}),"byte range exact end");
        check(!Fits(80,{76,8}),"byte range past end");
        check(!Fits(80,{std::numeric_limits<std::uint32_t>::max(),8}),"unsigned wrap handled");
        DynamicRing ring(24);ByteRange r{0,0};
        check(ring.DiscardReserve(12,r) && r.offset==0 && r.length==12,"discard first chunk");
        check(ring.NoOverwriteReserve(8,r) && r.offset==12,"reserve unwritten region");
        check(!ring.NoOverwriteReserve(8,r),"reject overwriting prior region");
        check(!ring.DiscardReserve(25,r),"discard cannot exceed capacity");
        check(ring.DiscardReserve(24,r) && ring.cursor==24,"discard reinitialize ring");
        check(!ring.NoOverwriteReserve(1,r),"ring full");
        std::cout << "PASS " << checks << " resource-contract checks (CPU model only)\n";
    } catch(const std::exception& e){std::cerr << "FAIL " << checks << " " << e.what() << '\n';return 1;}
}