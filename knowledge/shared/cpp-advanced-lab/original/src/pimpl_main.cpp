#include "check.hpp"
#include "pimpl.hpp"
#include <type_traits>
static_assert(std::is_nothrow_move_constructible_v<Settings>);
int main() { return Run("pimpl", [](Checks& c) {
    Settings a("old"); Settings b = a; b.Rename("new");
    c.Expect(a.Name() == "old" && b.Name() == "new", "deep pimpl copy");
    Settings moved = std::move(a); c.Expect(a.Empty() && moved.Name() == "old", "move contract");
    c.Throws<std::logic_error>([&] { (void)a.Name(); }, "empty contract");
    a.Rename("restored"); c.Expect(a.Name() == "restored", "restore moved-from");
    using std::swap; swap(a,b); c.Expect(a.Name() == "new" && b.Name() == "restored", "ADL swap");
    b = moved; c.Expect(b.Name() == "old", "assignment");
}); }
