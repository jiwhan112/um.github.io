#include "check.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <new>
#include <vector>
struct Pool {
    int allocated{}, freed{};
    void* Allocate(std::size_t bytes) { void* p = ::operator new(bytes); ++allocated; return p; }
    void Free(void* p) noexcept { if (p) { ++freed; ::operator delete(p); } }
};
struct Widget {
    Pool* pool;
    explicit Widget(Pool& source, bool fail) : pool(&source) {
        if (fail) throw std::runtime_error("construction");
    }
    static void* operator new(std::size_t bytes, Pool& source) { return source.Allocate(bytes); }
    static void operator delete(void* p, Pool& source) noexcept { source.Free(p); }
};
struct ReturnToPool {
    Pool* pool;
    void operator()(Widget* p) const noexcept { if (p) { std::destroy_at(p); pool->Free(p); } }
};
int main() { return Run("allocation", [](Checks& c) {
    Pool pool;
    c.Throws<std::runtime_error>([&] { (void)new(pool) Widget(pool, true); }, "placement ctor failure");
    c.Expect(pool.allocated == 1 && pool.freed == 1, "matching placement delete");
    {
        std::unique_ptr<Widget, ReturnToPool> owner(new(pool) Widget(pool, false), ReturnToPool{&pool});
        c.Expect(pool.allocated == 2 && pool.freed == 1, "live custom allocation");
    }
    c.Expect(pool.allocated == 2 && pool.freed == 2, "custom deleter on success");
    alignas(int) std::array<std::byte, sizeof(int)> bytes{};
    int* value = ::new(static_cast<void*>(bytes.data())) int(42);
    c.Expect(*value == 42, "nonallocating placement new"); std::destroy_at(value);
    // Do not delete value: bytes owns this automatic storage.
    alignas(std::max_align_t) std::array<std::byte, 1024> arena{};
    std::pmr::monotonic_buffer_resource resource(arena.data(), arena.size(), std::pmr::null_memory_resource());
    {
        std::pmr::vector<int> values(&resource); values.reserve(8); values.push_back(7);
        c.Expect(values[0] == 7, "explicit pmr resource");
    }
    c.Throws<std::bad_alloc>([&] { (void)resource.allocate(4096); }, "bounded deterministic failure");
}); }
