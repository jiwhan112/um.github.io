cmake_minimum_required(VERSION 3.10)
project(dx9_alpha_contract_cpu LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(alpha_contract main.cpp)
enable_testing()
add_test(NAME alpha_contract_cpu COMMAND alpha_contract)

