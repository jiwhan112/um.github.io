<callout icon="📦" color="gray_bg">
	**03A · 전체 소스·CMake·검증 로그의 지속 보관**
	<mention-date start="2026-10-16"/>. 상위 교재 <mention-page url="https://app.notion.com/p/3fba99703f40817ba1b2cceecf06ce59">03 · DX11 Input Assembler·Shader·DrawIndexed·RTV/SRV 충돌</mention-page>. CPU 모델과 Windows 전용 D3D11 삼각형 예제를 구분합니다. Windows 분기는 이번 Linux 환경에서 컴파일하지 않았습니다.
</callout>
## 03_pipeline/main.cpp
```c++
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
struct Pipeline {
    bool inputLayout=false, vertexShader=false, pixelShader=false, rtv=false, viewport=false;
    bool constantBuffer=false;
    VB vb{0,0,0};
    std::vector<std::uint32_t> indices;
    IndexFormat indexFormat=IndexFormat::U16;
    Topology topology=Topology::Undefined;
    std::uint32_t startIndex=0;
    std::int32_t baseVertex=0;
    Pipeline() = default;
    bool DrawIndexed(std::uint32_t indexCount) const {
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
struct PassBindings {
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
struct alignas(16) FrameCB {Matrix4 wvp;float tint[4];};
static_assert(sizeof(FrameCB)%16==0, "CB size multiple 16");
int main(){
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

```
## 03_pipeline/d3d11_triangle_window.cpp
```c++
// Windows-only Direct3D 11 triangle teaching example. NOT built or run in Linux validation.
// Windows SDK, d3d11.lib, dxgi.lib, d3dcompiler.lib required; C++11 or later.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
using Microsoft::WRL::ComPtr;

struct DxError : std::runtime_error {
    HRESULT hr;
    DxError(HRESULT h, const char* where) : std::runtime_error(where), hr(h) {}
};
void Check(HRESULT hr, const char* where) { if (FAILED(hr)) throw DxError(hr,where); }
struct Vertex {float x,y,z;float r,g,b,a;};
static_assert(sizeof(Vertex)==28,"InputLayout requires 28 byte Vertex");
const Vertex kVerts[3]={{0,.6f,.5f,1,0,0,1},{.6f,-.6f,.5f,0,1,0,1},{-.6f,-.6f,.5f,0,0,1,1}};
const std::uint16_t kIdx[3]={0,1,2};
const char kShader[] = R"(
struct VSIn {float3 pos:POSITION; float4 color:COLOR0;};
struct VSOut {float4 pos:SV_POSITION; float4 color:COLOR0;};
VSOut VSMain(VSIn input){VSOut o; o.pos=float4(input.pos,1);o.color=input.color;return o;}
float4 PSMain(VSOut input):SV_Target {return input.color;}
)";
ComPtr<ID3DBlob> Compile(const char* entry,const char* target){
    ComPtr<ID3DBlob> code, errors;
    const HRESULT hr=D3DCompile(kShader,std::strlen(kShader),"embedded_triangle.hlsl",nullptr,nullptr,
        entry,target,D3DCOMPILE_ENABLE_STRICTNESS,0,code.GetAddressOf(),errors.GetAddressOf());
    if(FAILED(hr)){
        if(errors)OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
        Check(hr,"D3DCompile");
    }
    return code;
}
class Demo {
    HWND hwnd_;
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGISwapChain> swap_;
    ComPtr<ID3D11RenderTargetView> rtv_;
    ComPtr<ID3D11Buffer> vb_,ib_;
    ComPtr<ID3D11InputLayout> layout_;
    ComPtr<ID3D11VertexShader> vs_;
    ComPtr<ID3D11PixelShader> ps_;
    UINT width_=0,height_=0;
    void NewTarget(UINT w,UINT h){
        ComPtr<ID3D11Texture2D> back;
        Check(swap_->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(back.GetAddressOf())),"SwapChain::GetBuffer");
        Check(device_->CreateRenderTargetView(back.Get(),nullptr,rtv_.GetAddressOf()),"CreateRenderTargetView");
        width_=w;height_=h;
    }
    void Resize(UINT w,UINT h){
        if(!w||!h|| (w==width_ && h==height_))return;
        // Ensure all references to back-buffer views are released before ResizeBuffers.
        context_->OMSetRenderTargets(0,nullptr,nullptr);
        context_->ClearState();
        rtv_.Reset();
        Check(swap_->ResizeBuffers(0,w,h,DXGI_FORMAT_UNKNOWN,0),"ResizeBuffers");
        NewTarget(w,h);
    }
public:
    Demo(HWND hwnd,UINT w,UINT h):hwnd_(hwnd){
        if(!w||!h) throw std::invalid_argument("zero client size");
        DXGI_SWAP_CHAIN_DESC sd={};
        sd.BufferCount=1;
        sd.BufferDesc.Width=w;sd.BufferDesc.Height=h;sd.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow=hwnd_;sd.SampleDesc.Count=1;
        sd.Windowed=TRUE;sd.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
        const D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1};
        D3D_FEATURE_LEVEL chosen={};
        Check(D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,
            levels,2,D3D11_SDK_VERSION,&sd,swap_.GetAddressOf(),device_.GetAddressOf(),
            &chosen,context_.GetAddressOf()),"D3D11CreateDeviceAndSwapChain");
        NewTarget(w,h);
        const auto vsBlob=Compile("VSMain","vs_4_0"),psBlob=Compile("PSMain","ps_4_0");
        Check(device_->CreateVertexShader(vsBlob->GetBufferPointer(),vsBlob->GetBufferSize(),nullptr,vs_.GetAddressOf()),"CreateVertexShader");
        Check(device_->CreatePixelShader(psBlob->GetBufferPointer(),psBlob->GetBufferSize(),nullptr,ps_.GetAddressOf()),"CreatePixelShader");
        const D3D11_INPUT_ELEMENT_DESC input[]={
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
        };
        Check(device_->CreateInputLayout(input,2,vsBlob->GetBufferPointer(),vsBlob->GetBufferSize(),layout_.GetAddressOf()),"CreateInputLayout");
        D3D11_BUFFER_DESC bd={};bd.ByteWidth=sizeof(kVerts);bd.Usage=D3D11_USAGE_IMMUTABLE;bd.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA data={};data.pSysMem=kVerts;
        Check(device_->CreateBuffer(&bd,&data,vb_.GetAddressOf()),"CreateVertexBuffer");
        bd.ByteWidth=sizeof(kIdx);bd.BindFlags=D3D11_BIND_INDEX_BUFFER;data.pSysMem=kIdx;
        Check(device_->CreateBuffer(&bd,&data,ib_.GetAddressOf()),"CreateIndexBuffer");
    }
    void Frame(UINT width,UINT height){
        if(!width||!height)return;
        Resize(width,height);
        ID3D11RenderTargetView* output=rtv_.Get();
        context_->OMSetRenderTargets(1,&output,nullptr);
        D3D11_VIEWPORT vp={};vp.TopLeftX=0;vp.TopLeftY=0;vp.Width=static_cast<float>(width_);vp.Height=static_cast<float>(height_);vp.MinDepth=0;vp.MaxDepth=1;
        context_->RSSetViewports(1,&vp);
        const float clear[4]={.08f,.10f,.16f,1};
        context_->ClearRenderTargetView(rtv_.Get(),clear);
        const UINT stride=sizeof(Vertex),offset=0;
        ID3D11Buffer* vb=vb_.Get();
        context_->IASetVertexBuffers(0,1,&vb,&stride,&offset);
        context_->IASetIndexBuffer(ib_.Get(),DXGI_FORMAT_R16_UINT,0);
        context_->IASetInputLayout(layout_.Get());
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(vs_.Get(),nullptr,0);
        context_->PSSetShader(ps_.Get(),nullptr,0);
        context_->DrawIndexed(3,0,0);
        Check(swap_->Present(1,0),"Present");
    }
    ~Demo(){if(context_)context_->ClearState();}
};
LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_CLOSE){DestroyWindow(h);return 0;}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(h,m,w,l);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR,int show){
    HWND window=nullptr;
    const wchar_t* cls=L"DX11Pipeline03Example";
    WNDCLASSW wc={};wc.lpfnWndProc=WndProc;wc.hInstance=instance;wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.lpszClassName=cls;
    if(!RegisterClassW(&wc))return 2;
    int exitCode=0;
    try{
        window=CreateWindowExW(0,cls,L"DX11 Pipeline / Indexed Triangle",WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,CW_USEDEFAULT,800,600,nullptr,nullptr,instance,nullptr);
        if(!window)throw std::runtime_error("CreateWindowExW failed");
        ShowWindow(window,show);
        RECT r={};GetClientRect(window,&r);
        {
            Demo demo(window,static_cast<UINT>(r.right),static_cast<UINT>(r.bottom));
            MSG msg={};bool running=true;
            while(running){
                while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){
                    if(msg.message==WM_QUIT){running=false;break;}
                    TranslateMessage(&msg);DispatchMessageW(&msg);
                }
                if(!running)break;
                if(IsIconic(window)){Sleep(30);continue;}
                GetClientRect(window,&r);
                if(r.right<=0||r.bottom<=0){Sleep(30);continue;}
                demo.Frame(static_cast<UINT>(r.right),static_cast<UINT>(r.bottom));
            }
        }
    }catch(const DxError& e){
        std::ostringstream m;m<<e.what()<<" HRESULT 0x"<<std::hex<<static_cast<unsigned long>(e.hr);
        MessageBoxA(window,m.str().c_str(),"DX11 API failure",MB_OK|MB_ICONERROR);exitCode=1;
    }catch(const std::exception& e){MessageBoxA(window,e.what(),"DX11 initialization failure",MB_OK|MB_ICONERROR);exitCode=2;}
    if(window&&IsWindow(window))DestroyWindow(window);
    UnregisterClassW(cls,instance);
    return exitCode;
}

```
## 03_pipeline/CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_pipeline_contract LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 study default; 11 compatibility")
add_executable(dx11_pipeline_contract main.cpp)
set_target_properties(dx11_pipeline_contract PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
  target_compile_options(dx11_pipeline_contract PRIVATE /W4 /EHsc)
else()
  target_compile_options(dx11_pipeline_contract PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME pipeline_contract COMMAND dx11_pipeline_contract)
if(WIN32)
  add_executable(dx11_pipeline_window WIN32 d3d11_triangle_window.cpp)
  set_target_properties(dx11_pipeline_window PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
  target_link_libraries(dx11_pipeline_window PRIVATE d3d11 dxgi d3dcompiler user32)
  if(MSVC)
    target_compile_options(dx11_pipeline_window PRIVATE /W4 /EHsc)
  endif()
endif()

```
## 03_pipeline/[README.md](http://README.md)
```markdown
# DX11 03 pipeline CPU model
No actual GPU or D3D11 API usage. Demonstrates an explicit *lesson policy* for required state, DrawIndexed bounds, IA stride, index format, row-vector matrix math, and CPU-side SRV/RTV hazard avoidance. D3D11 itself may accept null PS/RTV/zero-index draws depending on circumstances: do not mistake this validator for the runtime spec. State binding and shaders must be tested separately on Windows.

`cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure -V`
Use `-DLAB_CXX_STANDARD=11` for compatibility build.

`d3d11_triangle_window.cpp` is an additional **Windows-only** complete indexed-triangle sample, compiling inline VS/PS HLSL through `D3DCompile`, binding input layout, immutable VB/IB, RTV, viewport, shaders, and DrawIndexed. It uses legacy discard swap effect to match the didactic sample, not modern flip-model guidance. **Not compiled, linked, or executed on this Linux host**. On Windows with Visual Studio/Windows SDK: `cmake -S . -B build -A x64 && cmake --build build --config Debug` then launch `dx11_pipeline_window.exe`. Record Windows SDK, D3DCompiler availability, HRESULTs, resize behavior, final image and Debug Layer messages.

```
## 전체 배치 CTest 원시 로그 (03은 테스트 2)
## gcc20 complete CTest output
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

Total Test time (real) =   0.02 sec

```
## gcc11 complete CTest output
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
1/2 Test #1: resource_contract ................   Passed    0.01 sec
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
## clangsan complete CTest output
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
1/2 Test #1: resource_contract ................   Passed    0.03 sec
test 2
    Start 2: pipeline_contract

2: Test command: /mnt/data/dx11_batch02_03/build-clangsan/03_pipeline/dx11_pipeline_contract
2: Working Directory: /mnt/data/dx11_batch02_03/build-clangsan/03_pipeline
2: Test timeout computed to be: 10000000
2: PASS 32 IA/pipeline/hazard checks (CPU model only)
2/2 Test #2: pipeline_contract ................   Passed    0.03 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.07 sec

```
검증 경계: GCC C++20·C++11, Clang ASan/UBSan의 동일한 32조건 및 구성별 CTest 1/1. 02를 함께 구성한 배치 전체는 각 2/2. Windows/MSVC/D3D11/GPU/HLSL 컴파일 미검증. 두 소스를 동일한 성공 결과로 해석하지 않습니다.
