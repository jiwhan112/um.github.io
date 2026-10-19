#include "com_owner.h"
#include <cassert>
#include <iostream>
#include <type_traits>
#include <utility>

struct Counts {
    int created = 0;
    int destroyed = 0;
    int addrefs = 0;
    int releases = 0;
};

struct FakeCom {
    explicit FakeCom(Counts& c) : stats(c), refs(1) { ++stats.created; }
    ~FakeCom() { assert(refs == 0); ++stats.destroyed; }
    unsigned long AddRef() { ++stats.addrefs; return ++refs; }
    unsigned long Release() {
        ++stats.releases;
        assert(refs > 0);
        const auto remaining = --refs;
        if (remaining == 0) delete this;
        return remaining;
    }
    Counts& stats;
    unsigned long refs;
};

int main() {
    static_assert(!std::is_copy_constructible<ComOwner<FakeCom>>::value, "COM owner must not copy");
    static_assert(std::is_move_constructible<ComOwner<FakeCom>>::value, "COM owner must move");
    Counts count;
    {
        ComOwner<FakeCom> a(new FakeCom(count));
        assert(a && a->refs == 1);
        ComOwner<FakeCom> b(std::move(a));
        assert(!a && b && b->refs == 1);
        b.reset();
        assert(!b && count.destroyed == 1);
    }
    {
        ComOwner<FakeCom> first;
        *first.put() = new FakeCom(count);  // Emulates a COM API out parameter.
        ComOwner<FakeCom> other(new FakeCom(count));
        first = std::move(other);
        assert(!other && first && count.destroyed == 2);
    }
    {
        ComOwner<FakeCom> first(new FakeCom(count));
        FakeCom* shared = first.get();
        shared->AddRef(); // A second owner requires a new reference.
        ComOwner<FakeCom> second(shared);
        first.reset();
        assert(second && second->refs == 1);
        second.reset();
        assert(count.destroyed == 4);
    }
    assert(count.created == 4);
    assert(count.destroyed == 4);
    assert(count.addrefs == 1);
    assert(count.releases == 5);
    std::cout << "COM lifetime tests passed: objects=" << count.created
              << ", AddRef=" << count.addrefs << ", Release=" << count.releases << '\n';
}
