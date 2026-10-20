## Source provenance
Original v2 CPU main: <mention-page url="https://app.notion.com/p/3fba99703f4081c68669c3d4a2d5eb89"/>. It is a portable application-side RTV/SRV pass-ownership model, not the actual D3D11 API's automatic hazard resolution. Existing main is retained in its original native code block.
## Reproducibility
Recovered `main.cpp`: 6,550 ASCII bytes, FNV64 `c06dab334f839884`; exact match between original native code block and compiled local source. Windows/MSVC/GPU not run.
## 재현용 CMakeLists.txt — 457바이트, FNV64 b759ff16885f7496
```plain text
cmake_minimum_required(VERSION 3.16)
project(Deferred_CPU_v2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(deferred_cpu main.cpp)
if(MSVC)
  target_compile_options(deferred_cpu PRIVATE /W4)
else()
  target_compile_options(deferred_cpu PRIVATE -Wall -Wextra -Wpedantic)
endif()
enable_testing()
add_test(NAME deferred_cpu_contract COMMAND deferred_cpu)
```
기존 18B 코드 블록을 `main.cpp`로 저장하고 같은 폴더에서 위 CMake를 사용합니다. C++11 구성은 `-DCMAKE_CXX_STANDARD=11`, Clang sanitizer 구성은 `-fsanitize=address,undefined -fno-omit-frame-pointer`를 컴파일·링크에 함께 지정합니다.
## 2026-10-17 20시 · GCC14.2 C++11 원시 로그
세 개의 환경별 원시 configure/build/CTest 출력. 테스트 1/1 통과, 환경별 반복은 고유 테스트로 중복 집계하지 않음.
### gcc11_deferred_configure.log · 404 bytes · FNV64 9bf969f94ad06e15
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc11
```
### gcc11_deferred_build.log · 142 bytes · FNV64 faca72c4eb6142eb
```plain text
[ 50%] Building CXX object CMakeFiles/deferred_cpu.dir/main.cpp.o
[100%] Linking CXX executable deferred_cpu
[100%] Built target deferred_cpu
```
### gcc11_deferred_ctest.log · 1039 bytes · FNV64 acdcda334d32663c
```plain text
Internal ctest changing into directory: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc11
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/deferred-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/deferred-gcc11/DartConfiguration.tcl
Test project /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: deferred_cpu_contract

1: Test command: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc11/deferred_cpu
1: Working Directory: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc11
1: Test timeout computed to be: 10000000
1: PASS 41 Deferred MRT SRV hazard CPU contract checks
1/1 Test #1: deferred_cpu_contract ............   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
## 2026-10-17 20시 · Clang17 C++20 ASan/UBSan 원시 로그
세 개의 환경별 원시 configure/build/CTest 출력. 테스트 1/1 통과, 환경별 반복은 고유 테스트로 중복 집계하지 않음.
### clangsan_deferred_configure.log · 429 bytes · FNV64 66d0725ae0570c0e
```plain text
-- The CXX compiler identification is Clang 17.0.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/local/swift/usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.4s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/notion_dx11_audit_20kst/build/deferred-clangsan
```
### clangsan_deferred_build.log · 142 bytes · FNV64 faca72c4eb6142eb
```plain text
[ 50%] Building CXX object CMakeFiles/deferred_cpu.dir/main.cpp.o
[100%] Linking CXX executable deferred_cpu
[100%] Built target deferred_cpu
```
### clangsan_deferred_ctest.log · 1057 bytes · FNV64 075c2c7a15b61dfa
```plain text
Internal ctest changing into directory: /mnt/data/notion_dx11_audit_20kst/build/deferred-clangsan
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/deferred-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/deferred-clangsan/DartConfiguration.tcl
Test project /mnt/data/notion_dx11_audit_20kst/build/deferred-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: deferred_cpu_contract

1: Test command: /mnt/data/notion_dx11_audit_20kst/build/deferred-clangsan/deferred_cpu
1: Working Directory: /mnt/data/notion_dx11_audit_20kst/build/deferred-clangsan
1: Test timeout computed to be: 10000000
1: PASS 41 Deferred MRT SRV hazard CPU contract checks
1/1 Test #1: deferred_cpu_contract ............   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```
### deferred GCC14.2 C++20 · configure 원시 로그 · 404바이트 · FNV64 92dfd8f945894251
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc20
```
### deferred GCC14.2 C++20 · build 원시 로그 · 142바이트 · FNV64 faca72c4eb6142eb
```plain text
[ 50%] Building CXX object CMakeFiles/deferred_cpu.dir/main.cpp.o
[100%] Linking CXX executable deferred_cpu
[100%] Built target deferred_cpu
```
### deferred GCC14.2 C++20 · ctest 원시 로그 · 1039바이트 · FNV64 e908ab10db0c918c
```plain text
Internal ctest changing into directory: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc20
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/deferred-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/deferred-gcc20/DartConfiguration.tcl
Test project /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: deferred_cpu_contract

1: Test command: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc20/deferred_cpu
1: Working Directory: /mnt/data/notion_dx11_audit_20kst/build/deferred-gcc20
1: Test timeout computed to be: 10000000
1: PASS 41 Deferred MRT SRV hazard CPU contract checks
1/1 Test #1: deferred_cpu_contract ............   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
## 재현 명령 · 18C
기존 네이티브 소스 `18B/main.cpp`를 위 CMake와 같은 디렉터리에 저장합니다. 아래는 실제 사용한 명령의 형식입니다.
```bash
cmake -S deferred -B build/deferred-gcc20 -DCMAKE_CXX_COMPILER=g++ -DCMAKE_CXX_STANDARD=20
cmake --build build/deferred-gcc20 -j2
ctest --test-dir build/deferred-gcc20 --output-on-failure -V
cmake -S deferred -B build/deferred-gcc11 -DCMAKE_CXX_COMPILER=g++ -DCMAKE_CXX_STANDARD=11
cmake --build build/deferred-gcc11 -j2
ctest --test-dir build/deferred-gcc11 --output-on-failure -V
cmake -S deferred -B build/deferred-clangsan -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer'
cmake --build build/deferred-clangsan -j2
ctest --test-dir build/deferred-clangsan --output-on-failure -V
```
모든 빌드 디렉터리는 서로 분리합니다. 17D/18C에 남긴 실제 로그의 절대 경로는 `/mnt/data/notion_dx11_audit_20kst` 아래입니다.
