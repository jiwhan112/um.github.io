#pragma once

#include <cassert>
#include <utility>

// Owns exactly one COM reference. The raw pointer passed to reset is adopted:
// it must already represent a reference owned by the caller.
template <class T> // [한국어 해설] 정확히 COM 참조 하나를 소유한다. raw pointer를 받으면 AddRef하지 않고 이미 소유한 참조를 인수한다.
class ComOwner {
public:
    ComOwner() noexcept : ptr_(nullptr) {}
    explicit ComOwner(T* ptr) noexcept : ptr_(ptr) {}

    ~ComOwner() { reset(); }

    ComOwner(const ComOwner&) = delete;
    ComOwner& operator=(const ComOwner&) = delete;

    ComOwner(ComOwner&& other) noexcept : ptr_(other.detach()) {} // [한국어 해설] 이동은 참조 카운트를 늘리지 않고 기존 포인터 소유권만 이전한다.
    ComOwner& operator=(ComOwner&& other) noexcept {
        if (this != &other) reset(other.detach());
        return *this;
    }

    T* get() const noexcept { return ptr_; }
    T* operator->() const noexcept { assert(ptr_); return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    // For COM out-parameters. Releases any previous reference first.
    T** put() noexcept { reset(); return &ptr_; } // [한국어 해설] out 매개변수로 쓰기 전에 기존 참조를 해제한다. API 실패 후 소유 상태도 확인해야 한다.

    T* detach() noexcept { // [한국어 해설] 해제하지 않고 포인터를 넘긴다. 호출자가 참조의 새 소유자가 되어야 한다.
        T* old = ptr_;
        ptr_ = nullptr;
        return old;
    }

    void reset(T* replacement = nullptr) noexcept { // [한국어 해설] 같은 포인터면 유지한다. 두 소유자가 필요하면 별도 AddRef한 참조를 전달해야 한다.
        if (ptr_ == replacement) return; // Never release-and-retain same pointer.
        T* old = ptr_;
        ptr_ = replacement;
        if (old) old->Release();
    }

private:
    T* ptr_;
};
