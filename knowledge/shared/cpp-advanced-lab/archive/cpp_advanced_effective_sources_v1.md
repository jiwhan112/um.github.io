# C++ 심화 · Effective C++ 실습 소스 v1.0

각 파일 경로대로 저장한 뒤 README 빌드 명령을 실행합니다.

## `CMakeLists.txt`

````cmake
cmake_minimum_required(VERSION 3.20)
project(programmer_rewrite_batch2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
enable_testing()
function(study_target target)
  add_executable(${target} ${ARGN})
  target_include_directories(${target} PRIVATE src)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive- /EHsc /utf-8)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
    if(ENABLE_SANITIZERS)
      target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
      target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
  endif()
  add_test(NAME ${target} COMMAND ${target})
endfunction()
foreach(name lifetime ownership polymorphism templates containers interfaces transaction allocation)
  study_target(${name} src/${name}.cpp)
endforeach()
study_target(pimpl src/pimpl.cpp src/pimpl_main.cpp)

````

## `README.md`

````markdown
# C++ 심화 · Effective C++ 학습 실습 v1.0

개정일: 2026-10-13. 독립적으로 새로 작성한 C++20 학습 예제입니다.
회사 코드나 원문 책의 예제 구현을 복제한 저장소가 아닙니다.

## 빌드

CMake 3.20 이상, C++20을 지원하는 컴파일러가 필요합니다.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Visual Studio 생성기:
```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```
Windows/MSVC 명령은 제공했지만 이 환경에서 실행 검증하지 않았습니다.

GCC/Clang을 모두 설치한 환경에서 전체 재현:
```sh
python3 run_checks.py
```

이 스크립트는 build-gcc, build-clang, build-clang-sanitized와 logs 폴더에만
빌드 결과를 기록합니다. 사용자 프로젝트나 시스템 설정을 변경하지 않습니다.
메모리 고갈, 전역 할당 함수 교체, 실제 GPU 호출을 하지 않습니다.

## 실행 파일

| 파일 | 주제 |
|---|---|
| lifetime | 초기화·파괴 순서, 부분 생성 실패, 체력 불변식 |
| ownership | Rule of Zero/Five, 깊은 복사, 실패 주입, 이동, 자기 대입 |
| polymorphism | 가상 호출, 생성·소멸 단계, 기본 인수, cast, NVI, clone |
| templates | concept, typename, 의존 기반 클래스, forwarding, capture |
| containers | 수정·조회·반복자 무효화·정렬 비교자 |
| interfaces | explicit, const iterator, 임시 객체에서의 뷰 반환 금지 |
| transaction | 준비·검증·교체, 실패 후 상태 보존, weak_ptr |
| pimpl | 실제 h/cpp 분리, incomplete type, 깊은 복사·이동 |
| allocation | 위치 지정 생성 실패, custom deleter, 한정 pmr 자원 |

negative 폴더 6개 파일은 **컴파일에 실패해야 정상**인 학습용입니다.
기본 CMake 빌드에 포함하지 않습니다. run_checks.py는 `-c`로 거부를 확인합니다.
이 파일들은 경고를 오류로 만든 것 때문에 실패하는 것이 아니라 언어/라이브러리
요구사항 위반을 보여 줍니다. 정확한 진단 위치는 logs에서 확인하세요.

## 범위와 주의

- `Counter`, `Quantity`, `AddLater`, `PowerOfTwo` 등은 테스트가 사용하는 작은 정수와
  이 실습의 GCC/Clang/MSVC형 환경을 전제로 합니다. 범용 안전 수치 라이브러리가 아닙니다.
- IntBuffer는 int 전용 학습 타입입니다. 표준 vector 대체용이 아닙니다.
- GameState의 강한 예외 보장은 자신이 소유한 동기 상태에 관한 것입니다.
  파일/네트워크 롤백, 동시성 원자성, 저장 내구성을 제공하지 않습니다.
- Pool은 단일 Widget 교육 예입니다. 범용 정렬 지원·다중 스레드 할당자·전역 new 구현이 아닙니다.
- sanitizer 무진단은 실행하지 않은 모든 경로의 안전성을 증명하지 않습니다.
- C++11 회사 코드에 그대로 복사하지 마세요. make_unique는 C++14,
  pmr·string_view는 C++17, concepts·ranges·erase_if는 C++20 기능을 사용합니다.

## 검증

Linux x86-64에서 GCC 14.2.0, Clang 17.0.0, Clang 17 ASan/UBSan의
각 정상 빌드 9/9, 조건 검사 89회 통과. 두 컴파일러가 각각 부정 예제 6개를 거부.
세 빌드의 반복 검사는 독립된 조건 267개를 뜻하지 않습니다.
실제 실행 로그 및 시스템 정보: `logs/report.json`.

연결 문서:
- https://app.notion.com/p/aaa55bced8ff459ba67fb036fb34bb3b
- https://app.notion.com/p/8178c17f593b4f4e83a0b42544b8fe2b

````

## `run_checks.py`

````python
"""Build and test the portable C++20 examples; expected failures use -c (no link)."""
from pathlib import Path
import subprocess, json, re, platform, sys
root=Path(__file__).resolve().parent
logs=root/'logs';logs.mkdir(exist_ok=True)
def execute(args, path):
    result=subprocess.run(args, cwd=root, capture_output=True, text=True, timeout=120)
    path.write_text('$ '+ ' '.join(map(str,args))+'\n'+result.stdout+result.stderr,encoding='utf-8')
    if result.returncode: print(path.name, 'returned', result.returncode)
    return result
report={'system':platform.platform(),'learning_standard':'C++20','builds':{},'negative':{},'limitations':['Windows/MSVC not executed','No DirectX/GPU integration test','Representative tests, not complete correctness proof']}
for tag, compiler, sanitizer in [('gcc','g++',False),('clang','clang++',False),('clang-sanitized','clang++',True)]:
    build=root/f'build-{tag}'
    report['builds'][tag]={'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0]}
    args=['cmake','-S',str(root),'-B',str(build),f'-DCMAKE_CXX_COMPILER={compiler}','-DCMAKE_BUILD_TYPE=Debug']
    if sanitizer: args+=['-DENABLE_SANITIZERS=ON']
    for step, cmd in [('configure',args),('build',['cmake','--build',str(build),'-j4']),('tests',['ctest','--test-dir',str(build),'-V'])]:
        result=execute(cmd,logs/f'{tag}-{step}.txt')
        report['builds'][tag][step]=result.returncode
        if result.returncode: break
        if step=='tests':
            entries=re.findall(r'PASS (\w+): (\d+) checks',result.stdout)
            report['builds'][tag]['checks']={n:int(c) for n,c in entries}
            report['builds'][tag]['total_checks']=sum(int(c) for n,c in entries)
for compiler,tag in [('g++','gcc'),('clang++','clang')]:
    report['negative'][tag]={}
    for file in sorted((root/'negative').glob('*.cpp')):
        result=execute([compiler,'-std=c++20','-Wall','-Wextra','-pedantic-errors','-c',str(file),'-o',str(logs/f'{tag}-{file.stem}.o')],logs/f'{tag}-negative-{file.stem}.txt')
        report['negative'][tag][file.stem]={'rejected': result.returncode!=0,'exit_code':result.returncode}
(logs/'report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False,indent=2))
valid=all(v.get('tests')==0 for v in report['builds'].values()) and all(i['rejected'] for group in report['negative'].values() for i in group.values())
sys.exit(0 if valid else 1)

````

## `src/allocation.cpp`

````cpp
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

````

## `src/check.hpp`

````cpp
#ifndef STUDY_CHECK_HPP
#define STUDY_CHECK_HPP
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>
class Checks {
    int count_{};
public:
    void Expect(bool condition, const char* label) {
        ++count_;
        if (!condition) throw std::runtime_error(label);
    }
    template<class E, class F> void Throws(F&& f, const char* label) {
        bool matched = false;
        try { std::invoke(std::forward<F>(f)); }
        catch (const E&) { matched = true; }
        Expect(matched, label);
    }
    int Count() const noexcept { return count_; }
};
template<class F> int Run(const char* name, F&& test) {
    try {
        Checks checks;
        std::invoke(std::forward<F>(test), checks);
        std::cout << "PASS " << name << ": " << checks.Count() << " checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL " << name << ": " << e.what() << '\n';
        return 1;
    }
}
#endif

````

## `src/containers.cpp`

````cpp
#include "check.hpp"
#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
struct Item { int priority; int id; };
struct Less {
    bool operator()(const Item& a, const Item& b) const noexcept {
        return std::tie(a.priority, a.id) < std::tie(b.priority, b.id);
    }
};
int main() { return Run("containers", [](Checks& c) {
    std::vector<int> v{1,2,3,4}; const auto capacity = v.capacity();
    std::erase_if(v, [](int n) { return n % 2 == 0; });
    c.Expect(v == std::vector<int>({1,3}), "erase_if");
    c.Expect(v.capacity() == capacity, "erase retains capacity");
    v.insert(v.begin()+1, 2); c.Expect(v == std::vector<int>({1,2,3}), "middle insert");
    for (auto it = v.begin(); it != v.end();) { if (*it >= 2) it = v.erase(it); else ++it; }
    c.Expect(v == std::vector<int>({1}), "erase return iterator");
    std::list<int> nodes{3,1,2}; auto keep = std::next(nodes.begin());
    nodes.push_front(9); c.Expect(*keep == 1, "list iterator stable on insert");
    nodes.sort(); c.Expect(*keep == 1 && nodes.front() == 1, "list member sort");
    std::deque<int> queue; queue.push_front(2); queue.push_back(3); queue.push_front(1);
    c.Expect(queue[1] == 2, "deque random access");
    std::map<std::string, int, std::less<>> counts{{"potion",3}};
    c.Expect(counts.find("absent") == counts.end() && counts.size() == 1, "find no insert");
    c.Expect(counts["absent"] == 0 && counts.size() == 2, "brackets insert");
    const char a[] = "potion"; const char b[] = "potion";
    c.Expect(counts.at(a) == counts.at(b), "string content keys");
    std::unordered_map<int, std::string> lookup{{1,"one"}};
    const std::string* value = &lookup.at(1); lookup.rehash(100);
    c.Expect(value == &lookup.at(1) && *value == "one", "rehash preserves reference, not iterator");
    std::vector<Item> items{{2,3},{1,8},{2,1}}; std::ranges::sort(items, Less{});
    c.Expect(items[0].id == 8 && items[1].id == 1, "strict comparator");
    for (const auto& x : items) c.Expect(!Less{}(x,x), "irreflexivity sample");
    std::vector<std::unique_ptr<int>> owners; owners.push_back(std::make_unique<int>(7));
    owners.clear(); c.Expect(owners.empty(), "owning elements released");
}); }

````

## `src/interfaces.cpp`

````cpp
#include "check.hpp"
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
class Seconds {
    double value_;
public:
    explicit Seconds(double value) : value_(value) {
        if (!(value >= 0.0) || value > 3600.0) throw std::invalid_argument("seconds");
    }
    double Value() const noexcept { return value_; }
};
static_assert(!std::is_convertible_v<double, Seconds>);
class Label {
    std::string text_;
public:
    explicit Label(std::string text) : text_(std::move(text)) {}
    std::string_view View() const & noexcept { return text_; }
    std::string_view View() && = delete;
    std::string_view View() const && = delete;
    std::string Copy() const { return text_; }
};
// Small integers only: arithmetic-range validation is not this experiment's purpose.
class Counter {
    int value_{};
public:
    Counter(int value = 0) : value_(value) {}
    int Value() const noexcept { return value_; }
    Counter& operator+=(const Counter& rhs) { value_ += rhs.value_; return *this; }
    friend Counter operator+(Counter lhs, const Counter& rhs) { lhs += rhs; return lhs; }
};
template<class T> class Quantity {
    T value_;
public:
    Quantity(T value) : value_(value) {}
    T Value() const { return value_; }
    friend Quantity operator+(Quantity a, Quantity b) { return Quantity(a.value_ + b.value_); }
};
int main() { return Run("interfaces", [](Checks& c) {
    Seconds dt(0.016); c.Expect(dt.Value() == 0.016, "explicit valid duration");
    c.Throws<std::invalid_argument>([] { Seconds invalid(-1.0); }, "negative duration");
    std::vector<int> data{1,2}; const auto fixed = data.begin(); *fixed = 7;
    auto readonly = data.cbegin(); ++readonly;
    c.Expect(data.front() == 7 && *readonly == 2, "const iterator vs const_iterator");
    Label label("player"); c.Expect(label.View() == "player", "borrow lvalue");
    const auto copy = Label("temporary").Copy(); c.Expect(copy == "temporary", "owned result");
    Counter count(3); c.Expect((count+2).Value() == 5 && (2+count).Value() == 5, "symmetric conversion");
    Quantity<int> q(4); c.Expect((q+3).Value() == 7 && (3+q).Value() == 7, "hidden friend");
}); }

````

## `src/lifetime.cpp`

````cpp
#include "check.hpp"
#include <array>
#include <cstddef>
#include <initializer_list>
struct Trace {
    std::array<int, 32> values{};
    std::size_t used{};
    bool overflow{};
    void Add(int value) noexcept {
        if (used < values.size()) values[used++] = value;
        else overflow = true;
    }
    bool Equals(std::initializer_list<int> expected) const {
        if (overflow || used != expected.size()) return false;
        std::size_t i{};
        for (int value : expected) if (values[i++] != value) return false;
        return true;
    }
};
struct Part {
    Trace& trace;
    int id;
    Part(Trace& t, int value, bool fail = false) : trace(t), id(value) {
        trace.Add(id);
        if (fail) throw std::runtime_error("part construction failed");
    }
    ~Part() noexcept { trace.Add(-id); }
};
struct Base {
    Trace& trace;
    explicit Base(Trace& t) : trace(t) { trace.Add(1); }
    ~Base() noexcept { trace.Add(-1); }
};
struct Whole : Base {
    Part first;
    Part second;
    Whole(Trace& t, bool fail = false) : Base(t), first(t, 2), second(t, 3, fail) { trace.Add(4); }
    ~Whole() noexcept { trace.Add(-4); }
};
class Health {
    int maximum_;
    int current_;
public:
    explicit Health(int maximum) : maximum_(maximum), current_(maximum) {
        if (maximum <= 0) throw std::invalid_argument("maximum must be positive");
    }
    int Current() const noexcept { return current_; }
    void Damage(int amount) {
        if (amount < 0) throw std::invalid_argument("negative damage");
        current_ = amount >= current_ ? 0 : current_ - amount;
    }
    void Heal(int amount) {
        if (amount < 0) throw std::invalid_argument("negative heal");
        const int room = maximum_ - current_;
        current_ += amount >= room ? room : amount;
    }
};
int main() { return Run("lifetime", [](Checks& c) {
    Trace trace;
    { Whole whole(trace); c.Expect(trace.Equals({1,2,3,4}), "construction order"); }
    c.Expect(trace.Equals({1,2,3,4,-4,-3,-2,-1}), "destruction order");
    Trace failed;
    c.Throws<std::runtime_error>([&] { Whole whole(failed, true); }, "constructor failure");
    c.Expect(failed.Equals({1,2,3,-2,-1}), "only completed subobjects destroyed");
    Health hp(100);
    hp.Damage(30); c.Expect(hp.Current() == 70, "damage");
    hp.Heal(200); c.Expect(hp.Current() == 100, "heal clamp");
    hp.Damage(200); c.Expect(hp.Current() == 0, "damage clamp");
    c.Throws<std::invalid_argument>([&] { hp.Damage(-1); }, "negative rejected");
    c.Expect(hp.Current() == 0, "failure preserves state");
    c.Throws<std::invalid_argument>([] { Health invalid(0); }, "invalid construction");
}); }

````

## `src/ownership.cpp`

````cpp
#include "check.hpp"
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>
struct FailPoint { inline static bool next_copy = false; };
// Teaching invariant: size == 0 iff data is null. Prefer vector in production.
class IntBuffer {
    std::size_t size_{};
    std::unique_ptr<int[]> data_;
    static std::unique_ptr<int[]> Copy(const IntBuffer& source) {
        if (std::exchange(FailPoint::next_copy, false)) throw std::bad_alloc{};
        if (source.size_ == 0) return {};
        auto result = std::make_unique<int[]>(source.size_);
        std::copy_n(source.data_.get(), source.size_, result.get());
        return result;
    }
public:
    IntBuffer() = default;
    explicit IntBuffer(std::initializer_list<int> values)
        : size_(values.size()), data_(size_ ? std::make_unique<int[]>(size_) : nullptr) {
        if (size_ != 0) std::copy(values.begin(), values.end(), data_.get());
    }
    ~IntBuffer() = default;
    IntBuffer(const IntBuffer& other) : size_(other.size_), data_(Copy(other)) {}
    IntBuffer& operator=(const IntBuffer& other) {
        if (this != &other) { IntBuffer temporary(other); Swap(temporary); }
        return *this;
    }
    IntBuffer(IntBuffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)), data_(std::move(other.data_)) {}
    IntBuffer& operator=(IntBuffer&& other) noexcept {
        if (this != &other) { data_ = std::move(other.data_); size_ = std::exchange(other.size_, 0); }
        return *this;
    }
    void Swap(IntBuffer& other) noexcept { using std::swap; swap(size_, other.size_); data_.swap(other.data_); }
    friend void swap(IntBuffer& a, IntBuffer& b) noexcept { a.Swap(b); }
    std::size_t Size() const noexcept { return size_; }
    bool Valid() const noexcept { return (size_ == 0) == (data_ == nullptr); }
    int& At(std::size_t index) { if (index >= size_) throw std::out_of_range("index"); return data_[index]; }
    const int& At(std::size_t index) const { if (index >= size_) throw std::out_of_range("index"); return data_[index]; }
};
struct RuleZero { std::vector<int> values; };
struct CopyOnly {
    int* copies;
    explicit CopyOnly(int& value) : copies(&value) {}
    CopyOnly(const CopyOnly& other) : copies(other.copies) { ++*copies; }
};
struct Immovable {
    Immovable() = default;
    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
};
Immovable Make() { return Immovable{}; }
static_assert(std::is_nothrow_move_constructible_v<IntBuffer>);
static_assert(std::is_nothrow_move_assignable_v<IntBuffer>);
int main() { return Run("ownership", [](Checks& c) {
    RuleZero a{{1,2,3}}; auto b = a; b.values[0] = 9;
    c.Expect(a.values[0] == 1, "rule zero copies values");
    IntBuffer source({10,20,30}); IntBuffer copy(source); copy.At(0) = 77;
    c.Expect(source.At(0) == 10 && copy.At(0) == 77, "deep copy");
    IntBuffer target({5}); FailPoint::next_copy = true;
    c.Throws<std::bad_alloc>([&] { target = source; }, "injected copy failure");
    c.Expect(target.Size() == 1 && target.At(0) == 5 && target.Valid(), "strong guarantee");
    target = source; c.Expect(target.At(2) == 30, "copy assignment");
    IntBuffer& alias = target; target = alias; c.Expect(target.At(0) == 10, "self copy");
    IntBuffer moved(std::move(target));
    c.Expect(target.Size() == 0 && target.Valid(), "move source");
    c.Expect(moved.At(2) == 30 && moved.Valid(), "move destination");
    IntBuffer& moved_alias = moved; moved = std::move(moved_alias);
    c.Expect(moved.At(2) == 30 && moved.Valid(), "self move contract");
    IntBuffer empty; empty = std::move(moved);
    c.Expect(moved.Size() == 0 && empty.Size() == 3 && moved.Valid(), "move assignment");
    c.Throws<std::out_of_range>([&] { (void)moved.At(0); }, "moved source bounds");
    int copies{}; CopyOnly old(copies); CopyOnly copied(std::move(old)); (void)copied;
    c.Expect(copies == 1, "std::move can copy");
    auto elided = Make(); (void)elided;
    c.Expect(true, "prvalue construction despite deleted copy and move");
}); }

````

## `src/pimpl.cpp`

````cpp
#include "pimpl.hpp"
#include <stdexcept>
#include <utility>
struct Settings::Impl { std::string name; };
Settings::Settings(std::string name) : impl_(std::make_unique<Impl>(Impl{std::move(name)})) {}
Settings::~Settings() = default;
Settings::Settings(Settings&&) noexcept = default;
Settings& Settings::operator=(Settings&&) noexcept = default;
Settings::Settings(const Settings& other) : impl_(other.impl_ ? std::make_unique<Impl>(*other.impl_) : nullptr) {}
Settings& Settings::operator=(const Settings& other) {
    if (this != &other) { Settings copy(other); swap(*this, copy); }
    return *this;
}
void Settings::Rename(std::string name) {
    auto candidate = std::make_unique<Impl>(Impl{std::move(name)});
    impl_.swap(candidate);
}
std::string Settings::Name() const {
    if (!impl_) throw std::logic_error("empty moved-from settings");
    return impl_->name;
}
bool Settings::Empty() const noexcept { return !impl_; }
void swap(Settings& a, Settings& b) noexcept { a.impl_.swap(b.impl_); }

````

## `src/pimpl.hpp`

````cpp
#ifndef STUDY_PIMPL_HPP
#define STUDY_PIMPL_HPP
#include <memory>
#include <string>
class Settings {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    explicit Settings(std::string name);
    ~Settings();
    Settings(Settings&&) noexcept;
    Settings& operator=(Settings&&) noexcept;
    Settings(const Settings&);
    Settings& operator=(const Settings&);
    void Rename(std::string name);
    std::string Name() const;
    bool Empty() const noexcept;
    friend void swap(Settings&, Settings&) noexcept;
};
#endif

````

## `src/pimpl_main.cpp`

````cpp
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

````

## `src/polymorphism.cpp`

````cpp
#include "check.hpp"
#include <array>
#include <memory>
#include <string>
#include <typeinfo>
struct Events {
    std::array<int, 16> data{};
    std::size_t count{};
    void Add(int value) noexcept { if (count < data.size()) data[count++] = value; }
};
class Effect {
    Events& events_;
public:
    explicit Effect(Events& events) : events_(events) { events_.Add(Kind()); }
    virtual ~Effect() noexcept { events_.Add(Kind()); }
    virtual int Kind() const noexcept { return 1; }
    virtual int Strength(int scale = 2) const noexcept { return scale; }
    Effect(const Effect&) = delete;
    Effect& operator=(const Effect&) = delete;
};
class Fire final : public Effect {
    Events& events_;
public:
    explicit Fire(Events& events) : Effect(events), events_(events) { events_.Add(Kind()); }
    ~Fire() noexcept override { events_.Add(Kind()); }
    int Kind() const noexcept override { return 2; }
    int Strength(int scale = 3) const noexcept override { return scale * 10; }
};
class Ice final : public Effect { public: using Effect::Effect; };
struct ValueBase { int Kind() const { return 1; } };
struct ValueDerived : ValueBase { int Kind() const { return 2; } };
struct Overloads { int F(int) const { return 1; } };
struct AddOverload : Overloads { using Overloads::F; int F(const std::string&) const { return 2; } };
class Damage {
public:
    int Apply(int amount) const {
        if (amount < 0 || amount > 1000) throw std::invalid_argument("amount");
        return ApplyImpl(amount);
    }
    virtual ~Damage() = default;
private:
    virtual int ApplyImpl(int amount) const = 0;
};
class HalfDamage final : public Damage { int ApplyImpl(int amount) const override { return amount / 2; } };
struct Cloneable {
    virtual ~Cloneable() = default;
    virtual std::unique_ptr<Cloneable> Clone() const = 0;
    virtual int Value() const = 0;
};
struct Number final : Cloneable {
    int value;
    explicit Number(int n) : value(n) {}
    std::unique_ptr<Cloneable> Clone() const override { return std::make_unique<Number>(*this); }
    int Value() const override { return value; }
};
int main() { return Run("polymorphism", [](Checks& c) {
    Events events;
    {
        auto concrete = std::make_unique<Fire>(events);
        Effect& base = *concrete;
        c.Expect(events.count == 2 && events.data[0] == 1 && events.data[1] == 2, "construction dispatch");
        c.Expect(base.Kind() == 2, "dynamic dispatch");
        c.Expect(base.Strength() == 20 && concrete->Strength() == 30, "static defaults");
        c.Expect(dynamic_cast<Fire*>(&base) == concrete.get(), "pointer cast success");
        c.Expect(dynamic_cast<Ice*>(&base) == nullptr, "pointer cast failure");
        c.Throws<std::bad_cast>([&] { (void)dynamic_cast<Ice&>(base); }, "reference cast failure");
        std::unique_ptr<Effect> owner = std::move(concrete); owner.reset();
    }
    c.Expect(events.count == 4 && events.data[2] == 2 && events.data[3] == 1, "virtual destruction");
    ValueDerived derived; ValueBase sliced = derived;
    c.Expect(sliced.Kind() == 1 && derived.Kind() == 2, "slicing illustration");
    AddOverload over; c.Expect(over.F(1) == 1 && over.F(std::string("x")) == 2, "using overloads");
    HalfDamage half; c.Expect(half.Apply(10) == 5, "NVI");
    c.Throws<std::invalid_argument>([&] { (void)half.Apply(-1); }, "NVI validation");
    Number number(7); auto clone = number.Clone();
    c.Expect(clone.get() != &number && clone->Value() == 7, "polymorphic clone");
}); }

````

## `src/templates.cpp`

````cpp
#include "check.hpp"
#include <concepts>
#include <cstddef>
#include <functional>
#include <list>
#include <string>
#include <type_traits>
#include <vector>
template<class T> concept Sized = requires(const T& value) {
    { value.size() } -> std::convertible_to<std::size_t>;
};
template<Sized T> std::size_t SizeOf(const T& value) { return static_cast<std::size_t>(value.size()); }
std::string Category(std::string&) { return "mutable lvalue"; }
std::string Category(const std::string&) { return "const lvalue"; }
std::string Category(std::string&&) { return "rvalue"; }
template<class T> std::string ForwardCategory(T&& value) { return Category(std::forward<T>(value)); }
template<class T> struct Base { int Read() const { return 42; } };
template<class T> struct Derived : Base<T> { int ReadWithLog() const { return this->Read(); } };
template<class Container> typename Container::value_type FirstCopy(const Container& values) {
    if (values.empty()) throw std::out_of_range("empty");
    return *values.begin();
}
template<class T> struct Borrow {
    T* pointer;
    explicit Borrow(T* p) : pointer(p) {}
    template<class U> requires std::convertible_to<U*, T*>
    Borrow(const Borrow<U>& other) : pointer(other.pointer) {}
};
struct Animal { virtual ~Animal() = default; }; struct Cat : Animal {};
std::function<int(int)> AddLater(int offset) { return [offset](int value) { return value + offset; }; }
constexpr unsigned PowerOfTwo(unsigned exponent) {
    if (exponent >= 31) throw std::out_of_range("exponent");
    return 1u << exponent;
}
static_assert(PowerOfTwo(10) == 1024);
static_assert(std::constructible_from<Borrow<const Animal>, Borrow<Cat>>);
static_assert(!std::constructible_from<Borrow<Cat>, Borrow<Animal>>);
int main() { return Run("templates", [](Checks& c) {
    std::vector<int> values{7,8,9}; c.Expect(SizeOf(values) == 3, "concept");
    c.Expect(FirstCopy(values) == 7, "dependent type");
    std::list<int> empty; c.Throws<std::out_of_range>([&] { (void)FirstCopy(empty); }, "empty");
    Derived<int> derived; c.Expect(derived.ReadWithLog() == 42, "dependent base");
    std::string text = "x"; const std::string fixed = "y";
    c.Expect(ForwardCategory(text) == "mutable lvalue", "forward lvalue");
    c.Expect(ForwardCategory(fixed) == "const lvalue", "forward const");
    c.Expect(ForwardCategory(std::string("z")) == "rvalue", "forward rvalue");
    auto callback = AddLater(5); c.Expect(callback(7) == 12, "capture by value");
    Cat cat; Borrow<Cat> child(&cat); Borrow<const Animal> parent(child);
    c.Expect(parent.pointer == &cat, "converting member template");
    c.Throws<std::out_of_range>([] { (void)PowerOfTwo(31); }, "constexpr runtime error");
}); }

````

## `src/transaction.cpp`

````cpp
#include "check.hpp"
#include <memory>
#include <string>
#include <vector>
struct State { std::vector<int> inventory; std::string scene; };
class GameState {
    std::unique_ptr<State> state_ = std::make_unique<State>(State{{1,2},"old"});
public:
    const State& Get() const noexcept { return *state_; }
    void Transition(std::string scene, bool fail_validation = false) {
        auto candidate = std::make_unique<State>(*state_);
        candidate->inventory.push_back(3);
        candidate->scene = std::move(scene);
        if (fail_validation || candidate->scene.empty()) throw std::runtime_error("validation");
        state_.swap(candidate); // Commit only owned state; no external I/O is performed.
    }
};
struct Resource { inline static int alive{}; Resource() { ++alive; } ~Resource() { --alive; } };
int FailPriority() { throw std::runtime_error("priority"); }
void Consume(std::unique_ptr<Resource>, int) {}
int main() { return Run("transaction", [](Checks& c) {
    GameState game;
    c.Throws<std::runtime_error>([&] { game.Transition("new", true); }, "late failure");
    c.Expect(game.Get().scene == "old" && game.Get().inventory == std::vector<int>({1,2}), "strong guarantee");
    game.Transition("new"); c.Expect(game.Get().scene == "new" && game.Get().inventory.size() == 3, "commit");
    c.Throws<std::runtime_error>([] { Consume(std::make_unique<Resource>(), FailPriority()); }, "argument failure");
    c.Expect(Resource::alive == 0, "RAII despite argument failure");
    auto owner = std::make_shared<Resource>(); std::weak_ptr<Resource> observer = owner;
    c.Expect(!observer.expired() && observer.lock() != nullptr, "weak lock");
    owner.reset(); c.Expect(observer.expired() && !observer.lock(), "weak not owning");
    c.Expect(Resource::alive == 0, "reclaimed");
}); }

````

## `negative/constraint.cpp`

````cpp
#include <concepts>
template<std::integral T> T F(T value) { return value; }
int main() { return F(1.5); }

````

## `negative/copy_by_value.cpp`

````cpp
struct Bad { Bad(Bad); };

````

## `negative/copy_unique.cpp`

````cpp
#include <memory>
void F() { auto a = std::make_unique<int>(1); auto b = a; }

````

## `negative/list_sort.cpp`

````cpp
#include <algorithm>
#include <list>
void F() { std::list<int> values{3,1,2}; std::sort(values.begin(), values.end()); }

````

## `negative/temporary_view.cpp`

````cpp
#include <string>
#include <string_view>
struct Label {
    std::string data;
    std::string_view View() const & { return data; }
    std::string_view View() && = delete;
};
int main() { auto view = Label{"temporary"}.View(); return static_cast<int>(view.size()); }

````

## `negative/wrong_override.cpp`

````cpp
struct Base { virtual int Read() const = 0; };
struct Derived : Base { int Read() override { return 1; } };

````

# 실제 검증 결과 · 2026-10-13

환경: Linux-6.18.44-x86_64-with-glibc2.41

## gcc
g++ (Debian 14.2.0-19) 14.2.0

정상 실행: 9/9, 조건 검사 89회

- lifetime: 10회
- ownership: 13회
- polymorphism: 12회
- templates: 10회
- containers: 16회
- interfaces: 7회
- transaction: 8회
- allocation: 7회
- pimpl: 6회

## clang
clang version 17.0.0 (https://github.com/swiftlang/llvm-project.git 10999b6d034fe318f3d56c83bddb6572593a8bb0)

정상 실행: 9/9, 조건 검사 89회

- lifetime: 10회
- ownership: 13회
- polymorphism: 12회
- templates: 10회
- containers: 16회
- interfaces: 7회
- transaction: 8회
- allocation: 7회
- pimpl: 6회

## clang-sanitized
clang version 17.0.0 (https://github.com/swiftlang/llvm-project.git 10999b6d034fe318f3d56c83bddb6572593a8bb0)

정상 실행: 9/9, 조건 검사 89회

- lifetime: 10회
- ownership: 13회
- polymorphism: 12회
- templates: 10회
- containers: 16회
- interfaces: 7회
- transaction: 8회
- allocation: 7회
- pimpl: 6회

## 한계
Windows/MSVC·GPU·실제 게임 통합·경쟁 조건·성능은 미검증. 로그는 특정 실행 경로의 결과이며 언어 규칙의 증명이나 전체 코드 커버리지가 아닙니다.
