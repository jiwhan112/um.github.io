#pragma once

#include <cassert>
#include <utility>

// Owns exactly one COM reference. The raw pointer passed to reset is adopted:
// it must already represent a reference owned by the caller.
template <class T>
class ComOwner {
public:
    ComOwner() noexcept : ptr_(nullptr) {}
    explicit ComOwner(T* ptr) noexcept : ptr_(ptr) {}

    ~ComOwner() { reset(); }

    ComOwner(const ComOwner&) = delete;
    ComOwner& operator=(const ComOwner&) = delete;

    ComOwner(ComOwner&& other) noexcept : ptr_(other.detach()) {}
    ComOwner& operator=(ComOwner&& other) noexcept {
        if (this != &other) reset(other.detach());
        return *this;
    }

    T* get() const noexcept { return ptr_; }
    T* operator->() const noexcept { assert(ptr_); return ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    // For COM out-parameters. Releases any previous reference first.
    T** put() noexcept { reset(); return &ptr_; }

    T* detach() noexcept {
        T* old = ptr_;
        ptr_ = nullptr;
        return old;
    }

    void reset(T* replacement = nullptr) noexcept {
        if (ptr_ == replacement) return; // Never release-and-retain same pointer.
        T* old = ptr_;
        ptr_ = replacement;
        if (old) old->Release();
    }

private:
    T* ptr_;
};
