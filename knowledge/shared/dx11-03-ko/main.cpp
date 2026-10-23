#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

struct Vertex {float x,y,z,u,v;};
static_assert(sizeof(Vertex)==20, "Vertex stride must be 20 for this example");
struct VB {std::size_t count, stride, byteOffset;};
enum class IndexFormat {U16,U32};
enum class Topology {Undefined,TriangleList,LineList};
struct Pipeline { // [한국어 해설] 이 단원에 필요한 상태의 논리 모델이다. null 셰이더 등 API가 허용할 수 있는 경우도 수업 정책으로 거부한다.
    bool inputLayout=false, vertexShader=false, pixelShader=false, rtv=false, viewport=false;
    bool constantBuffer=false;
    VB vb{0,0,0};
    std::vector<std::uint32_t> indices;
    IndexFormat indexFormat=IndexFormat::U16;
    Topology topology=Topology::Undefined;
    std::uint32_t startIndex=0;
    std::int32_t baseVertex=0;
    Pipeline() = default;
    bool DrawIndexed(std::uint32_t indexCount) const { // [한국어 해설] stride·index 형식·start/base를 구분하고 실제 정점 범위를 검사한다.
        if (!inputLayout || !vertexShader || !pixelShader || !rtv || !viewport || !constantBuffer)
            return false; // chosen lesson contract; D3D11 API may permit some null shader states
        if (topology!=Topology::TriangleList || indexCount==0 || indexCount%3!=0) return false;
        if (vb.stride!=sizeof(Vertex) || vb.byteOffset%sizeof(Vertex) || vb.count==0) return false;
        if (startIndex>indices.size() || indexCount>indices.size()-startIndex) return false;
        if(indexFormat==IndexFormat::U16){
            for(std::size_t i=startIndex;i<startIndex+indexCount;++i)
                if(indices[i]>65535u) return false;
        }
        for(std::size_t i=startIndex;i<startIndex+indexCount;++i){
            const std::int64_t v=static_cast<std::int64_t>(indices[i])+baseVertex;
            if(v<0 || static_cast<std::uint64_t>(v)>=vb.count) return false;
        }
        return true;
    }
};
// CPU-only subresource ownership model: cannot read while writing identical region.
struct PassBindings { // [한국어 해설] 읽기와 쓰기의 같은 자원 충돌을 표현하는 단순 모델이다. 모든 overlapping view 경우를 포괄하지 않는다.
    int renderTarget=-1;
    std::vector<int> shaderResources;
    bool BindRT(int resource) {
        if(resource<0) return false;
        if(std::find(shaderResources.begin(),shaderResources.end(),resource)!=shaderResources.end()) return false;
        renderTarget=resource;return true;
    }
    bool BindSRV(int resource) {
        if(resource<0 || resource==renderTarget) return false;
        shaderResources.push_back(resource);return true;
    }
    void ClearRead() {shaderResources.clear();}
    void UnbindRT() {renderTarget=-1;}
};
struct Matrix4 {float m[4][4];};
Matrix4 Identity(){Matrix4 r={};for(int i=0;i<4;++i) r.m[i][i]=1;return r;}
struct Vec4 {float x,y,z,w;};
Vec4 MulRow(Vec4 p,const Matrix4& m){
    const float x[4]={p.x,p.y,p.z,p.w};float y[4]={};
    for(int j=0;j<4;++j)for(int i=0;i<4;++i)y[j]+=x[i]*m.m[i][j];
    return {y[0],y[1],y[2],y[3]};
}
bool Near(float a,float b){return std::fabs(a-b)<1e-5f;}
// A stand-in for a CPU->CB payload; actual HLSL packing/row-major policy is separate.
struct alignas(16) FrameCB {Matrix4 wvp;float tint[4];}; // [한국어 해설] CPU payload 크기/정렬만 확인한다. 실제 HLSL packing·row_major 계약은 별도로 맞춰야 한다.
static_assert(sizeof(FrameCB)%16==0, "CB size multiple 16");
int main(){ // [한국어 해설] 빠진 상태, 주소 경계, SRV/RT 충돌과 상수 버퍼 배치를 검사한다.
    unsigned checks=0;
    const auto check=[&](bool b,const char* label){++checks;if(!b)throw std::runtime_error(label);};
    try{
        Pipeline p;
        check(!p.DrawIndexed(3),"missing state");
        p.inputLayout=p.vertexShader=p.pixelShader=p.rtv=p.viewport=p.constantBuffer=true;
        p.vb={4,sizeof(Vertex),0};p.indices={0,1,2,2,1,3};p.topology=Topology::TriangleList;
        check(p.DrawIndexed(6),"quad two triangles");
        check(p.DrawIndexed(3),"one triangle");
        check(!p.DrawIndexed(4),"incomplete triangle rejected by lesson policy");
        check(!p.DrawIndexed(0),"zero draw rejected by lesson policy");
        check(!p.DrawIndexed(9),"too many IB elements");
        p.startIndex=3;check(p.DrawIndexed(3),"offset DrawIndexed");
        p.startIndex=4;check(!p.DrawIndexed(3),"offset overflow");
        p.startIndex=0;p.indices[0]=4;check(!p.DrawIndexed(3),"VB out of range");
        p.indices[0]=0;p.baseVertex=-1;check(!p.DrawIndexed(3),"negative actual VB index");
        p.baseVertex=1;check(!p.DrawIndexed(6),"positive base overflow");
        p.baseVertex=0;p.vb.stride=16;check(!p.DrawIndexed(6),"stride mismatch");
        p.vb.stride=sizeof(Vertex);p.vb.byteOffset=4;check(!p.DrawIndexed(6),"unaligned offset lesson check");
        p.vb.byteOffset=0;p.topology=Topology::LineList;check(!p.DrawIndexed(6),"topology mismatch lesson check");
        p.topology=Topology::TriangleList;p.constantBuffer=false;check(!p.DrawIndexed(6),"lesson shader constants required");
        p.constantBuffer=true;p.vertexShader=false;check(!p.DrawIndexed(6),"VS missing");
        p.vertexShader=true;p.inputLayout=false;check(!p.DrawIndexed(6),"layout missing");
        p.inputLayout=true;p.rtv=false;check(!p.DrawIndexed(6),"RTV missing");
        p.rtv=true;check(p.DrawIndexed(6),"restored full state");
        p.indexFormat=IndexFormat::U16;p.indices[0]=65536;check(!p.DrawIndexed(3),"U16 overflow");
        p.indexFormat=IndexFormat::U32;check(!p.DrawIndexed(3),"U32 but VB too small");
        p.indexFormat=IndexFormat::U16;p.indices[0]=0;
        PassBindings b;
        check(b.BindSRV(5),"SRV bind");
        check(!b.BindRT(5),"same resource write blocked");
        check(b.BindRT(8),"different RT allowed");
        check(!b.BindSRV(8),"RT read hazard blocked");
        b.ClearRead();b.UnbindRT();check(b.BindRT(5),"after SRV unbind RT accepted");
        b.UnbindRT();check(b.BindSRV(5),"after RT unbind SRV accepted");
        check(!b.BindSRV(-1),"invalid resource rejected");
        Matrix4 m=Identity();m.m[3][0]=3;m.m[3][1]=-2;
        const Vec4 pt=MulRow({1,2,3,1},m),dir=MulRow({1,0,0,0},m);
        check(Near(pt.x,4)&&Near(pt.y,0)&&Near(pt.z,3),"point translation");
        check(Near(dir.x,1)&&Near(dir.y,0)&&Near(dir.w,0),"direction unaffected");
        check(sizeof(FrameCB)==80,"CB payload 80 bytes");
        check(alignof(FrameCB)>=16,"aligned structure");
        std::cout<<"PASS "<<checks<<" IA/pipeline/hazard checks (CPU model only)\n";
    }catch(const std::exception& e){std::cerr<<"FAIL "<<checks<<" "<<e.what()<<'\n';return 1;}
}
