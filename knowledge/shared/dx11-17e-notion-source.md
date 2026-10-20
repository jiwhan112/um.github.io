## GCC14.2 C++11 · 구성·빌드·CTest 원시 로그
실제 CTest 1/1 통과, 조건 49개는 C++20과 같은 검사.
### font_configure.log
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/notion_dx11_audit_20kst/build/font-gcc11
```
### font_build.log
```plain text
[ 50%] Building CXX object CMakeFiles/font_cpu.dir/main.cpp.o
[100%] Linking CXX executable font_cpu
[100%] Built target font_cpu
```
### font_ctest.log
```plain text
Internal ctest changing into directory: /mnt/data/notion_dx11_audit_20kst/build/font-gcc11
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/font-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/notion_dx11_audit_20kst/build/font-gcc11/DartConfiguration.tcl
Test project /mnt/data/notion_dx11_audit_20kst/build/font-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: font_cpu_contract

1: Test command: /mnt/data/notion_dx11_audit_20kst/build/font-gcc11/font_cpu
1: Working Directory: /mnt/data/notion_dx11_audit_20kst/build/font-gcc11
1: Test timeout computed to be: 10000000
1: PASS 49 Font UTF-8 atlas layout CPU contract checks
1/1 Test #1: font_cpu_contract ................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
