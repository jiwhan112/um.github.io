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
