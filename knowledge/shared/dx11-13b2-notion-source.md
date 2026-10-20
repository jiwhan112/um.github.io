CMake 3.16 이상; 기본 C++20, -DLAB_CXX_STANDARD=11로 C++11. 라이브러리 없이 동작하는 CPU-only 검사.
```plain text
cmake_minimum_required(VERSION 3.16)
project(dx11_13b_cpu LANGUAGES CXX)
set(LAB_CXX_STANDARD "20" CACHE STRING "11 or 20")
if(NOT LAB_CXX_STANDARD STREQUAL "11" AND NOT LAB_CXX_STANDARD STREQUAL "20")
  message(FATAL_ERROR "LAB_CXX_STANDARD must be 11 or 20")
endif()
add_executable(layout_contract main.cpp)
set_target_properties(layout_contract PROPERTIES CXX_STANDARD ${LAB_CXX_STANDARD} CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
if(MSVC)
  target_compile_options(layout_contract PRIVATE /W4)
else()
  target_compile_options(layout_contract PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
enable_testing()
add_test(NAME two_pass_layout_validation COMMAND layout_contract)
```
## 실제 실행 명령 (Linux)
```bash
cmake -S . -B build_gcc20 -DLAB_CXX_STANDARD=20 -DCMAKE_CXX_COMPILER=g++
cmake --build build_gcc20
ctest --test-dir build_gcc20 -V
cmake -S . -B build_gcc11 -DLAB_CXX_STANDARD=11 -DCMAKE_CXX_COMPILER=g++
cmake --build build_gcc11
ctest --test-dir build_gcc11 -V
cmake -S . -B build_clangsan -DLAB_CXX_STANDARD=20 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build_clangsan
ctest --test-dir build_clangsan -V
```
원본 SHA-256: `699e52c8ebc0ab3dedb48554df11cffc97655568bf631cbe7c79326db574e126`.
