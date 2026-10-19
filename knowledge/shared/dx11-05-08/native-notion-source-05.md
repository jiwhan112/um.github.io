<callout icon="📦" color="gray_bg">
	**수동 DX11 개정 소스 지속 보관**
	<mention-date start="2026-10-16"/>. 전체 소스·CMake·원시 GCC/Clang CTest 결과. Windows D3D11·GPU·FXC/DXC 실행 검증은 수행하지 않았습니다. 아래는 재현 가능한 CPU 모델 기준이며 오류 수정 전 시행착오와 수정 내용을 구분합니다.
</callout>
Standalone source source-of-truth. CPU-only tests, except uncompiled Windows and HLSL listed explicitly.
Save each code block to the filename in the heading. The root CMakeLists.txt registers four units; individual CMakeLists.txt are subdir fragments.
## Changes and test limitations
# DX11 05–08 CPU source validation
- Date: 2026-10-16 (KST); environment Linux x86-64, GCC 14.2.0, Clang 17.0.0, CMake 3.31.6.
- Default standalone learning: C++20; GCC C++11 compatibility explicitly compiled.
- These tests validate only the CPU contracts. Windows/MSVC, actual D3D11 GPU, HLSL FXC/DXC were NOT COMPILED OR RUN.
- 05 Windows source and HLSL are provided uncompiled with API static review; no claim of GPU success.
- Tests per program: 05=29, 06=30, 07=28, 08=33. The 120 checks are distinct within these four programs. Each configuration reruns the same 120, not 360 independent cases.
- Pre-fix GCC build failed on 06 due to `-Werror=unused-variable`; removed the unused local variable.
- Pre-fix CPU test 08 aborted because the constructor moved `h` into member `height` and then checked the moved-from parameter `h.size()`; fixed to check `height.size()` after move.
- All three complete configurations were rebuilt and CTest 4/4 passed after fixes.
```bash
cmake -S . -B build-gcc20 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DLAB_CXX_STANDARD=20
cmake --build build-gcc20 -j 4
ctest --test-dir build-gcc20 --output-on-failure -V
cmake -S . -B build-gcc11 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DLAB_CXX_STANDARD=11
cmake --build build-gcc11 -j 4
ctest --test-dir build-gcc11 --output-on-failure -V
cmake -S . -B build-clangsan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DLAB_CXX_STANDARD=20 '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer' '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'
cmake --build build-clangsan -j 4
ctest --test-dir build-clangsan --output-on-failure -V
```
## Results
<table header-row="true">
<tr>
<td>Configuration</td>
<td>Result</td>
<td>Internal conditions</td>
</tr>
<tr>
<td>GCC 14.2 C++20 Debug</td>
<td>CTest 4/4 passed</td>
<td>29+30+28+33</td>
</tr>
<tr>
<td>GCC 14.2 C++11 Debug</td>
<td>CTest 4/4 passed</td>
<td>Same 120 conditions</td>
</tr>
<tr>
<td>Clang 17 C++20 ASan/UBSan Debug</td>
<td>CTest 4/4 passed; no sanitizer diagnostics</td>
<td>Same 120 conditions</td>
</tr>
</table>
## Windows manual checklist (not executed)
1. Windows 10/11 with Visual Studio 2022, Windows SDK, d3d11.lib, dxgi.lib, d3dcompiler.lib and CMake.
2. Configure with `cmake -S . -B build -A x64`, build Debug, run CTest, then run `build\05_textured_rect\Debug\dx11_05_windows.exe` (actual location depends on generator).
3. Verify checker quad, aspect behavior, ResizeBuffers on resize, minimized window handling, HRESULT/debug layer messages, no COM leaks.
4. Compile quad.hlsl and skybox_lambert.hlsl using actual FXC/DXC as supported; HLSL compilation has not been performed here.
## CMakeLists.txt
```plain text
add_executable(dx11_05_cpu main.cpp)
set_target_properties(dx11_05_cpu PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
 target_compile_options(dx11_05_cpu PRIVATE /W4 /EHsc)
else()
 target_compile_options(dx11_05_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME dx11_05_cpu COMMAND dx11_05_cpu)
if(WIN32)
 add_executable(dx11_05_windows WIN32 textured_quad_win.cpp)
 target_compile_definitions(dx11_05_windows PRIVATE UNICODE _UNICODE)
 target_compile_options(dx11_05_windows PRIVATE /W4 /EHsc)
 target_link_libraries(dx11_05_windows PRIVATE d3d11 dxgi d3dcompiler user32)
endif()

```
## main.cpp
```c++
// Portable DX11 textured quad *CPU contract model*. No graphics API calls.
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
struct Vtx {float x,y,z,u,v;};
static_assert(sizeof(Vtx)==20,"vertex layout expected");
struct MaterialSlots {bool vs,ps,layout,vb,ib,srv,sampler,rtv;};
struct RowImage {std::size_t width,height,pitch;std::vector<std::uint8_t> bytes;};
static bool ValidateRow(const RowImage& t) {
 if(!t.width || !t.height || t.width>std::numeric_limits<std::size_t>::max()/4) return false;
 const std::size_t row=t.width*4;
 if(t.pitch<row || t.height>std::numeric_limits<std::size_t>::max()/t.pitch) return false;
 return t.bytes.size()>=t.pitch*t.height;
}
static bool Sample(const RowImage& t,std::size_t x,std::size_t y,std::array<unsigned,4>& out){
 if(!ValidateRow(t)||x>=t.width||y>=t.height)return false;
 const std::size_t off=y*t.pitch+x*4;
 for(unsigned k=0;k<4;++k)out[k]=t.bytes[off+k];
 return true;
}
static bool ValidateDraw(const std::vector<Vtx>& vb,const std::vector<std::uint16_t>& ib,
 std::size_t start,std::size_t count,std::int64_t base){
 if(vb.empty()||count==0||count%3||start>ib.size()||count>ib.size()-start)return false;
 for(std::size_t i=start;i<start+count;++i){
  const std::int64_t v=base+static_cast<std::int64_t>(ib[i]);
  if(v<0||static_cast<std::uint64_t>(v)>=vb.size())return false;
 }
 return true;
}
static bool Ready(const MaterialSlots& m){return m.vs&&m.ps&&m.layout&&m.vb&&m.ib&&m.srv&&m.sampler&&m.rtv;}
int main(){unsigned n=0; auto ck=[&](bool b,const char* s){++n;if(!b)throw std::runtime_error(s);};
 const std::vector<Vtx> v={{-1,-1,.5f,0,1},{-1,1,.5f,0,0},{1,-1,.5f,1,1},{1,1,.5f,1,0}};
 const std::vector<std::uint16_t> idx={0,1,2,2,1,3};
 ck(ValidateDraw(v,idx,0,6,0),"quad"); ck(!ValidateDraw(v,idx,0,7,0),"indices overflow");
 ck(!ValidateDraw(v,idx,0,5,0),"not triangles");ck(!ValidateDraw(v,idx,4,3,0),"start out");
 ck(!ValidateDraw(v,idx,0,6,-1),"negative vertex");ck(!ValidateDraw(v,idx,0,6,1),"vertex overflow");
 ck(ValidateDraw(v,{1,2,3},0,3,-1),"relative base");ck(!ValidateDraw(v,idx,0,0,0),"empty draw");
 RowImage img{2,2,12,std::vector<std::uint8_t>(24,0xCC)};
 img.bytes[0]=10;img.bytes[1]=20;img.bytes[2]=30;img.bytes[3]=255;
 img.bytes[12]=40;img.bytes[13]=50;img.bytes[14]=60;img.bytes[15]=255;
 ck(ValidateRow(img),"pitched image");std::array<unsigned,4> px={};
 ck(Sample(img,0,1,px),"sample row");ck(px[0]==40&&px[1]==50&&px[2]==60&&px[3]==255,"row pitch offset");
 ck(Sample(img,0,0,px),"sample origin");ck(px[0]==10,"first row");
 ck(!Sample(img,2,0,px),"out x");ck(!Sample(img,0,2,px),"out y");
 RowImage invalid=img;invalid.pitch=7;ck(!ValidateRow(invalid),"short pitch");
 invalid=img;invalid.bytes.resize(23);ck(!ValidateRow(invalid),"short bytes");
 invalid=img;invalid.width=std::numeric_limits<std::size_t>::max();ck(!ValidateRow(invalid),"width overflow");
 invalid=img;invalid.height=std::numeric_limits<std::size_t>::max();ck(!ValidateRow(invalid),"height overflow");
 invalid=img;invalid.width=0;ck(!ValidateRow(invalid),"zero width");
 invalid=img;invalid.height=0;ck(!ValidateRow(invalid),"zero height");
 MaterialSlots m{true,true,true,true,true,true,true,true};ck(Ready(m),"all bound");
 m.srv=false;ck(!Ready(m),"missing texture SRV");m.srv=true;m.sampler=false;ck(!Ready(m),"missing sampler");
 m.sampler=true;m.vs=false;ck(!Ready(m),"missing VS");m.vs=true;m.rtv=false;ck(!Ready(m),"missing render target");
 // Shader t0/s0 bindings are independent of IA vertex stream and render target.
 m.rtv=true;m.layout=false;ck(!Ready(m),"missing input layout");
 ck(v.size()*sizeof(Vtx)==80,"VB size");ck(idx.size()*sizeof(std::uint16_t)==12,"IB size");
 std::cout<<"PASS "<<n<<" textured rect CPU contract checks\n";
}

```
## quad.hlsl
```plain text
struct VSInput { float3 position : POSITION; float2 uv : TEXCOORD0; };
struct PSInput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
Texture2D colorTexture : register(t0);
SamplerState colorSampler : register(s0);
PSInput VSMain(VSInput input) { PSInput o; o.position=float4(input.position,1); o.uv=input.uv; return o; }
float4 PSMain(PSInput input) : SV_Target { return colorTexture.Sample(colorSampler,input.uv); }

```
## textured_quad_win.cpp
```c++
// Windows-only D3D11 textured quad integration. NOT COMPILED/EXECUTED in Linux verification.
// Required Windows SDK + d3d11.lib, dxgi.lib, d3dcompiler.lib, user32.lib.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
using Microsoft::WRL::ComPtr;
struct DxError:std::runtime_error { HRESULT hr; DxError(HRESULT h,const char* m):std::runtime_error(m),hr(h){} };
static void Req(HRESULT hr,const char* m){if(FAILED(hr))throw DxError(hr,m);}
struct Vertex{float x,y,z,u,v;};
static_assert(sizeof(Vertex)==20,"vertex stride");
static const Vertex kVertex[4]={{-.7f,-.7f,.5f,0,1},{-.7f,.7f,.5f,0,0},{.7f,-.7f,.5f,1,1},{.7f,.7f,.5f,1,0}};
static const std::uint16_t kIndices[6]={0,1,2,2,1,3};
static const char kShader[]=R"(
struct VSIn {float3 position:POSITION;float2 uv:TEXCOORD0;};
struct PSIn {float4 position:SV_POSITION;float2 uv:TEXCOORD0;};
Texture2D tex:register(t0);SamplerState samp:register(s0);
PSIn VSMain(VSIn input){PSIn o;o.position=float4(input.position,1);o.uv=input.uv;return o;}
float4 PSMain(PSIn input):SV_Target {return tex.Sample(samp,input.uv);}
)";
static ComPtr<ID3DBlob> Compile(const char* name,const char* profile){
 ComPtr<ID3DBlob> code,error;
 HRESULT hr=D3DCompile(kShader,std::strlen(kShader),"quad.hlsl",nullptr,nullptr,name,profile,D3DCOMPILE_ENABLE_STRICTNESS,0,code.GetAddressOf(),error.GetAddressOf());
 if(FAILED(hr) && error)OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
 Req(hr,"D3DCompile");return code;
}
class App {
 HWND window_;
 ComPtr<ID3D11Device> device_;
 ComPtr<ID3D11DeviceContext> context_;
 ComPtr<IDXGISwapChain> swap_;
 ComPtr<ID3D11RenderTargetView> rtv_;
 ComPtr<ID3D11VertexShader> vs_;
 ComPtr<ID3D11PixelShader> ps_;
 ComPtr<ID3D11InputLayout> layout_;
 ComPtr<ID3D11Buffer> vb_,ib_;
 ComPtr<ID3D11Texture2D> texture_;
 ComPtr<ID3D11ShaderResourceView> srv_;
 ComPtr<ID3D11SamplerState> sampler_;
 UINT width_=0,height_=0;
 void CreateRTV(UINT w,UINT h){
  ComPtr<ID3D11Texture2D> back;
  Req(swap_->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(back.GetAddressOf())),"GetBuffer");
  Req(device_->CreateRenderTargetView(back.Get(),nullptr,rtv_.GetAddressOf()),"CreateRenderTargetView");
  width_=w;height_=h;
 }
 void Resize(UINT w,UINT h){
  if(!w||!h||(width_==w&&height_==h))return;
  context_->OMSetRenderTargets(0,nullptr,nullptr);
  context_->ClearState();
  rtv_.Reset();
  Req(swap_->ResizeBuffers(0,w,h,DXGI_FORMAT_UNKNOWN,0),"ResizeBuffers");
  CreateRTV(w,h);
 }
 void CreateGeometry(){
  D3D11_BUFFER_DESC b={};b.Usage=D3D11_USAGE_IMMUTABLE;
  b.ByteWidth=sizeof(kVertex);b.BindFlags=D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA initial={};initial.pSysMem=kVertex;
  Req(device_->CreateBuffer(&b,&initial,vb_.GetAddressOf()),"Create VB");
  b.ByteWidth=sizeof(kIndices);b.BindFlags=D3D11_BIND_INDEX_BUFFER;initial.pSysMem=kIndices;
  Req(device_->CreateBuffer(&b,&initial,ib_.GetAddressOf()),"Create IB");
 }
 void CreateTexture(){
  constexpr UINT size=64;
  std::array<std::uint32_t,size*size> image{};
  for(UINT y=0;y<size;++y)for(UINT x=0;x<size;++x)
   image[y*size+x]=((x/8+y/8)&1)?0xffd04030u:0xff30b0e0u;
  D3D11_TEXTURE2D_DESC td={};td.Width=size;td.Height=size;td.MipLevels=1;td.ArraySize=1;
  td.Format=DXGI_FORMAT_R8G8B8A8_UNORM;td.SampleDesc.Count=1;td.Usage=D3D11_USAGE_IMMUTABLE;td.BindFlags=D3D11_BIND_SHADER_RESOURCE;
  D3D11_SUBRESOURCE_DATA initial={};initial.pSysMem=image.data();initial.SysMemPitch=size*4;
  Req(device_->CreateTexture2D(&td,&initial,texture_.GetAddressOf()),"CreateTexture2D");
  Req(device_->CreateShaderResourceView(texture_.Get(),nullptr,srv_.GetAddressOf()),"Create SRV");
  D3D11_SAMPLER_DESC sd={};sd.Filter=D3D11_FILTER_MIN_MAG_MIP_POINT;
  sd.AddressU=sd.AddressV=sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
  sd.MaxLOD=D3D11_FLOAT32_MAX;sd.ComparisonFunc=D3D11_COMPARISON_NEVER;
  Req(device_->CreateSamplerState(&sd,sampler_.GetAddressOf()),"Create Sampler");
 }
public:
 App(HWND h,UINT w,UINT height):window_(h){
  if(!w||!height)throw std::invalid_argument("zero client size");
  DXGI_SWAP_CHAIN_DESC sd={};sd.BufferCount=1;sd.BufferDesc.Width=w;sd.BufferDesc.Height=height;
  sd.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;sd.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow=window_;sd.SampleDesc.Count=1;sd.Windowed=TRUE;sd.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
  const D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1};
  D3D_FEATURE_LEVEL chosen={};
  Req(D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,levels,2,D3D11_SDK_VERSION,&sd,
      swap_.GetAddressOf(),device_.GetAddressOf(),&chosen,context_.GetAddressOf()),"CreateDeviceAndSwapChain");
  CreateRTV(w,height);
  const auto v=Compile("VSMain","vs_4_0"),p=Compile("PSMain","ps_4_0");
  Req(device_->CreateVertexShader(v->GetBufferPointer(),v->GetBufferSize(),nullptr,vs_.GetAddressOf()),"Create VS");
  Req(device_->CreatePixelShader(p->GetBufferPointer(),p->GetBufferSize(),nullptr,ps_.GetAddressOf()),"Create PS");
  const D3D11_INPUT_ELEMENT_DESC il[]={
    {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
    {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}};
  Req(device_->CreateInputLayout(il,2,v->GetBufferPointer(),v->GetBufferSize(),layout_.GetAddressOf()),"Create IL");
  CreateGeometry();CreateTexture();
 }
 ~App(){if(context_)context_->ClearState();}
 void Frame(UINT w,UINT h){
  if(!w||!h)return;Resize(w,h);
  const float clear[4]={.05f,.07f,.1f,1};
  context_->ClearRenderTargetView(rtv_.Get(),clear);
  ID3D11RenderTargetView* target=rtv_.Get();context_->OMSetRenderTargets(1,&target,nullptr);
  D3D11_VIEWPORT vp={};vp.Width=static_cast<float>(width_);vp.Height=static_cast<float>(height_);vp.MaxDepth=1;context_->RSSetViewports(1,&vp);
  const UINT stride=sizeof(Vertex),offset=0;ID3D11Buffer* vertex=vb_.Get();
  context_->IASetVertexBuffers(0,1,&vertex,&stride,&offset);
  context_->IASetIndexBuffer(ib_.Get(),DXGI_FORMAT_R16_UINT,0);
  context_->IASetInputLayout(layout_.Get());context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context_->VSSetShader(vs_.Get(),nullptr,0);context_->PSSetShader(ps_.Get(),nullptr,0);
  ID3D11ShaderResourceView* view=srv_.Get();ID3D11SamplerState* samp=sampler_.Get();
  context_->PSSetShaderResources(0,1,&view);context_->PSSetSamplers(0,1,&samp);
  context_->DrawIndexed(6,0,0);
  Req(swap_->Present(1,0),"Present");
 }
};
static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){
 if(m==WM_CLOSE){DestroyWindow(h);return 0;}if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
 return DefWindowProcW(h,m,w,l);
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){
 const wchar_t* cls=L"DX11TexturedRect05";
 WNDCLASSW wc={};wc.lpfnWndProc=WndProc;wc.hInstance=inst;wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.lpszClassName=cls;
 if(!RegisterClassW(&wc))return 2;
 HWND h=nullptr;int ret=0;
 try{
  h=CreateWindowExW(0,cls,L"DX11 Textured Rect / resize",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,800,600,nullptr,nullptr,inst,nullptr);
  if(!h)throw std::runtime_error("CreateWindowExW failed");ShowWindow(h,show);
  RECT rect={};GetClientRect(h,&rect);
  {
   App app(h,static_cast<UINT>(rect.right),static_cast<UINT>(rect.bottom));
   bool running=true;MSG msg={};
   while(running){while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){
      if(msg.message==WM_QUIT){running=false;break;}TranslateMessage(&msg);DispatchMessageW(&msg);
    }
    if(!running)break;
    if(IsIconic(h)){Sleep(30);continue;}
    GetClientRect(h,&rect);if(rect.right<=0||rect.bottom<=0){Sleep(30);continue;}
    app.Frame(static_cast<UINT>(rect.right),static_cast<UINT>(rect.bottom));
   }
  }
 }catch(const DxError& e){std::ostringstream ss;ss<<e.what()<<" 0x"<<std::hex<<static_cast<unsigned long>(e.hr);
     MessageBoxA(h,ss.str().c_str(),"D3D11 Error",MB_OK|MB_ICONERROR);ret=1;
 }catch(const std::exception& e){MessageBoxA(h,e.what(),"Initialization Error",MB_OK|MB_ICONERROR);ret=2;}
 if(h&&IsWindow(h))DestroyWindow(h);UnregisterClassW(cls,inst);return ret;
}

```
## Root CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_study_05_08 LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 default, 11 compatibility")
enable_testing()
add_subdirectory(05_textured_rect)
add_subdirectory(06_transform_camera)
add_subdirectory(07_skybox_lighting)
add_subdirectory(08_terrain)

```
## Full validation logs (all configurations rerun same tests)
### gcc20
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch05_08/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_batch05_08/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_05_cpu

1: Test command: /mnt/data/dx11_batch05_08/build-gcc20/05_textured_rect/dx11_05_cpu
1: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/05_textured_rect
1: Test timeout computed to be: 10000000
1: PASS 29 textured rect CPU contract checks
1/4 Test #1: dx11_05_cpu ......................   Passed    0.01 sec
test 2
    Start 2: dx11_06_cpu

2: Test command: /mnt/data/dx11_batch05_08/build-gcc20/06_transform_camera/dx11_06_cpu
2: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/06_transform_camera
2: Test timeout computed to be: 10000000
2: PASS 30 transform/camera CPU contract checks
2/4 Test #2: dx11_06_cpu ......................   Passed    0.01 sec
test 3
    Start 3: dx11_07_cpu

3: Test command: /mnt/data/dx11_batch05_08/build-gcc20/07_skybox_lighting/dx11_07_cpu
3: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/07_skybox_lighting
3: Test timeout computed to be: 10000000
3: PASS 28 skybox/lighting CPU contract checks
3/4 Test #3: dx11_07_cpu ......................   Passed    0.02 sec
test 4
    Start 4: dx11_08_cpu

4: Test command: /mnt/data/dx11_batch05_08/build-gcc20/08_terrain/dx11_08_cpu
4: Working Directory: /mnt/data/dx11_batch05_08/build-gcc20/08_terrain
4: Test timeout computed to be: 10000000
4: PASS 33 terrain CPU contract checks
4/4 Test #4: dx11_08_cpu ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.05 sec

```
### gcc11
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch05_08/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_batch05_08/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_05_cpu

1: Test command: /mnt/data/dx11_batch05_08/build-gcc11/05_textured_rect/dx11_05_cpu
1: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/05_textured_rect
1: Test timeout computed to be: 10000000
1: PASS 29 textured rect CPU contract checks
1/4 Test #1: dx11_05_cpu ......................   Passed    0.01 sec
test 2
    Start 2: dx11_06_cpu

2: Test command: /mnt/data/dx11_batch05_08/build-gcc11/06_transform_camera/dx11_06_cpu
2: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/06_transform_camera
2: Test timeout computed to be: 10000000
2: PASS 30 transform/camera CPU contract checks
2/4 Test #2: dx11_06_cpu ......................   Passed    0.00 sec
test 3
    Start 3: dx11_07_cpu

3: Test command: /mnt/data/dx11_batch05_08/build-gcc11/07_skybox_lighting/dx11_07_cpu
3: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/07_skybox_lighting
3: Test timeout computed to be: 10000000
3: PASS 28 skybox/lighting CPU contract checks
3/4 Test #3: dx11_07_cpu ......................   Passed    0.02 sec
test 4
    Start 4: dx11_08_cpu

4: Test command: /mnt/data/dx11_batch05_08/build-gcc11/08_terrain/dx11_08_cpu
4: Working Directory: /mnt/data/dx11_batch05_08/build-gcc11/08_terrain
4: Test timeout computed to be: 10000000
4: PASS 33 terrain CPU contract checks
4/4 Test #4: dx11_08_cpu ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.05 sec

```
### clangsan
```plain text
Internal ctest changing into directory: /mnt/data/dx11_batch05_08/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_batch05_08/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_batch05_08/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_05_cpu

1: Test command: /mnt/data/dx11_batch05_08/build-clangsan/05_textured_rect/dx11_05_cpu
1: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/05_textured_rect
1: Test timeout computed to be: 10000000
1: PASS 29 textured rect CPU contract checks
1/4 Test #1: dx11_05_cpu ......................   Passed    0.02 sec
test 2
    Start 2: dx11_06_cpu

2: Test command: /mnt/data/dx11_batch05_08/build-clangsan/06_transform_camera/dx11_06_cpu
2: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/06_transform_camera
2: Test timeout computed to be: 10000000
2: PASS 30 transform/camera CPU contract checks
2/4 Test #2: dx11_06_cpu ......................   Passed    0.03 sec
test 3
    Start 3: dx11_07_cpu

3: Test command: /mnt/data/dx11_batch05_08/build-clangsan/07_skybox_lighting/dx11_07_cpu
3: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/07_skybox_lighting
3: Test timeout computed to be: 10000000
3: PASS 28 skybox/lighting CPU contract checks
3/4 Test #3: dx11_07_cpu ......................   Passed    0.02 sec
test 4
    Start 4: dx11_08_cpu

4: Test command: /mnt/data/dx11_batch05_08/build-clangsan/08_terrain/dx11_08_cpu
4: Working Directory: /mnt/data/dx11_batch05_08/build-clangsan/08_terrain
4: Test timeout computed to be: 10000000
4: PASS 33 terrain CPU contract checks
4/4 Test #4: dx11_08_cpu ......................   Passed    0.04 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.11 sec

```
