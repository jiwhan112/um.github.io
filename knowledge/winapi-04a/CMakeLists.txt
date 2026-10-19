cmake_minimum_required(VERSION 3.16)
project(winapi04a_cpu_timestep LANGUAGES CXX)
set(LAB_CXX_STANDARD 20 CACHE STRING "20 or 11")
enable_testing()
add_executable(winapi04a_cpu main.cpp)
set_target_properties(winapi04a_cpu PROPERTIES
    CXX_STANDARD ${LAB_CXX_STANDARD}
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)
if(MSVC)
    target_compile_options(winapi04a_cpu PRIVATE /W4 /EHsc)
else()
    target_compile_options(winapi04a_cpu PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
add_test(NAME winapi04a_cpu COMMAND winapi04a_cpu)
