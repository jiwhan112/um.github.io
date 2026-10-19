<callout icon="📦" color="gray_bg">
	**02A · 전체 소스 지속 보관 및 실행 로그**
	<mention-date start="2026-10-16"/>. 상위 교재: <mention-page url="https://app.notion.com/p/3fba99703f4081c6a57dc836e3b50283">02 · DX11 Resource/View·Usage·Map·Constant Buffer 계약</mention-page>. 독립 C++20/11 CPU 모델로 실제 D3D11/GPU가 아닙니다.
</callout>
## 02_resources/main.cpp
```c++
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
```
## 02_resources/CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_resource_contract LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "C++20 study or C++11 compatibility")
add_executable(dx11_resource_contract main.cpp)
set_target_properties(dx11_resource_contract PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
  target_compile_options(dx11_resource_contract PRIVATE /W4 /EHsc)
else()
  target_compile_options(dx11_resource_contract PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME resource_contract COMMAND dx11_resource_contract)
```
## 02_resources/[README.md](http://README.md)
# DX11 02 Resource/View contract model
Portable CPU educational validator for a narrow, explicitly documented subset of D3D11_BUFFER_DESC usage and mapping. It **does not call D3D11**, does **not** inspect a Windows runtime or GPU driver, and excludes MiscFlags, structured buffers, Texture2D layout and advanced 11.1 constant-buffer rules.
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLAB_CXX_STANDARD=20
cmake --build build
ctest --test-dir build --output-on-failure -V
# Compatibility: -DLAB_CXX_STANDARD=11
```
Map WRITE_NO_OVERWRITE model assumes D3D11.1 support if runtime11_1 flag true; an actual app additionally queries runtime and feature options. DynamicRing is a CPU bookkeeping model, not GPU synchronization. Source and tests are in main.cpp.
## 실제 CTest 실행 로그 (배치, 각 구성의 테스트 1 포함)
### gcc20
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch02_03/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_batch02_03/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch02_03/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_batch02_03/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: resource_contract

1: Test command: /mnt/data/dx11_batch02_03/build-gcc20/02_resources/dx11_resource_contract
1: Working Directory: /mnt/data/dx11_batch02_03/build-gcc20/02_resources
1: Test timeout computed to be: 10000000
1: PASS 40 resource-contract checks (CPU model only)
1/2 Test #1: resource_contract ................   Passed    0.01 sec
test 2
    Start 2: pipeline_contract

2: Test command: /mnt/data/dx11_batch02_03/build-gcc20/03_pipeline/dx11_pipeline_contract
2: Working Directory: /mnt/data/dx11_batch02_03/build-gcc20/03_pipeline
2: Test timeout computed to be: 10000000
2: PASS 32 IA/pipeline/hazard checks (CPU model only)
2/2 Test #2: pipeline_contract ................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.03 sec
```
### gcc11
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch02_03/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_batch02_03/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch02_03/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_batch02_03/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: resource_contract

1: Test command: /mnt/data/dx11_batch02_03/build-gcc11/02_resources/dx11_resource_contract
1: Working Directory: /mnt/data/dx11_batch02_03/build-gcc11/02_resources
1: Test timeout computed to be: 10000000
1: PASS 40 resource-contract checks (CPU model only)
1/2 Test #1: resource_contract ................   Passed    0.00 sec
test 2
    Start 2: pipeline_contract

2: Test command: /mnt/data/dx11_batch02_03/build-gcc11/03_pipeline/dx11_pipeline_contract
2: Working Directory: /mnt/data/dx11_batch02_03/build-gcc11/03_pipeline
2: Test timeout computed to be: 10000000
2: PASS 32 IA/pipeline/hazard checks (CPU model only)
2/2 Test #2: pipeline_contract ................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.02 sec
```
### clangsan
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch02_03/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_batch02_03/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch02_03/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_batch02_03/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: resource_contract

1: Test command: /mnt/data/dx11_batch02_03/build-clangsan/02_resources/dx11_resource_contract
1: Working Directory: /mnt/data/dx11_batch02_03/build-clangsan/02_resources
1: Test timeout computed to be: 10000000
1: PASS 40 resource-contract checks (CPU model only)
1/2 Test #1: resource_contract ................   Passed    0.02 sec
test 2
    Start 2: pipeline_contract

2: Test command: /mnt/data/dx11_batch02_03/build-clangsan/03_pipeline/dx11_pipeline_contract
2: Working Directory: /mnt/data/dx11_batch02_03/build-clangsan/03_pipeline
2: Test timeout computed to be: 10000000
2: PASS 32 IA/pipeline/hazard checks (CPU model only)
2/2 Test #2: pipeline_contract ................   Passed    0.03 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.06 sec
```
검증 범위: GCC 14.2 C++20·C++11과 Clang 17 C++20 ASan/UBSan에서 동일한 40조건, CTest 각 1/1. 배치 전체는 각 2/2입니다. Windows/MSVC·D3D11/GPU 미실행.
