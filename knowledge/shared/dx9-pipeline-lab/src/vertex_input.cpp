#include "pipeline.hpp"
#include "check.hpp"
#include <type_traits>
struct VertexPT { float x,y,z,u,v; };
static_assert(std::is_standard_layout_v<VertexPT> && std::is_trivially_copyable_v<VertexPT>);
static_assert(sizeof(float)==4 && sizeof(VertexPT)==20 && offsetof(VertexPT,u)==12);
int main() { return Run("vertex_input",[](Checks& c) {
    using namespace pipeline;
    const std::array<VertexPT,4> vertices{{{-1,1,2,0,0},{1,1,2,1,0},{1,-1,2,1,1},{-1,-1,2,0,1}}};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto bytes=std::as_bytes(std::span(vertices));
    for(auto i:indices) {
        const auto off=FetchOffset(bytes.size(),0,sizeof(VertexPT),0,i,12,8);
        std::array<float,2> uv{}; std::memcpy(uv.data(),bytes.data()+off,sizeof(uv));
        c.Near(uv[0],vertices[i].u,"u matches declaration");
        c.Near(uv[1],vertices[i].v,"v matches declaration");
    }
    c.That(indices.size()/3==2,"two triangle-list primitives");
    c.That(FetchOffset(120,0,20,2,3,12,8)==112,"base index and byte offset differ");
    c.That(FetchOffset(100,20,20,0,3,12,8)==92,"stream offset in bytes");
    c.Rejects([]{FetchOffset(80,0,20,0,4,0,12);},"index beyond buffer");
    c.Rejects([]{FetchOffset(80,0,20,-1,0,0,12);},"negative effective index");
    c.Rejects([]{FetchOffset(80,0,16,0,0,12,8);},"stride too short");
    c.Rejects([]{FetchOffset(80,81,20,0,0,0,4);},"stream offset beyond buffer");
    c.Rejects([]{FetchOffset(80,0,0,0,0,0,4);},"zero stride unsupported by this model");
    c.Rejects([]{FetchOffset(80,0,20,0,0,19,4);},"element crosses stride");
    c.Rejects([]{FetchOffset(80,0,20,0,0xffffffffu,0,4);},"huge index cannot wrap");
    std::cout<<"quad: 4 vertices / 6 indices / 2 triangles / stride=20\n";
}); }

