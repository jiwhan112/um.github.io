#include <string>
#include <string_view>
struct Label {
    std::string data;
    std::string_view View() const & { return data; }
    std::string_view View() && = delete;
};
int main() { auto view = Label{"temporary"}.View(); return static_cast<int>(view.size()); }
