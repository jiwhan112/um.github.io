# 다운로드 전용 빌드: 원문 CMake는 reference/CMakeLists.txt에 보존합니다.
cmake_minimum_required(VERSION 3.10)
project(dx9_sky_ui_cpu_lab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(dx9_sky_ui_cpu main.cpp)
enable_testing()
add_test(NAME dx9_sky_ui_cpu COMMAND dx9_sky_ui_cpu)