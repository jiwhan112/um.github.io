## 검증 계획
기존 17B 헤더와 17C main의 CPU 참조 버전만 재현합니다. Windows/MSVC/D3D11/GPU 미검증. CMake와 실행 로그는 추가 검증 후 업데이트합니다.
## 원본 소스 연결·해시
17B 헤더: <mention-page url="https://app.notion.com/p/3fba99703f408105a5c8c1d371e2c8cb"/> · 3,777바이트 · FNV64 `df9977f4b72878d6`.
17C main: <mention-page url="https://app.notion.com/p/3fba99703f4081aeb641e5b540bf2848"/> · 3,736바이트 · FNV64 `0c6e6785dee070f8`.
두 원본 페이지의 네이티브 C++ 코드 블록을 추출해 로컬 파일로 재구성했고 각각 정확히 동일한 FNV64를 확인했습니다.
## 테스트 CMakeLists.txt
```plain text
cmake_minimum_required(VERSION 3.16)
project(Font_CPU_v2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(font_cpu main.cpp)
if(MSVC)
  target_compile_options(font_cpu PRIVATE /W4)
else()
  target_compile_options(font_cpu PRIVATE -Wall -Wextra -Wpedantic)
endif()
enable_testing()
add_test(NAME font_cpu_contract COMMAND font_cpu)
```
본 CMake는 433바이트, FNV64 `2538e08efd2fbfce`입니다. 기존 17B/17C 코드는 덮어쓰지 않았습니다.
## 2026-10-17 20시 · GCC14.2 C++20 원시 로그
아래 3블록은 실제 환경의 원시 configure/build/CTest 출력이며 세 파일의 끝 개행을 포함합니다. GCC20 CTest 1/1, Font 49 조건 통과.
### font_configure.log
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/notion_dx11_audit_20kst/build/font-gcc20
```
### font_build.log
```plain text
[ 50%] Building CXX object CMakeFiles/font_cpu.dir/main.cpp.o
[100%] Linking CXX executable font_cpu
[100%] Built target font_cpu
```
### font_ctest.log
```plain text
Internal ctest changing into directory: /mnt/data/notion_dx11_audit_20kst/build/font-gcc20
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/font-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/font-gcc20/DartConfiguration.tcl
Test project /mnt/data/notion_dx11_audit_20kst/build/font-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: font_cpu_contract

1: Test command: /mnt/data/notion_dx11_audit_20kst/build/font-gcc20/font_cpu
1: Working Directory: /mnt/data/notion_dx11_audit_20kst/build/font-gcc20
1: Test timeout computed to be: 10000000
1: PASS 49 Font UTF-8 atlas layout CPU contract checks
1/1 Test #1: font_cpu_contract ................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
### font Clang17 C++20 ASan/UBSan · configure 원시 로그 · 425바이트 · FNV64 a97e7bf4049da895
```plain text
-- The CXX compiler identification is Clang 17.0.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/local/swift/usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.7s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/notion_dx11_audit_20kst/build/font-clangsan
```
### font Clang17 C++20 ASan/UBSan · ctest 원시 로그 · 1025바이트 · FNV64 b930400969eef57b
```plain text
Internal ctest changing into directory: /mnt/data/notion_dx11_audit_20kst/build/font-clangsan
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/font-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/font-clangsan/DartConfiguration.tcl
Test project /mnt/data/notion_dx11_audit_20kst/build/font-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: font_cpu_contract

1: Test command: /mnt/data/notion_dx11_audit_20kst/build/font-clangsan/font_cpu
1: Working Directory: /mnt/data/notion_dx11_audit_20kst/build/font-clangsan
1: Test timeout computed to be: 10000000
1: PASS 49 Font UTF-8 atlas layout CPU contract checks
1/1 Test #1: font_cpu_contract ................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```
<page url="https://app.notion.com/p/3fca99703f408196bfa1e7f44bde7677">17E · Font GCC C++11 실제 원시 로그 3종</page>
<page url="https://app.notion.com/p/3fca99703f4081f397a3db46cf8b8273">17F · Font Clang ASan/UBSan 원시 Build 로그</page>
