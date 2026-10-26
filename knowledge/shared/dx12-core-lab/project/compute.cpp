#include "dx12_common.hpp"
#include <iostream>

// 실제 compute queue -> fence -> copy queue -> readback 예제.
// CPU 모델 테스트의 통과가 이 프로그램의 Windows 실행을 뜻하지 않는다.
int main(int argc, char** argv) {
    try {
        const bool warp = argc == 2 && std::string(argv[1]) == "--warp";
        if (argc > 2 || (argc == 2 && !warp)) throw std::runtime_error("Usage: atlas_dx12_compute [--warp]");
        DeviceContext context(warp);
        EventHandle event;
        ComPtr<ID3D12CommandQueue> computeQueue, copyQueue;
        D3D12_COMMAND_QUEUE_DESC queueDesc{};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        Check(context.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(computeQueue.GetAddressOf())), "Create compute queue");
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        Check(context.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(copyQueue.GetAddressOf())), "Create copy queue");
        ComPtr<ID3D12Fence> computeDone, copyDone;
        Check(context.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(computeDone.GetAddressOf())), "Create compute fence");
        Check(context.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(copyDone.GetAddressOf())), "Create copy fence");
        ComPtr<ID3D12CommandAllocator> computeAllocator, copyAllocator;
        Check(context.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE,
            IID_PPV_ARGS(computeAllocator.GetAddressOf())), "Create compute allocator");
        Check(context.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY,
            IID_PPV_ARGS(copyAllocator.GetAddressOf())), "Create copy allocator");
        constexpr UINT Count = 64; constexpr UINT64 Bytes = Count * sizeof(std::uint32_t);
        auto output = Buffer(context.device.Get(), Bytes, D3D12_HEAP_TYPE_DEFAULT,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        auto readback = Buffer(context.device.Get(), Bytes, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST);
        D3D12_ROOT_PARAMETER params[2]{};
        params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        params[0].Descriptor = {0, 0}; params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        params[1].Constants = {0, 0, 1}; params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        D3D12_ROOT_SIGNATURE_DESC rootDesc{}; rootDesc.NumParameters = 2; rootDesc.pParameters = params;
        ComPtr<ID3DBlob> signature, errors;
        Check(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            signature.GetAddressOf(), errors.GetAddressOf()), "Serialize compute root");
        ComPtr<ID3D12RootSignature> root;
        Check(context.device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(root.GetAddressOf())), "Create compute root");
        const char* shader = R"HLSL(
RWStructuredBuffer<uint> result : register(u0);
cbuffer Bounds : register(b0) { uint count; };
[numthreads(32,1,1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    if (id.x < count) result[id.x] = id.x * id.x + 7;
}
)HLSL";
        auto cs = Compile(shader, "CSMain", "cs_5_1");
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{}; psoDesc.pRootSignature = root.Get();
        psoDesc.CS = {cs->GetBufferPointer(), cs->GetBufferSize()};
        ComPtr<ID3D12PipelineState> pso;
        Check(context.device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf())), "Create compute PSO");
        ComPtr<ID3D12GraphicsCommandList> computeList, copyList;
        Check(context.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, computeAllocator.Get(),
            pso.Get(), IID_PPV_ARGS(computeList.GetAddressOf())), "Create compute list");
        computeList->SetComputeRootSignature(root.Get());
        computeList->SetComputeRootUnorderedAccessView(0, output->GetGPUVirtualAddress());
        computeList->SetComputeRoot32BitConstant(1, Count, 0);
        computeList->Dispatch((Count + 31) / 32, 1, 1);
        // copy queue에 넘기기 전 COMMON. 상태 전환과 queue fence는 서로 다른 책임이다.
        Transition(computeList.Get(), output.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
        Check(computeList->Close(), "Close compute list");
        Check(context.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, copyAllocator.Get(),
            nullptr, IID_PPV_ARGS(copyList.GetAddressOf())), "Create copy list");
        Transition(copyList.Get(), output.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
        copyList->CopyBufferRegion(readback.Get(), 0, output.Get(), 0, Bytes);
        Transition(copyList.Get(), output.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
        Check(copyList->Close(), "Close copy list");
        ID3D12CommandList* first[] = {computeList.Get()}; computeQueue->ExecuteCommandLists(1, first);
        Check(computeQueue->Signal(computeDone.Get(), 1), "Signal compute completion");
        // GPU 큐 대기이므로 호출한 CPU 스레드는 이 지점에서 완료를 기다리지 않는다.
        Check(copyQueue->Wait(computeDone.Get(), 1), "Copy queue wait");
        ID3D12CommandList* second[] = {copyList.Get()}; copyQueue->ExecuteCommandLists(1, second);
        Check(copyQueue->Signal(copyDone.Get(), 1), "Signal copy completion");
        WaitForFence(copyDone.Get(), 1, event.value);
        void* mapped = nullptr; D3D12_RANGE readRange{0, static_cast<SIZE_T>(Bytes)};
        Check(readback->Map(0, &readRange, &mapped), "Map readback");
        bool matches = true;
        for (UINT i = 0; i < Count; ++i) {
            std::uint32_t actual = 0;
            std::memcpy(&actual, static_cast<const unsigned char*>(mapped) + i * sizeof actual, sizeof actual);
            if (actual != i * i + 7) matches = false;
        }
        D3D12_RANGE noWrite{0, 0}; readback->Unmap(0, &noWrite);
        if (!matches) throw std::runtime_error("GPU result mismatch");
        std::cout << "PASS: 64 GPU values (adapter chosen at runtime)\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
