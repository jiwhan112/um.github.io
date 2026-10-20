<callout icon="🧪" color="green_bg">
	**13B CPU InputLayout 독립 실습 검증 원시 로그 보관**. 동일한 13개 CPU 조건을 GCC C++20, GCC C++11 및 Clang ASan/UBSan에서 각각 CTest 1/1 수행했습니다. 환경 반복을 서로 다른 39개 검사로 집계하지 않습니다.
</callout>
## 입력과 빌드 기준
- 상위 실습 <mention-page url="https://app.notion.com/p/3fda99703f4081ef8863c783ed5a6e8f">13B · Instancing InputLayout 오버플로·StepRate 경계 CPU 실습</mention-page>: main.cpp 2,840B FNV64 02218c40adef928a, CMake 616B FNV64 7ad4bb35e2621f80. 모두 Notion에서 재조회한 원본과 로컬 빌드 입력을 일치시킨 자료입니다.
- Clang C++20 빌드는 CXXFLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer 및 LDFLAGS=-fsanitize=address,undefined로 **첫 Configure부터** 적용했습니다. GPU, MSVC, HLSL 컴파일은 미실행.
## 실제 Configure / Build / CTest -V 원시 출력 (9개)
### gcc20-configure.log · 388바이트 · FNV64 e9e988284044ba2d
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx11_13b_compact/build-gcc20
```
### gcc20-build.log · 151바이트 · FNV64 8a7b90a81c99dab1
```plain text
[ 50%] Building CXX object CMakeFiles/dx11_13b_layout.dir/main.cpp.o
[100%] Linking CXX executable dx11_13b_layout
[100%] Built target dx11_13b_layout
```
### gcc20-ctest.log · 919바이트 · FNV64 f5674828cc4d594c
```plain text
Internal ctest changing into directory: /mnt/data/dx11_13b_compact/build-gcc20
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_compact/build-gcc20/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_compact/build-gcc20/DartConfiguration.tcl
Test project /mnt/data/dx11_13b_compact/build-gcc20
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13b_layout

1: Test command: /mnt/data/dx11_13b_compact/build-gcc20/dx11_13b_layout
1: Working Directory: /mnt/data/dx11_13b_compact/build-gcc20
1: Test timeout computed to be: 10000000
1: PASS 13 CPU InputLayout checks
1/1 Test #1: dx11_13b_layout ..................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
### gcc11-configure.log · 388바이트 · FNV64 f3032928458c0121
```plain text
-- The CXX compiler identification is GNU 14.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx11_13b_compact/build-gcc11
```
### gcc11-build.log · 151바이트 · FNV64 8a7b90a81c99dab1
```plain text
[ 50%] Building CXX object CMakeFiles/dx11_13b_layout.dir/main.cpp.o
[100%] Linking CXX executable dx11_13b_layout
[100%] Built target dx11_13b_layout
```
### gcc11-ctest.log · 919바이트 · FNV64 1d9bb55bd192b428
```plain text
Internal ctest changing into directory: /mnt/data/dx11_13b_compact/build-gcc11
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_compact/build-gcc11/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_compact/build-gcc11/DartConfiguration.tcl
Test project /mnt/data/dx11_13b_compact/build-gcc11
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13b_layout

1: Test command: /mnt/data/dx11_13b_compact/build-gcc11/dx11_13b_layout
1: Working Directory: /mnt/data/dx11_13b_compact/build-gcc11
1: Test timeout computed to be: 10000000
1: PASS 13 CPU InputLayout checks
1/1 Test #1: dx11_13b_layout ..................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec
```
### clangsan-configure.log · 413바이트 · FNV64 846c4d8dc7012069
```plain text
-- The CXX compiler identification is Clang 17.0.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/local/swift/usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done (0.5s)
-- Generating done (0.0s)
-- Build files have been written to: /mnt/data/dx11_13b_compact/build-clangsan
```
### clangsan-build.log · 151바이트 · FNV64 8a7b90a81c99dab1
```plain text
[ 50%] Building CXX object CMakeFiles/dx11_13b_layout.dir/main.cpp.o
[100%] Linking CXX executable dx11_13b_layout
[100%] Built target dx11_13b_layout
```
### clangsan-ctest.log · 937바이트 · FNV64 0cdecc480fae7636
```plain text
Internal ctest changing into directory: /mnt/data/dx11_13b_compact/build-clangsan
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_compact/build-clangsan/DartConfiguration.tcl
UpdateCTestConfiguration  from :/mnt/data/dx11_13b_compact/build-clangsan/DartConfiguration.tcl
Test project /mnt/data/dx11_13b_compact/build-clangsan
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: dx11_13b_layout

1: Test command: /mnt/data/dx11_13b_compact/build-clangsan/dx11_13b_layout
1: Working Directory: /mnt/data/dx11_13b_compact/build-clangsan
1: Test timeout computed to be: 10000000
1: PASS 13 CPU InputLayout checks
1/1 Test #1: dx11_13b_layout ..................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```
## 로그 검증·변경 이력
- 저장되는 9개 텍스트 블록은 위 로컬 실측 원본의 bytes와 FNV64가 **작성 전 9/9 일치**한 자료입니다. 저장 후 fetch해서 각 블록 재구성 결과를 검사해야 Notion 영구 검증 완료로 간주합니다.
- <mention-date start="2026-10-18"/>: DX11 13B의 GCC20/GCC11/ClangSan Configure·Build·CTest 로그를 독립 네이티브 블록으로 정리. 빌드 도구 출력은 실행 환경에 따라 다른 경로·시간을 포함할 수 있습니다.
