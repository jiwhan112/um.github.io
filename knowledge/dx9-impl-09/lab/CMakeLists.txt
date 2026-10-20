cmake_minimum_required(VERSION 3.20)
project(dx9_09c_korean_commentary LANGUAGES CXX)

# 원본 CMake는 original/에 그대로 보존하고, 이 파생 빌드에서 비교할 표준을 선택한다.
set(LAB_CXX_STANDARD "11" CACHE STRING "C++ standard: 11 or 20")
if(NOT LAB_CXX_STANDARD STREQUAL "11" AND NOT LAB_CXX_STANDARD STREQUAL "20")
  message(FATAL_ERROR "LAB_CXX_STANDARD must be 11 or 20")
endif()
set(CMAKE_CXX_STANDARD ${LAB_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(alpha_original original/main.cpp)
add_executable(alpha_korean main.ko.cpp)
foreach(target alpha_original alpha_korean)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /utf-8)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endforeach()

enable_testing()
# 같은 53개 조건을 두 표현으로 실행한다. 독립 조건 수를 106개로 합산하지 않는다.
add_test(NAME alpha_original_cpu COMMAND alpha_original)
add_test(NAME alpha_korean_cpu COMMAND alpha_korean)
