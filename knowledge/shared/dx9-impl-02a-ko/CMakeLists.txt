cmake_minimum_required(VERSION 3.16)
project(dx9_dll_boundary_lab LANGUAGES CXX)

add_library(engine SHARED src/engine_api.cpp)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(engine PRIVATE ENGINE_BUILD)
set_target_properties(engine PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_executable(client src/client.cpp)
add_executable(engine_contract_tests tests/engine_contract_tests.cpp)
foreach(target IN ITEMS client engine_contract_tests)
  target_link_libraries(${target} PRIVATE engine)
  set_target_properties(${target} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endforeach()

# On Windows, both engine.dll and executables are placed in build/bin/<config>.
# On Linux, CMake sets a build-tree RUNPATH to the engine library location.
enable_testing()
add_test(NAME client_smoke COMMAND client)
add_test(NAME exported_api_contract COMMAND engine_contract_tests)
