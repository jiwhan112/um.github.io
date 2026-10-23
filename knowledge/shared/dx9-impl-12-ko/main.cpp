// DX9 12: one complete source; CPU data tests are NOT GPU tests.
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

struct Vertex { float x, y, z; std::uint32_t color; float u, v; }; // [한국어 해설] 위치3float·packed색·UV2float의 24바이트 배치를 static_assert로 확인한다.
static_assert(sizeof(float)==4 && sizeof(Vertex)==24, "FVF layout requires 24 bytes");
static_assert(offsetof(Vertex,color)==12 && offsetof(Vertex,u)==16, "FVF offsets");
const std::array<Vertex,7> kVertices = {{
    {-.90f,-.50f,.5f,0xffff4040u,0,1},
    {-.55f, .50f,.5f,0xff40ff40u,.5f,0},
    {-.20f,-.50f,.5f,0xff4040ffu,1,1},
    { .15f,-.50f,.5f,0xffffffffu,0,1},
    { .15f, .50f,.5f,0xffffffffu,0,0},
    { .85f,-.50f,.5f,0xffffffffu,1,1},
    { .85f, .50f,.5f,0xffffffffu,1,0}
}};
const std::array<std::uint16_t,9> kIndices = {{0,1,2,3,4,5,5,4,6}};
constexpr unsigned kTextureSize=64;
std::array<std::uint32_t,kTextureSize*kTextureSize> MakeChecker() { // [한국어 해설] 외부 텍스처 파일 없이 64×64 체커 데이터를 만든다. CPU 데이터 검사는 GPU 업로드 성공과 별개다.
    std::array<std::uint32_t,kTextureSize*kTextureSize> pixels{};
    for(unsigned y=0;y<kTextureSize;++y)
        for(unsigned x=0;x<kTextureSize;++x)
            pixels[y*kTextureSize+x]=((x/8+y/8)%2) ? 0xff3050c0u : 0xfff0d070u;
    return pixels;
}
bool DrawRangeFits(unsigned start,unsigned primitives,unsigned min,unsigned count) { // [한국어 해설] 삼각형 수, IB 시작 위치와 선언 정점 범위를 분리해 검사한다.
    if(count==0 || primitives==0 || min>=kVertices.size() || count>kVertices.size()-min)
        return false;
    if(start>kIndices.size() || primitives>(kIndices.size()-start)/3) return false;
    for(unsigned i=start;i<start+primitives*3;++i)
        if(kIndices[i]<min || kIndices[i]>=min+count) return false;
    return true;
}

#if defined(_WIN32) && !defined(DX9_CPU_TEST)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d9.h>
#include <sstream>

struct D3DError : std::runtime_error {
    HRESULT code;
    D3DError(HRESULT hr,const char* where):std::runtime_error(where),code(hr) {}
};
void Require(HRESULT hr,const char* where) { if(FAILED(hr)) throw D3DError(hr,where); }
#define HR(call) Require((call),#call)

template<class T> class Com { // [한국어 해설] COM 참조 하나를 소유한다. put은 기존 참조를 먼저 해제하므로 사용 시점을 주의한다.
    T* p_=nullptr;
public:
    Com()=default;
    Com(const Com&)=delete;
    Com& operator=(const Com&)=delete;
    ~Com(){reset();}
    void reset(){T* old=p_;p_=nullptr;if(old)old->Release();}
    T** put(){reset();return &p_;}
    T* get()const{return p_;}
    T* operator->()const{return p_;}
};
struct Input { bool quit=false; bool forceReset=false; };
LRESULT CALLBACK WindowProc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp) {
    Input* input=reinterpret_cast<Input*>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
    if(message==WM_NCCREATE) {
        input=static_cast<Input*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
        SetWindowLongPtrW(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(input));
    }
    if(input) {
        if(message==WM_CLOSE){input->quit=true;return 0;}
        if(message==WM_KEYDOWN && wp==VK_ESCAPE){input->quit=true;return 0;}
        if(message==WM_KEYDOWN && wp=='R'){input->forceReset=true;return 0;}
    }
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(hwnd,message,wp,lp);
}
struct WindowOwner {
    HINSTANCE instance; HWND hwnd=nullptr; bool registered=false;
    explicit WindowOwner(HINSTANCE i):instance(i) {}
    ~WindowOwner(){if(hwnd && IsWindow(hwnd))DestroyWindow(hwnd);
        if(registered)UnregisterClassW(L"DX9TriangleQuad12",instance);}
};
class Demo { // [한국어 해설] Windows 분기 전체 렌더러다. 이번 Linux 검증은 아래 CPU 분기만 실행한다.
    HWND hwnd_;
    Com<IDirect3D9> d3d_;
    Com<IDirect3DDevice9> device_;
    Com<IDirect3DVertexBuffer9> vb_;
    Com<IDirect3DIndexBuffer9> ib_;
    Com<IDirect3DTexture9> texture_;
    UINT width_=0,height_=0;
    bool ready_=false;
    D3DPRESENT_PARAMETERS Parameters(UINT w,UINT h)const {
        D3DPRESENT_PARAMETERS pp{};
        pp.Windowed=TRUE;pp.hDeviceWindow=hwnd_;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
        pp.BackBufferFormat=D3DFMT_UNKNOWN;pp.BackBufferWidth=w;pp.BackBufferHeight=h;
        pp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
        pp.EnableAutoDepthStencil=FALSE;
        return pp;
    }
    void CreateBuffers() { // [한국어 해설] DEFAULT VB/IB는 Reset 때 해제하고 재생성한다. 생성된 CPU 데이터에서 복사한다.
        HR(device_->CreateVertexBuffer(static_cast<UINT>(sizeof(kVertices)),
            D3DUSAGE_WRITEONLY,D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1,
            D3DPOOL_DEFAULT,vb_.put(),nullptr));
        void* data=nullptr;
        HR(vb_->Lock(0,static_cast<UINT>(sizeof(kVertices)),&data,0));
        std::memcpy(data,kVertices.data(),sizeof(kVertices));
        HR(vb_->Unlock());
        HR(device_->CreateIndexBuffer(static_cast<UINT>(sizeof(kIndices)),
            D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,ib_.put(),nullptr));
        HR(ib_->Lock(0,static_cast<UINT>(sizeof(kIndices)),&data,0));
        std::memcpy(data,kIndices.data(),sizeof(kIndices));
        HR(ib_->Unlock());
        ready_=true;
    }
    void CreateTexture() { // [한국어 해설] MANAGED 텍스처는 실제 Pitch를 검사한 뒤 행별 복사한다. 연속 메모리라고 가정하지 않는다.
        HR(device_->CreateTexture(kTextureSize,kTextureSize,1,0,D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED,texture_.put(),nullptr));
        const auto pixels=MakeChecker();
        D3DLOCKED_RECT locked{};
        HR(texture_->LockRect(0,&locked,nullptr,0));
        if(!locked.pBits || locked.Pitch<static_cast<int>(kTextureSize*4)) {
            const HRESULT cleanup=texture_->UnlockRect(0);
            if(FAILED(cleanup))OutputDebugStringA("Texture cleanup UnlockRect failed\n");
            throw std::runtime_error("unsupported texture pitch");
        }
        for(unsigned y=0;y<kTextureSize;++y)
            std::memcpy(static_cast<unsigned char*>(locked.pBits)+
                static_cast<std::size_t>(y)*static_cast<unsigned>(locked.Pitch),
                pixels.data()+y*kTextureSize,kTextureSize*4);
        HR(texture_->UnlockRect(0));
    }
    void StateAndDraw() { // [한국어 해설] 고정 기능 파이프라인의 핵심 상태를 명시하고 색 삼각형과 체커 사각형을 따로 그린다.
        D3DMATRIX identity{};
        identity._11=identity._22=identity._33=identity._44=1.0f;
        HR(device_->SetVertexShader(nullptr));HR(device_->SetPixelShader(nullptr));
        HR(device_->SetTransform(D3DTS_WORLD,&identity));
        HR(device_->SetTransform(D3DTS_VIEW,&identity));
        HR(device_->SetTransform(D3DTS_PROJECTION,&identity));
        D3DVIEWPORT9 viewport{0,0,width_,height_,0.0f,1.0f};
        HR(device_->SetViewport(&viewport));
        HR(device_->SetRenderState(D3DRS_LIGHTING,FALSE));
        HR(device_->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE));
        HR(device_->SetRenderState(D3DRS_ZWRITEENABLE,FALSE));
        HR(device_->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE));
        HR(device_->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE));
        HR(device_->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE));
        HR(device_->SetRenderState(D3DRS_FOGENABLE,FALSE));
        HR(device_->SetRenderState(D3DRS_STENCILENABLE,FALSE));
        HR(device_->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE));
        HR(device_->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID));
        HR(device_->SetRenderState(D3DRS_COLORWRITEENABLE,0x0f));
        HR(device_->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1));
        HR(device_->SetStreamSource(0,vb_.get(),0,sizeof(Vertex)));
        HR(device_->SetIndices(ib_.get()));
        HR(device_->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0));
        HR(device_->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE));
        HR(device_->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1));
        HR(device_->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE));
        HR(device_->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1));
        HR(device_->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE));
        HR(device_->SetTexture(0,nullptr));
        HR(device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,3,0,1));
        HR(device_->SetTexture(0,texture_.get()));
        HR(device_->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE));
        HR(device_->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE));
        HR(device_->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE));
        HR(device_->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT));
        HR(device_->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT));
        HR(device_->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE));
        HR(device_->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP));
        HR(device_->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP));
        HR(device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,3,4,3,2));
        HR(device_->SetTexture(0,nullptr));
    }
public:
    Demo(HWND hwnd,UINT w,UINT h):hwnd_(hwnd) {
        if(w==0||h==0)throw std::invalid_argument("zero client size");
        *d3d_.put()=Direct3DCreate9(D3D_SDK_VERSION);
        if(!d3d_.get())throw std::runtime_error("Direct3DCreate9 returned null");
        auto pp=Parameters(w,h);
        HR(d3d_->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,device_.put()));
        width_=w;height_=h;
        CreateTexture();CreateBuffers();
    }
    bool Frame(UINT w,UINT h,bool forceReset) { // [한국어 해설] 장치 협력 상태·크기 변경·수동 Reset 요청을 처리한다. Reset 전에 DEFAULT 자원 참조를 해제한다.
        HRESULT cooperative=device_->TestCooperativeLevel();
        if(cooperative==D3DERR_DEVICELOST)return false;
        if(cooperative!=D3DERR_DEVICENOTRESET)Require(cooperative,"TestCooperativeLevel");
        if(forceReset || w!=width_ || h!=height_ || cooperative==D3DERR_DEVICENOTRESET || !ready_) {
            if(ready_) {
                HR(device_->SetStreamSource(0,nullptr,0,0));HR(device_->SetIndices(nullptr));
            }
            vb_.reset();ib_.reset();ready_=false;
            auto pp=Parameters(w,h);
            const HRESULT hr=device_->Reset(&pp);
            if(hr==D3DERR_DEVICELOST)return false;
            Require(hr,"Reset");
            width_=w;height_=h;
            CreateBuffers();
            OutputDebugStringA("DX9 Reset success: default VB/IB recreated\n");
        }
        HR(device_->Clear(0,nullptr,D3DCLEAR_TARGET,D3DCOLOR_XRGB(25,30,40),1.0f,0));
        HR(device_->BeginScene());
        try { StateAndDraw(); }
        catch(...) { if(FAILED(device_->EndScene()))OutputDebugStringA("EndScene cleanup failed\n"); throw; }
        HR(device_->EndScene());
        const HRESULT present=device_->Present(nullptr,nullptr,nullptr,nullptr);
        if(present==D3DERR_DEVICELOST)return false;
        Require(present,"Present");
        return true;
    }
};
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int show) { // [한국어 해설] 메시지 처리에서는 요청만 기록한다. 렌더 루프가 장치 작업을 수행한다.
    Input input;
    WindowOwner window(instance);
    try {
        WNDCLASSW wc{};wc.lpfnWndProc=WindowProc;wc.hInstance=instance;
        wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.lpszClassName=L"DX9TriangleQuad12";
        if(!RegisterClassW(&wc))throw std::runtime_error("RegisterClassW failed");
        window.registered=true;
        window.hwnd=CreateWindowExW(0,wc.lpszClassName,L"DX9: triangle + checker quad (R: Reset, Esc: quit)",
            WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,960,640,nullptr,nullptr,instance,&input);
        if(!window.hwnd)throw std::runtime_error("CreateWindowExW failed");
        ShowWindow(window.hwnd,show);
        RECT rect{};
        if(!GetClientRect(window.hwnd,&rect))throw std::runtime_error("GetClientRect failed");
        Demo demo(window.hwnd,static_cast<UINT>(rect.right),static_cast<UINT>(rect.bottom));
        while(!input.quit) {
            MSG msg{};
            while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {
                if(msg.message==WM_QUIT){input.quit=true;break;}
                TranslateMessage(&msg);DispatchMessageW(&msg);
            }
            if(input.quit)break;
            if(IsIconic(window.hwnd)){Sleep(30);continue;}
            if(!GetClientRect(window.hwnd,&rect))throw std::runtime_error("GetClientRect failed");
            if(rect.right<=0||rect.bottom<=0){Sleep(30);continue;}
            const bool reset=input.forceReset;input.forceReset=false;
            try {
                if(!demo.Frame(static_cast<UINT>(rect.right),static_cast<UINT>(rect.bottom),reset))Sleep(30);
            } catch(const D3DError& e) {
                if(e.code==D3DERR_DEVICELOST){Sleep(30);continue;}
                throw;
            }
        }
    } catch(const D3DError& e) {
        std::ostringstream out;out<<e.what()<<"\nHRESULT=0x"<<std::hex<<static_cast<unsigned long>(e.code);
        MessageBoxA(window.hwnd,out.str().c_str(),"D3D9 failure",MB_OK|MB_ICONERROR);return 1;
    } catch(const std::exception& e) {
        MessageBoxA(window.hwnd,e.what(),"Initialization failure",MB_OK|MB_ICONERROR);return 2;
    }
    return 0;
}
#else
int main() { // [한국어 해설] CPU에서는 데이터 배치·그리기 범위·체커·클립과 UV만 검사한다. Windows 렌더러는 미실행이다.
    int checks=0;
    auto check=[&](bool ok,const char* name){++checks;if(!ok)throw std::runtime_error(name);};
    try {
        check(sizeof(kVertices)==168,"vertex bytes");
        check(sizeof(kIndices)==18,"index bytes");
        check(DrawRangeFits(0,1,0,3),"triangle range");
        check(DrawRangeFits(3,2,3,4),"quad range");
        check(!DrawRangeFits(3,3,3,4),"too many primitives");
        check(!DrawRangeFits(3,2,3,3),"wrong declared vertex range");
        check(!DrawRangeFits(10,1,0,3),"bad start");
        check(!DrawRangeFits(0,0,0,3),"zero draw policy");
        const auto pixels=MakeChecker();
        check(pixels.size()==4096,"pixel count");
        check(pixels[0]==0xfff0d070u,"checker origin");
        check(pixels[8]==0xff3050c0u,"checker next column tile");
        check(pixels[8*64]==0xff3050c0u,"checker next row tile");
        check(pixels[8*64+8]==pixels[0],"checker diagonal tile");
        bool clips=true,uv=true,opaque=true;
        for(const auto& v:kVertices){clips=clips&&v.x>=-1&&v.x<=1&&v.y>=-1&&v.y<=1&&v.z>=0&&v.z<=1;
            uv=uv&&v.u>=0&&v.u<=1&&v.v>=0&&v.v<=1;opaque=opaque&&(v.color>>24)==255;}
        check(clips,"identity clip coordinates");check(uv,"uv range");check(opaque,"vertex alpha");
        std::cout<<"PASS "<<checks<<" data checks (CPU only; Windows renderer NOT RUN)\n";
    } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
#endif