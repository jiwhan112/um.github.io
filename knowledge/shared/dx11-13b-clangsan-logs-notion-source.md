원본 기록의 각 stdout를 그대로 보존합니다. 각 단계 stderr는 실제 0바이트였습니다(컴파일 경고 없음). 출력 코드 펜스의 표시용 개행은 제외하고 FNV64 검증합니다. 21개 고유 논리 조건을 동일하게 검사한 환경별 반복입니다.
## configure · stdout 417바이트 · FNV64 a1dc987ca51cb0bc
```plain text
-- The CXX compiler identification is Clang 17.0.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/local/swift/usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (1.0s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx11_13b_inputlayout/build_clangsan
```
stderr: **0바이트**, SHA256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
## build · stdout 151바이트 · FNV64 07e4ca854d8e0716
```plain text
[ 50%] Building CXX object CMakeFiles/layout_contract.dir/main.cpp.o
[100%] Linking CXX executable layout_contract
[100%] Built target layout_contract
```
stderr: **0바이트**, SHA256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
## ctest · stdout 979바이트 · FNV64 108526f3c515ca79
```plain text
Internal ctest changing into directory: /mnt/data/dx11_13b_inputlayout/build_clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_inputlayout/build_clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_inputlayout/build_clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_13b_inputlayout/build_clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: two_pass_layout_validation

1: Test command: /mnt/data/dx11_13b_inputlayout/build_clangsan/layout_contract
1: Working Directory: /mnt/data/dx11_13b_inputlayout/build_clangsan
1: Test timeout computed to be: 10000000
1: PASS 21 unique layout contract checks
1/1 Test #1: two_pass_layout_validation .......   Passed    0.02 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.02 sec
```
stderr: **0바이트**, SHA256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
