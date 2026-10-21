#include "check.hpp"
#include "inventory.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <system_error>
namespace fs = std::filesystem;
class TempDirectory {
public:
    TempDirectory() {
        const auto token = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int i = 0; i < 100; ++i) {
            auto candidate = fs::temp_directory_path() / ("cpp_basics_" + std::to_string(token) + "_" + std::to_string(i));
            if (fs::create_directory(candidate)) { path = std::move(candidate); return; }
        }
        throw std::runtime_error("cannot create temporary directory");
    }
    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;
    ~TempDirectory() { std::error_code error; fs::remove_all(path, error); }
    fs::path path;
};
bool Rejects(const std::string& text) {
    try { std::istringstream input{text}; (void)study::Load(input); }
    catch (const std::runtime_error&) { return true; }
    return false;
}
int main() {
    try {
        study::Inventory items{{1,"Health Potion",3},{2,"Mana",1}};
        std::ostringstream text; study::Save(text, items);
        std::istringstream input{text.str()}; const auto loaded = study::Load(input);
        CHECK(loaded.size() == 2); CHECK(loaded.at(0).name == "Health Potion"); CHECK(loaded.at(0).quantity == 3);
        CHECK(Rejects("")); CHECK(Rejects("INV2 0\n")); CHECK(Rejects("INV1 -1\n"));
        CHECK(Rejects("INV1 1001\n")); CHECK(Rejects("INV1 2\n1 \"A\" 1\n"));
        CHECK(Rejects("INV1 1\n1 \"A\" -1\n")); CHECK(Rejects("INV1 2\n1 \"A\" 1\n1 \"B\" 2\n"));
        CHECK(Rejects("INV1 1\n1 \"A\" 1 junk\n")); CHECK(Rejects("INV1 0\nextra\n"));
        CHECK(Rejects("INV1 1\n1 \"\" 0\n"));
        std::istringstream empty{"INV1 0\n"}; CHECK(study::Load(empty).empty());
        bool rejected = false;
        try { std::ostringstream out; study::Save(out, {{1,"A",-1}}); }
        catch (const std::runtime_error&) { rejected = true; }
        CHECK(rejected);
        TempDirectory temp; const auto file = temp.path / "inventory.txt";
        { std::ofstream out{file, std::ios::binary}; if(!out) throw std::runtime_error("open failed");
          study::Save(out, items); out.close(); if(!out) throw std::runtime_error("close failed"); }
        { std::ifstream in{file, std::ios::binary}; if(!in) throw std::runtime_error("open failed");
          CHECK(study::Load(in).size() == 2); }
        const auto copied = temp.path / "copy.bin";
        CHECK(fs::copy_file(file, copied)); CHECK(fs::file_size(copied) == fs::file_size(file));
        { std::ifstream missing{temp.path / "missing.txt"}; CHECK(!missing); }
        std::cout << "cwd=" << fs::current_path().string() << '\n';
        Report();
    } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
