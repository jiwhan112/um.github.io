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
static ComPtr<ID3DBlob> Compile(const char* name,const char* profile){ // [한국어 해설] 내장 raw string HLSL 바이트는 원본 그대로다. 오류 blob과 HRESULT를 숨기지 않는다.
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
 void Resize(UINT w,UINT h){ // [한국어 해설] 백 버퍼 바인딩·컨텍스트 상태·RTV 참조를 해제하고 새 크기의 뷰를 만든다.
  if(!w||!h||(width_==w&&height_==h))return;
  context_->OMSetRenderTargets(0,nullptr,nullptr);
  context_->ClearState();
  rtv_.Reset();
  Req(swap_->ResizeBuffers(0,w,h,DXGI_FORMAT_UNKNOWN,0),"ResizeBuffers");
  CreateRTV(w,h);
 }
 void CreateGeometry(){ // [한국어 해설] 초기 데이터로 immutable 정점/인덱스 버퍼를 만든다. 이후 Map 갱신 대상이 아니다.
  D3D11_BUFFER_DESC b={};b.Usage=D3D11_USAGE_IMMUTABLE;
  b.ByteWidth=sizeof(kVertex);b.BindFlags=D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA initial={};initial.pSysMem=kVertex;
  Req(device_->CreateBuffer(&b,&initial,vb_.GetAddressOf()),"Create VB");
  b.ByteWidth=sizeof(kIndices);b.BindFlags=D3D11_BIND_INDEX_BUFFER;initial.pSysMem=kIndices;
  Req(device_->CreateBuffer(&b,&initial,ib_.GetAddressOf()),"Create IB");
 }
 void CreateTexture(){ // [한국어 해설] CPU 체커 데이터와 SysMemPitch를 넘겨 텍스처를 생성하고 SRV와 샘플러를 별도 만든다.
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
 void Frame(UINT w,UINT h){ // [한국어 해설] t0/s0와 IA/VS/PS/RTV를 연결해 사각형을 그린다. Windows/GPU 경로는 이번에 미실행이다.
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
