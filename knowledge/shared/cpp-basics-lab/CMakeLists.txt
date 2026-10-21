cmake_minimum_required(VERSION 3.20)
project(atlas_cpp_basics_teaching LANGUAGES CXX)

# 이 실습 팩은 C++20 고정이다. C++11 요청을 받아 놓고 20으로 바꾸는 혼동을 막는다.
if(DEFINED CMAKE_CXX_STANDARD AND NOT CMAKE_CXX_STANDARD STREQUAL "20")
    message(FATAL_ERROR "This preserved lab requires C++20. Use verify_experiments.py for the separate C++11 rejection probe.")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(CPP_BASICS_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer on positive tests" OFF)

enable_testing()
# 회수한 원본 CMake를 편집하지 않고 여섯 정상 실행 파일과 테스트를 그대로 구성한다.
add_subdirectory(original)
foreach(name IN ITEMS types flow memory)
    add_executable(${name}_ko ko/${name}.cpp)
    target_include_directories(${name}_ko PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/original/src")
    add_test(NAME ${name}_ko COMMAND ${name}_ko)
endforeach()

foreach(name IN ITEMS types flow memory containers buffer inventory types_ko flow_ko memory_ko)
    if(MSVC)
        target_compile_options(${name} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
    endif()
    if(CPP_BASICS_SANITIZERS)
        if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            message(FATAL_ERROR "The sanitizer configuration requires Clang or GCC.")
        endif()
        target_compile_options(${name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()
endforeach()

# negative/의 컴파일 거부와 diagnostics/의 의도적 미정의 동작은 정상 CTest에 등록하지 않는다.
