# C++ 기초문법 실습 코드 v1.0

개정일 2026-10-12. 아래 파일 경로대로 나누면 CMake 프로젝트가 됩니다. 바로 사용할 수 있는 ZIP은 이 대화에 별도로 제공했습니다. ZIP의 노션 직접 업로드는 실행 환경의 네트워크 제한으로 실패하여, 노션에는 이 텍스트 코드 원본을 첨부합니다.

C++20 기준입니다. Linux x86-64 / GCC 14.2.0 / Clang 17.0.0 / CMake 3.31.6에서 정상 실행 6개가 모두 통과했습니다. Clang의 ASan+UBSan 빌드도 6개 모두 통과했습니다. 정상 빌드 경고 없음. Windows/MSVC 실행은 미검증입니다. 조건 검사 수는 types15, flow15, memory19, containers49, buffer104, inventory19(합221)이며 루프 각 원소 검사도 포함합니다.

## 빌드

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(cpp_basics_lab VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()
foreach(name IN ITEMS types flow memory containers buffer)
    add_executable(${name} src/${name}.cpp)
    add_test(NAME ${name} COMMAND ${name})
endforeach()
add_executable(inventory src/inventory.cpp src/inventory_main.cpp)
add_test(NAME inventory COMMAND inventory)
foreach(name IN ITEMS types flow memory containers buffer inventory)
    if(MSVC)
        target_compile_options(${name} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    endif()
endforeach()
```

## src/check.hpp

```cpp
#ifndef CPP_BASICS_CHECK_HPP
#define CPP_BASICS_CHECK_HPP
#include <cstdlib>
#include <iostream>
inline int checks = 0;
inline void Check(bool condition, const char* expression, int line) {
    ++checks;
    if (!condition) { std::cerr << "FAIL line " << line << ": " << expression << '\n'; std::exit(1); }
}
#define CHECK(expression) Check(static_cast<bool>(expression), #expression, __LINE__)
inline void Report() { std::cout << "PASS " << checks << " checks\n"; }
#endif
```

## src/types.cpp

```cpp
#include "check.hpp"
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
int main() {
    static_assert(sizeof(char) == 1);
    static_assert(CHAR_BIT >= 8);
    static_assert(std::is_same_v<decltype(10.0), double>);
    static_assert(std::is_same_v<decltype(10.0f), float>);
    int zero{};
    int hp{100}; hp = 80;
    CHECK(zero == 0); CHECK(hp == 80);
    CHECK(0b00000010 == 2); CHECK((0b0100 ^ 0b0011) == 7); CHECK(0132 == 90);
    CHECK(7 / 2 == 3); CHECK(7.0 / 2 == 3.5);
    int i{5}; const int old = i++; const int now = ++i;
    CHECK(old == 5); CHECK(now == 7); CHECK(i == 7);
    unsigned int n = std::numeric_limits<unsigned int>::max();
    CHECK(n + 1u == 0u); // Unsigned arithmetic, not signed overflow.
    std::uint32_t flags{0}; constexpr std::uint32_t burning = std::uint32_t{1} << 2;
    flags |= burning; CHECK((flags & burning) != 0);
    flags &= ~burning; CHECK((flags & burning) == 0);
    bool evaluated = false;
    const bool result = false && (evaluated = true);
    CHECK(!result && !evaluated);
    CHECK(std::abs((0.1 + 0.2) - 0.3) < 1e-12);
    std::cout << "CHAR_BIT=" << CHAR_BIT << " sizeof(int)=" << sizeof(int)
              << " sizeof(long)=" << sizeof(long) << " sizeof(void*)=" << sizeof(void*)
              << " sizeof(wchar_t)=" << sizeof(wchar_t) << '\n';
    Report();
}
```

## src/flow.cpp

```cpp
#include "check.hpp"
#include <cstdint>
#include <limits>
#include <optional>
#include <sstream>
std::optional<std::uint64_t> Factorial(int n) {
    if (n < 0 || n > 20) return std::nullopt;
    if (n == 0) return std::uint64_t{1};
    const auto previous = Factorial(n - 1);
    const auto factor = static_cast<std::uint64_t>(n);
    if (!previous || *previous > std::numeric_limits<std::uint64_t>::max() / factor)
        return std::nullopt;
    return *previous * factor;
}
int Add(int a, int b) { return a + b; } // Precondition: sum representable in int.
int Scale(int value, int factor = 2) { return value * factor; }
using Operation = int (*)(int, int);
int main() {
    CHECK(Factorial(0) == 1); CHECK(Factorial(5) == 120);
    CHECK(Factorial(20) == 2432902008176640000ULL);
    CHECK(!Factorial(-1)); CHECK(!Factorial(21)); CHECK(!Factorial(1000000));
    int count{};
    for (int i = 0; i < 5; ++i) { if (i == 2) continue; ++count; }
    CHECK(count == 4);
    enum class State { Idle, Running }; const State state = State::Running;
    switch (state) { case State::Idle: count = 0; break; case State::Running: count = 7; break; }
    CHECK(count == 7);
    int once{}; do { ++once; } while (false); CHECK(once == 1);
    Operation operation = &Add; CHECK(operation(3, 4) == 7);
    operation = [](int a, int b) { return a - b; }; CHECK(operation(7, 3) == 4);
    CHECK(Scale(3) == 6); CHECK(Scale(3, 4) == 12);
    std::istringstream good{"42"}, bad{"oops"}; int value{};
    CHECK(static_cast<bool>(good >> value) && value == 42);
    CHECK(!(bad >> value));
    Report();
}
```

## src/memory.cpp

```cpp
#include "check.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <memory>
#include <utility>
void SetValue(int value) { value = 9; (void)value; }
void SetReference(int& value) { value = 9; }
void ReseatCopy(int* p, int* target) { p = target; (void)p; }
void Reseat(int*& p, int* target) { p = target; }
void ReseatOut(int** p, int* target) { if (p) *p = target; }
int main() {
    int hp{100}; int& alias = hp; int* p = &hp;
    CHECK(p == &hp); CHECK(*p == 100);
    alias = 80; CHECK(hp == 80);
    SetValue(hp); CHECK(hp == 80);
    SetReference(hp); CHECK(hp == 9);
    int other{7}; ReseatCopy(p, &other); CHECK(p == &hp);
    Reseat(p, &other); CHECK(p == &other);
    ReseatOut(&p, &hp); CHECK(p == &hp);
    const int* read_only = &hp; CHECK(*read_only == 9);
    int* const fixed = &hp; *fixed = 6; CHECK(hp == 6);
    void* erased = &hp; CHECK(*static_cast<int*>(erased) == 6);
    int values[]{10, 20, 30}; int* end = values + 3;
    CHECK(*(values + 1) == values[1]); CHECK(end - values == 3);
    // Never dereference end: it is a one-past pointer.
    auto owner = std::make_unique<int>(42);
    int* observer = owner.get(); CHECK(*observer == 42);
    auto new_owner = std::move(owner);
    CHECK(!owner); CHECK(new_owner.get() == observer);
    new_owner.reset(); observer = nullptr; CHECK(observer == nullptr);
    // Explicit byte decoding avoids aliasing and host-endianness assumptions.
    std::array<std::uint8_t, 2> bytes{0x01, 0x02};
    const auto decoded = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[0]) |
        (static_cast<std::uint16_t>(bytes[1]) << 8));
    CHECK(decoded == 513);
    const std::uint32_t source{0x12345678u};
    const auto representation = std::bit_cast<std::array<unsigned char, sizeof(source)>>(source);
    CHECK(std::bit_cast<std::uint32_t>(representation) == source);
    Report();
}
```

## src/containers.cpp

```cpp
#include "check.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
int Sum(std::span<const int> values) { int result{}; for (int value : values) result += value; return result; }
struct Item { int id; std::string name; int quantity; };
int main() {
    std::array<int, 3> a{10, 20, 30}; CHECK(Sum(a) == 60);
    std::vector<int> v; v.reserve(4);
    CHECK(v.empty()); CHECK(v.capacity() >= 4);
    v.resize(3); CHECK(v.size() == 3); CHECK(v[0] == 0);
    v[1] = 8; CHECK(Sum(v) == 8);
    bool rejected = false; try { (void)v.at(3); } catch (const std::out_of_range&) { rejected = true; }
    CHECK(rejected);
    const auto old_capacity = v.capacity();
    v.reserve(old_capacity + 1); // Do not retain or inspect any old element pointers.
    CHECK(v.capacity() > old_capacity); CHECK(v[1] == 8);
    int grid[2][3]{{1,2,3},{4,5,6}}; int (*row)[3] = grid;
    CHECK(row[1][2] == 6);
    std::vector<int> flat{1,2,3,4,5,6}; CHECK(flat.at(1 * 3 + 2) == 6);
    std::string embedded{"a\0b", 3};
    CHECK(embedded.size() == 3); CHECK(std::strlen(embedded.c_str()) == 1);
    std::string name{"Potion"}; const std::string_view view{name}; CHECK(view == "Potion");
    CHECK(std::strcmp("a", "b") < 0); CHECK(std::strcmp("b", "b") == 0);
    std::vector<Item> items{{2,"Mana",1},{1,"Health",3},{3,"Shield",1}};
    std::sort(items.begin(), items.end(), [](const Item& x, const Item& y) {
        return x.quantity != y.quantity ? x.quantity > y.quantity : x.id < y.id;
    });
    CHECK(items[0].id == 1); CHECK(items[1].id == 2); CHECK(items[2].id == 3);
    std::variant<int, std::string> data{7}; CHECK(std::get<int>(data) == 7);
    data = std::string{"text"}; CHECK(std::holds_alternative<std::string>(data));
    std::mt19937 engine_a{1234u}, engine_b{1234u};
    for (int i = 0; i < 8; ++i) CHECK(engine_a() == engine_b());
    std::uniform_int_distribution<int> die{1, 6};
    for (int i = 0; i < 20; ++i) { int roll = die(engine_a); CHECK(roll >= 1 && roll <= 6); }
    std::cout << "sizeof(Item)=" << sizeof(Item) << " alignof(Item)=" << alignof(Item) << '\n';
    Report();
}
```

## src/buffer.cpp

```cpp
#include "check.hpp"
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
// Educational int-only buffer. Not a replacement for std::vector.
class IntBuffer {
public:
    IntBuffer() = default;
    IntBuffer(const IntBuffer&) = delete;
    IntBuffer& operator=(const IntBuffer&) = delete;
    IntBuffer(IntBuffer&&) = delete;
    IntBuffer& operator=(IntBuffer&&) = delete;
    void PushBack(int value) {
        if (size_ == capacity_) {
            constexpr auto limit = std::numeric_limits<std::size_t>::max() / sizeof(int);
            if (capacity_ > limit / 2) throw std::length_error("capacity overflow");
            const auto next_capacity = capacity_ == 0 ? std::size_t{2} : capacity_ * 2;
            auto next = std::make_unique<int[]>(next_capacity);
            if (size_ != 0) std::copy_n(data_.get(), size_, next.get());
            data_ = std::move(next); // Old allocation released only after successful allocation/copy.
            capacity_ = next_capacity;
        }
        data_[size_++] = value;
    }
    int At(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("index");
        return data_[index];
    }
    std::size_t Size() const { return size_; }
    std::size_t Capacity() const { return capacity_; }
private:
    std::unique_ptr<int[]> data_;
    std::size_t size_{};
    std::size_t capacity_{};
};
int main() {
    IntBuffer buffer; CHECK(buffer.Size() == 0);
    bool rejected = false; try { (void)buffer.At(0); } catch(const std::out_of_range&) { rejected = true; }
    CHECK(rejected);
    for (int i = 0; i < 100; ++i) buffer.PushBack(i);
    CHECK(buffer.Size() == 100); CHECK(buffer.Capacity() == 128); // This policy only.
    for (std::size_t i = 0; i < 100; ++i) CHECK(buffer.At(i) == static_cast<int>(i));
    Report();
}
```

## src/inventory.hpp

```cpp
#ifndef CPP_BASICS_INVENTORY_HPP
#define CPP_BASICS_INVENTORY_HPP
#include <istream>
#include <ostream>
#include <string>
#include <vector>
namespace study {
struct Item { int id{}; std::string name; int quantity{}; };
using Inventory = std::vector<Item>;
// These functions throw std::runtime_error for invalid data or stream failures.
void Save(std::ostream& output, const Inventory& items);
Inventory Load(std::istream& input);
}
#endif
```

## src/inventory.cpp

```cpp
#include "inventory.hpp"
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
namespace study {
namespace {
void Validate(const Inventory& items) {
    if (items.size() > 1000) throw std::runtime_error("too many items");
    std::set<int> ids;
    for (const auto& item : items) {
        if (item.id <= 0 || !ids.insert(item.id).second) throw std::runtime_error("invalid/duplicate id");
        if (item.quantity < 0 || item.quantity > 1000000) throw std::runtime_error("quantity range");
        if (item.name.empty() || item.name.size() > 256 || item.name.find_first_of("\r\n") != std::string::npos)
            throw std::runtime_error("invalid name");
    }
}
void RequireEnd(std::istream& line) {
    line >> std::ws;
    if (!line.eof()) throw std::runtime_error("trailing data");
}
}
void Save(std::ostream& output, const Inventory& items) {
    Validate(items); // Check all values before writing.
    output << "INV1 " << items.size() << '\n';
    for (const auto& item : items)
        output << item.id << ' ' << std::quoted(item.name) << ' ' << item.quantity << '\n';
    if (!output) throw std::runtime_error("write failed");
}
Inventory Load(std::istream& input) {
    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("missing header");
    std::istringstream header{line}; std::string magic; long long count{};
    if (!(header >> magic >> count) || magic != "INV1" || count < 0 || count > 1000)
        throw std::runtime_error("bad header");
    RequireEnd(header);
    Inventory items; items.reserve(static_cast<std::size_t>(count));
    for (long long i = 0; i < count; ++i) {
        if (!std::getline(input, line)) throw std::runtime_error("truncated file");
        std::istringstream row{line}; Item item;
        if (!(row >> item.id >> std::quoted(item.name) >> item.quantity)) throw std::runtime_error("bad row");
        RequireEnd(row);
        items.push_back(std::move(item));
    }
    while (std::getline(input, line)) {
        if (line.find_first_not_of(" \t\r") != std::string::npos) throw std::runtime_error("extra row");
    }
    if (input.bad() || (input.fail() && !input.eof())) throw std::runtime_error("read failed");
    Validate(items); // Return only a complete valid result.
    return items;
}
}
```

## src/inventory_main.cpp

```cpp
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
```

## 컴파일 거부 실험 (기본 빌드 제외)

GCC/Clang의 `-std=c++20 -pedantic-errors -c`에서 4개 모두 거부되었습니다.

### negative/const_write.cpp
```cpp
int main() { const int hp{100}; hp = 80; }
```
### negative/narrowing.cpp
```cpp
int main() { int count{3.5}; return count; }
```
### negative/array_pointer.cpp
```cpp
int main() { int grid[2][3]{}; int** wrong = grid; (void)wrong; }
```
### negative/reference_temporary.cpp
```cpp
int main() { int& wrong = 10; (void)wrong; }
```

## 위험 동작 진단 실험 (기본 빌드 제외)

정상 출력값을 기대하거나 게임 코드에 적용하지 마세요. 아래 두 파일은 의도적인 미정의 동작입니다.

### diagnostics/signed_overflow.cpp
```cpp
// Intentional undefined behavior. Compile with UBSan; never use as normal code.
#include <limits>
int main() { volatile int n = std::numeric_limits<int>::max(); return n + 1; }
```

### diagnostics/use_after_free.cpp
```cpp
// Intentional undefined behavior. Compile with ASan; never use as normal code.
int main() { int* owner = new int{7}; int* observer = owner; delete owner; return *observer; }
```

```sh
clang++ -std=c++20 -g -O0 -fsanitize=undefined -fno-sanitize-recover=all diagnostics/signed_overflow.cpp -o overflow_demo
./overflow_demo
# 이번 관찰: signed integer overflow 진단, 비정상 종료

clang++ -std=c++20 -g -O0 -fsanitize=address -fno-sanitize-recover=all -fno-omit-frame-pointer diagnostics/use_after_free.cpp -o lifetime_demo
./lifetime_demo
# 이번 관찰: heap-use-after-free 진단, 비정상 종료
```

## 적용 한계

Add/Scale/Sum은 작은 정상 입력에 대한 시연이며 임의 입력에 대한 범용 안전 산술이 아닙니다. IntBuffer는 int만 지원하고 복사·이동을 금지했습니다. 성장률 2배는 이 예제의 정책입니다. Inventory는 ASCII 이름으로 시험했고 UTF-8 검증·읽기 전 총 바이트 제한·원자적 저장·전원 장애 복구는 구현하지 않았습니다. MSVC·GPU·DirectX 게임 통합은 실행 미검증입니다. 원문 코드 이미지 전체를 전사한 것이 아니라 기존 주제를 바탕으로 새로 작성한 실습입니다.

## 공식 검증 참고

- C++20 공개 초안: https://timsong-cpp.github.io/cppwp/n4861/
- Microsoft 표준 선택: https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170
- Clang ASan: https://clang.llvm.org/docs/AddressSanitizer.html
- Clang UBSan: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
