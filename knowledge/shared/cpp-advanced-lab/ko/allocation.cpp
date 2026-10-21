#include "check.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <new>
#include <vector>

// 이름은 Pool이지만 블록 재사용이나 자체 arena를 구현한 일반 목적 풀은 아니다.
// 전역 할당·해제 호출에 카운터를 붙여 두 정리 경로를 관찰하는 최소 예제다.
// 일반 operator new(bytes)를 사용하므로 확장 정렬 타입을 위한 별도 계약은 없다.
struct Pool {
    int allocated{}, freed{};
    void* Allocate(std::size_t bytes) {
        // 할당이 먼저 성공해야 allocated가 증가한다. 실제 메모리 고갈을 유도하지 않는다.
        void* p = ::operator new(bytes);
        ++allocated;
        return p;
    }
    void Free(void* p) noexcept {
        if (p) {
            ++freed;
            ::operator delete(p);
        }
    }
};

struct Widget {
    Pool* pool;
    explicit Widget(Pool& source, bool fail) : pool(&source) {
        // 저장소 할당 뒤 생성자 본문에서 의도적으로 예외를 던진다.
        // 완성되지 않은 Widget의 소멸자는 호출되지 않는다.
        if (fail) throw std::runtime_error("construction");
    }
    // new(pool) 표현식의 추가 인자 Pool&를 받는 사용자 정의 할당 함수다.
    static void* operator new(std::size_t bytes, Pool& source) { return source.Allocate(bytes); }
    // 생성자가 던지면 new 표현식이 추가 인자 형식이 맞는 이 delete를 찾아 호출한다.
    // 정상 생성된 객체의 수명이 끝날 때 자동으로 이 함수가 선택되는 것은 아니다.
    static void operator delete(void* p, Pool& source) noexcept { source.Free(p); }
};

struct ReturnToPool {
    // deleter는 Pool을 소유하지 않는다. owner보다 Pool이 오래 살아 있어야 한다.
    Pool* pool;
    void operator()(Widget* p) const noexcept {
        if (p) {
            // 정상 생성 경로에서는 객체 수명을 먼저 끝낸 뒤 같은 저장소를 반환한다.
            // destroy_at은 객체 소멸이고 Free는 저장소 해제로 서로 다른 단계다.
            std::destroy_at(p);
            pool->Free(p);
        }
    }
};

int main() { return Run("allocation", [](Checks& c) {
    Pool pool;
    // 검사 1·2: 예외의 종류와 matching placement delete에 의한 반환을 따로 확인한다.
    c.Throws<std::runtime_error>([&] { (void)new(pool) Widget(pool, true); }, "placement ctor failure");
    c.Expect(pool.allocated == 1 && pool.freed == 1, "matching placement delete");
    {
        // 정상 객체에는 사용자 정의 deleter를 가진 unique_ptr를 연결한다.
        // Pool과 deleter의 Pool*가 같은 대상임은 이 구성 코드가 유지하는 전제다.
        std::unique_ptr<Widget, ReturnToPool> owner(new(pool) Widget(pool, false), ReturnToPool{&pool});
        c.Expect(pool.allocated == 2 && pool.freed == 1, "live custom allocation");
    }
    c.Expect(pool.allocated == 2 && pool.freed == 2, "custom deleter on success");

    // 이번에는 이미 있는 자동 저장소에 int를 생성한다. 전역 비할당 placement new다.
    // int의 크기와 정렬을 모두 갖춘 저장소를 제공하며 새 힙 할당을 요청하지 않는다.
    alignas(int) std::array<std::byte, sizeof(int)> bytes{};
    int* value = ::new(static_cast<void*>(bytes.data())) int(42);
    c.Expect(*value == 42, "nonallocating placement new");
    std::destroy_at(value);
    // value를 delete하면 안 된다. bytes가 이 자동 저장소를 제공하며 블록 끝에 정리된다.
    // int의 수명을 끝내는 것과 bytes의 저장소를 해제하는 것은 같은 작업이 아니다.

    // arena → resource → vector 순으로 선언·구성하여 역순 정리가 안전하게 이뤄진다.
    // 고정 버퍼를 기본 정렬로 준비하고 상위 자원은 null_memory_resource로 제한한다.
    // vector<int>의 요청을 관찰하는 예이며 임의의 확장 정렬을 시험하지 않는다.
    alignas(std::max_align_t) std::array<std::byte, 1024> arena{};
    std::pmr::monotonic_buffer_resource resource(arena.data(), arena.size(), std::pmr::null_memory_resource());
    {
        std::pmr::vector<int> values(&resource);
        // reserve(8)은 원소 여덟 개를 담을 용량을 요청한다. 버퍼의 정확한 소비 바이트나
        // 라이브러리 내부 성장 정책을 이 소스에서 측정하지는 않는다.
        values.reserve(8);
        values.push_back(7);
        c.Expect(values[0] == 7, "explicit pmr resource");
    }
    // monotonic 자원은 개별 deallocate로 사용 구간을 재사용하지 않는다.
    // 4096바이트는 처음부터 1024바이트 arena보다 크다. 따라서 이 실패는
    // vector가 얼마나 소비했는지와 무관한 고정 요청이며 실제 시스템 OOM이 아니다.
    c.Throws<std::bad_alloc>([&] { (void)resource.allocate(4096); }, "bounded deterministic failure");
}); }
