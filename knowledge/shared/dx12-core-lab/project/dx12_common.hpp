#pragma once
#include <windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <cstdint>
#include <climits>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
using Microsoft::WRL::ComPtr;

// HRESULT 실패를 소멸자 밖에서 예외로 전환한다. 호출 이름도 오류에 남긴다.
inline void Check(HRESULT hr, const char* operation) {
    if (FAILED(hr)) {
        char text[192];
        std::snprintf(text, sizeof text, "%s failed: HRESULT=0x%08lX", operation,
                      static_cast<unsigned long>(hr));
        throw std::runtime_error(text);
    }
}
struct EventHandle {
    HANDLE value = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    EventHandle() {
        if (!value) throw std::runtime_error("CreateEvent failed");
    }
    ~EventHandle() { CloseHandle(value); }
    EventHandle(const EventHandle&) = delete;
    EventHandle& operator=(const EventHandle&) = delete;
};
inline void WaitForFence(ID3D12Fence* fence, std::uint64_t value, HANDLE event) {
    const auto completed = fence->GetCompletedValue();
    if (completed == UINT64_MAX) throw std::runtime_error("Device removed during fence query");
    if (completed >= value) return;
    Check(fence->SetEventOnCompletion(value, event), "SetEventOnCompletion");
    // 장치 오류 때 무한 대기하지 않는다. 타임아웃은 성공으로 취급하지 않는다.
    if (WaitForSingleObject(event, 10000) != WAIT_OBJECT_0)
        throw std::runtime_error("Fence wait failed or timed out (10 seconds)");
    const auto after = fence->GetCompletedValue();
    if (after == UINT64_MAX || after < value)
        throw std::runtime_error("Fence completion is not valid");
}
inline ComPtr<ID3DBlob> Compile(const char* source, const char* entry, const char* target) {
    ComPtr<ID3DBlob> bytecode, errors;
    const HRESULT hr = D3DCompile(source, std::strlen(source), "atlas_original_shader",
        nullptr, nullptr, entry, target, D3DCOMPILE_ENABLE_STRICTNESS, 0,
        bytecode.GetAddressOf(), errors.GetAddressOf());
    if (FAILED(hr)) {
        if (errors) OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
        Check(hr, "D3DCompile");
    }
    return bytecode;
}
inline D3D12_HEAP_PROPERTIES Heap(D3D12_HEAP_TYPE type) {
    D3D12_HEAP_PROPERTIES p{};
    p.Type = type; p.CreationNodeMask = 1; p.VisibleNodeMask = 1;
    return p;
}
inline D3D12_RESOURCE_DESC BufferDesc(UINT64 bytes) {
    D3D12_RESOURCE_DESC d{};
    d.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; d.Width = bytes;
    d.Height = 1; d.DepthOrArraySize = 1; d.MipLevels = 1;
    d.SampleDesc.Count = 1; d.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    return d;
}
inline ComPtr<ID3D12Resource> Buffer(ID3D12Device* device, UINT64 bytes,
    D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES state,
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) {
    auto heap = Heap(heapType); auto desc = BufferDesc(bytes); desc.Flags = flags;
    ComPtr<ID3D12Resource> result;
    Check(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, state,
        nullptr, IID_PPV_ARGS(result.GetAddressOf())), "CreateCommittedResource(buffer)");
    return result;
}
inline void Transition(ID3D12GraphicsCommandList* list, ID3D12Resource* resource,
    D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
    if (before == after) return;
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition = {resource, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, before, after};
    list->ResourceBarrier(1, &barrier);
}
struct DeviceContext {
    ComPtr<IDXGIFactory6> factory;
    ComPtr<ID3D12Device> device;
    explicit DeviceContext(bool warp) {
        UINT factoryFlags = 0;
#ifndef NDEBUG
        // 개발 시 Graphics Tools 설치가 없으면 명확히 실패한다. 검증을 조용히 끄지 않는다.
        ComPtr<ID3D12Debug> debug;
        Check(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())), "D3D12 debug layer unavailable");
        debug->EnableDebugLayer(); factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
        Check(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(factory.GetAddressOf())), "CreateDXGIFactory2");
        ComPtr<IDXGIAdapter1> adapter;
        if (warp) {
            Check(factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.GetAddressOf())), "EnumWarpAdapter");
            Check(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(device.GetAddressOf())), "Create WARP device");
        } else {
            for (UINT index = 0; ; ++index) {
                adapter.Reset();
                HRESULT hr = factory->EnumAdapters1(index, adapter.GetAddressOf());
                if (hr == DXGI_ERROR_NOT_FOUND) break;
                Check(hr, "EnumAdapters1");
                DXGI_ADAPTER_DESC1 desc{}; Check(adapter->GetDesc1(&desc), "GetDesc1");
                if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) continue;
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                    IID_PPV_ARGS(device.GetAddressOf())))) break;
            }
            if (!device) throw std::runtime_error("No D3D12 hardware adapter. Retry explicitly with --warp.");
        }
        Check(device->SetName(L"ATLAS original educational device"), "SetName(device)");
    }
};
