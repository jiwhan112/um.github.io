#include "dx12_common.hpp"
#include <array>
#include <memory>

// ATLAS NOTE 자체 제작 예제. 고정 960x540 창, RGBA8, mip 1개, 깊이 없음.
// CPU 단원 테스트와 달리 이 파일은 실제 Windows SDK/D3D12 장치를 요구한다.
class Renderer {
    static constexpr UINT Count = 2;
    DeviceContext context;
    EventHandle event;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<IDXGISwapChain3> swap;
    ComPtr<ID3D12DescriptorHeap> rtvHeap, srvHeap;
    std::array<ComPtr<ID3D12Resource>, Count> backBuffers;
    std::array<ComPtr<ID3D12CommandAllocator>, Count> allocators;
    std::array<UINT64, Count> lastUse{};
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    ComPtr<ID3D12RootSignature> root;
    ComPtr<ID3D12PipelineState> pipeline;
    ComPtr<ID3D12Resource> texture;
    UINT rtvStride = 0;
    UINT64 nextFence = 1;

    UINT64 Signal() {
        if (nextFence == UINT64_MAX) throw std::runtime_error("Fence timeline exhausted");
        const UINT64 value = nextFence++;
        Check(queue->Signal(fence.Get(), value), "Signal");
        return value;
    }
    void CreateBindingsAndPipeline() {
        // t0 하나를 담는 descriptor table + b0의 float4 root constants.
        D3D12_DESCRIPTOR_RANGE range{};
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors = 1; range.BaseShaderRegister = 0;
        range.OffsetInDescriptorsFromTableStart = 0;
        D3D12_ROOT_PARAMETER parameters[2]{};
        parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[0].DescriptorTable = {1, &range};
        parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        parameters[1].Constants = {0, 0, 4};
        parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        sampler.MaxAnisotropy = 1; sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0; sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        D3D12_ROOT_SIGNATURE_DESC desc{};
        desc.NumParameters = 2; desc.pParameters = parameters;
        desc.NumStaticSamplers = 1; desc.pStaticSamplers = &sampler;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        ComPtr<ID3DBlob> serialized, errors;
        const auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
            serialized.GetAddressOf(), errors.GetAddressOf());
        if (FAILED(hr) && errors) OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
        Check(hr, "SerializeRootSignature");
        Check(context.device->CreateRootSignature(0, serialized->GetBufferPointer(),
            serialized->GetBufferSize(), IID_PPV_ARGS(root.GetAddressOf())), "CreateRootSignature");
        // 외부 파일을 읽지 않는 짧은 자체 HLSL. 최신 SM6/DXC 기능의 예제가 아니다.
        const char* shader = R"HLSL(
Texture2D<float4> checker : register(t0);
SamplerState nearestSampler : register(s0);
cbuffer Material : register(b0) { float4 tint; };
struct VertexOut { float4 position : SV_Position; float2 uv : TEXCOORD0; };
VertexOut VSMain(uint vertex : SV_VertexID) {
    const float2 positions[3] = { float2(-0.8,-0.7), float2(0,0.8), float2(0.8,-0.7) };
    const float2 uv[3] = { float2(0,1), float2(0.5,0), float2(1,1) };
    VertexOut result; result.position = float4(positions[vertex],0.5,1);
    result.uv = uv[vertex]; return result;
}
float4 PSMain(VertexOut input) : SV_Target {
    return checker.Sample(nearestSampler, input.uv) * tint;
}
)HLSL";
        auto vs = Compile(shader, "VSMain", "vs_5_1");
        auto ps = Compile(shader, "PSMain", "ps_5_1");
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
        pso.pRootSignature = root.Get();
        pso.VS = {vs->GetBufferPointer(), vs->GetBufferSize()};
        pso.PS = {ps->GetBufferPointer(), ps->GetBufferSize()};
        auto& blend = pso.BlendState.RenderTarget[0];
        blend.SrcBlend = D3D12_BLEND_ONE; blend.DestBlend = D3D12_BLEND_ZERO;
        blend.BlendOp = D3D12_BLEND_OP_ADD;
        blend.SrcBlendAlpha = D3D12_BLEND_ONE; blend.DestBlendAlpha = D3D12_BLEND_ZERO;
        blend.BlendOpAlpha = D3D12_BLEND_OP_ADD; blend.LogicOp = D3D12_LOGIC_OP_NOOP;
        blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        pso.SampleMask = UINT_MAX;
        pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pso.RasterizerState.DepthClipEnable = TRUE;
        pso.DepthStencilState.DepthEnable = FALSE;
        pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        pso.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        pso.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        pso.DepthStencilState.FrontFace = {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP,
            D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS};
        pso.DepthStencilState.BackFace = pso.DepthStencilState.FrontFace;
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets = 1; pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count = 1;
        Check(context.device->CreateGraphicsPipelineState(&pso,
            IID_PPV_ARGS(pipeline.GetAddressOf())), "CreateGraphicsPipelineState");
    }
    void UploadTexture() {
        constexpr UINT Width = 8, Height = 8;
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; desc.Width = Width;
        desc.Height = Height; desc.DepthOrArraySize = 1; desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
        auto heap = Heap(D3D12_HEAP_TYPE_DEFAULT);
        Check(context.device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(texture.GetAddressOf())), "Create texture");
        Check(texture->SetName(L"Checker 8x8 mip0"), "SetName(texture)");
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        UINT rows = 0; UINT64 rowBytes = 0, total = 0;
        context.device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &rows, &rowBytes, &total);
        if (total == UINT64_MAX || rows != Height || rowBytes != Width * 4)
            throw std::runtime_error("Unexpected texture footprint");
        auto upload = Buffer(context.device.Get(), total, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
        void* mapped = nullptr; D3D12_RANGE noRead{0, 0};
        Check(upload->Map(0, &noRead, &mapped), "Map upload");
        auto* base = static_cast<unsigned char*>(mapped) + footprint.Offset;
        // 데이터 32바이트와 행 피치 256바이트를 혼동하지 않는다.
        for (UINT y = 0; y < Height; ++y) {
            auto* row = base + static_cast<size_t>(y) * footprint.Footprint.RowPitch;
            for (UINT x = 0; x < Width; ++x) {
                const bool bright = ((x / 2 + y / 2) % 2) == 0;
                row[4*x] = bright ? 240 : 40; row[4*x+1] = bright ? 200 : 80;
                row[4*x+2] = bright ? 80 : 200; row[4*x+3] = 255;
            }
        }
        upload->Unmap(0, nullptr);
        Check(allocators[0]->Reset(), "Reset upload allocator");
        Check(list->Reset(allocators[0].Get(), nullptr), "Reset upload list");
        D3D12_TEXTURE_COPY_LOCATION source{};
        source.pResource = upload.Get(); source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        source.PlacedFootprint = footprint;
        D3D12_TEXTURE_COPY_LOCATION destination{};
        destination.pResource = texture.Get(); destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        list->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
        Transition(list.Get(), texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        Check(list->Close(), "Close upload list");
        ID3D12CommandList* lists[] = {list.Get()}; queue->ExecuteCommandLists(1, lists);
        // upload를 scope 밖으로 해제하기 전에 복사 완료를 기다린다.
        Drain();
        D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
        srv.Format = desc.Format; srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv.Texture2D.MipLevels = 1;
        context.device->CreateShaderResourceView(texture.Get(), &srv, srvHeap->GetCPUDescriptorHandleForHeapStart());
    }
public:
    Renderer(HWND window, bool warp) : context(warp) {
        D3D12_COMMAND_QUEUE_DESC q{}; q.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        Check(context.device->CreateCommandQueue(&q, IID_PPV_ARGS(queue.GetAddressOf())), "CreateCommandQueue");
        Check(context.device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(fence.GetAddressOf())), "CreateFence");
        DXGI_SWAP_CHAIN_DESC1 sc{};
        sc.Width = 960; sc.Height = 540; sc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sc.SampleDesc.Count = 1; sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sc.BufferCount = Count; sc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        ComPtr<IDXGISwapChain1> temporary;
        Check(context.factory->CreateSwapChainForHwnd(queue.Get(), window, &sc, nullptr,
            nullptr, temporary.GetAddressOf()), "CreateSwapChainForHwnd");
        Check(temporary.As(&swap), "Query swapchain3");
        Check(context.factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER), "MakeWindowAssociation");
        D3D12_DESCRIPTOR_HEAP_DESC rtv{}; rtv.NumDescriptors = Count; rtv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        Check(context.device->CreateDescriptorHeap(&rtv, IID_PPV_ARGS(rtvHeap.GetAddressOf())), "Create RTV heap");
        rtvStride = context.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < Count; ++i) {
            Check(swap->GetBuffer(i, IID_PPV_ARGS(backBuffers[i].GetAddressOf())), "GetBuffer");
            context.device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
            handle.ptr += rtvStride;
            Check(context.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(allocators[i].GetAddressOf())), "CreateCommandAllocator");
        }
        D3D12_DESCRIPTOR_HEAP_DESC srv{}; srv.NumDescriptors = 1;
        srv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; srv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        Check(context.device->CreateDescriptorHeap(&srv, IID_PPV_ARGS(srvHeap.GetAddressOf())), "Create SRV heap");
        Check(context.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators[0].Get(),
            nullptr, IID_PPV_ARGS(list.GetAddressOf())), "CreateCommandList");
        Check(list->Close(), "Close initial list");
        CreateBindingsAndPipeline(); UploadTexture();
    }
    void Drain() { WaitForFence(fence.Get(), Signal(), event.value); }
    ~Renderer() {
        // 정상 종료의 Drain은 main에서 오류를 검사한다. 소멸자는 예외를 던지지 않는다.
        if (queue && fence) { try { Drain(); } catch (...) { OutputDebugStringA("Cleanup drain failed. Device recovery not implemented.\n"); } }
    }
    void Draw() {
        const UINT frame = swap->GetCurrentBackBufferIndex();
        WaitForFence(fence.Get(), lastUse[frame], event.value);
        Check(allocators[frame]->Reset(), "Reset frame allocator");
        Check(list->Reset(allocators[frame].Get(), pipeline.Get()), "Reset frame list");
        list->SetGraphicsRootSignature(root.Get());
        ID3D12DescriptorHeap* heaps[] = {srvHeap.Get()}; list->SetDescriptorHeaps(1, heaps);
        list->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
        const float tint[4] = {1, 1, 1, 1}; list->SetGraphicsRoot32BitConstants(1, 4, tint, 0);
        const D3D12_VIEWPORT viewport{0, 0, 960, 540, 0, 1};
        const D3D12_RECT scissor{0, 0, 960, 540};
        list->RSSetViewports(1, &viewport); list->RSSetScissorRects(1, &scissor);
        Transition(list.Get(), backBuffers[frame].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto rtv = rtvHeap->GetCPUDescriptorHandleForHeapStart(); rtv.ptr += static_cast<SIZE_T>(frame) * rtvStride;
        list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        const float clear[4] = {0.04f, 0.06f, 0.10f, 1}; list->ClearRenderTargetView(rtv, clear, 0, nullptr);
        list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        list->DrawInstanced(3, 1, 0, 0);
        Transition(list.Get(), backBuffers[frame].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        Check(list->Close(), "Close frame list");
        ID3D12CommandList* lists[] = {list.Get()}; queue->ExecuteCommandLists(1, lists);
        const HRESULT present = swap->Present(1, 0);
        if (FAILED(present)) {
            Check(context.device->GetDeviceRemovedReason(), "GetDeviceRemovedReason");
            Check(present, "Present");
        }
        lastUse[frame] = Signal();
    }
};

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM w, LPARAM l) {
    if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
    if (message == WM_PAINT) { PAINTSTRUCT ps{}; BeginPaint(window, &ps); EndPaint(window, &ps); return 0; }
    return DefWindowProcW(window, message, w, l);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int show) {
    HWND window = nullptr;
    try {
        WNDCLASSW wc{}; wc.lpfnWndProc = WindowProc; wc.hInstance = instance;
        wc.lpszClassName = L"AtlasDx12Original"; wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        if (!RegisterClassW(&wc)) throw std::runtime_error("RegisterClass failed");
        const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT rect{0, 0, 960, 540}; if (!AdjustWindowRect(&rect, style, FALSE)) throw std::runtime_error("AdjustWindowRect failed");
        window = CreateWindowW(wc.lpszClassName, L"ATLAS DX12 - original texture/fence lab", style,
            CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
            nullptr, nullptr, instance, nullptr);
        if (!window) throw std::runtime_error("CreateWindow failed");
        Renderer renderer(window, std::wstring(commandLine).find(L"--warp") != std::wstring::npos);
        ShowWindow(window, show);
        MSG message{};
        bool running = true;
        while (running) {
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
                if (message.message == WM_QUIT) { running = false; break; }
                TranslateMessage(&message); DispatchMessageW(&message);
            }
            if (!running) break;
            if (IsIconic(window)) { Sleep(20); continue; }
            renderer.Draw();
        }
        renderer.Drain(); return 0;
    } catch (const std::exception& error) {
        MessageBoxA(nullptr, error.what(), "ATLAS DX12 failure", MB_OK | MB_ICONERROR);
        if (window && IsWindow(window)) DestroyWindow(window);
        return 1;
    }
}
