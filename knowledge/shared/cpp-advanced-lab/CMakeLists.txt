cmake_minimum_required(VERSION 3.20)
project(atlas_cpp_advanced_teaching LANGUAGES CXX)

# 보존 실습은 C++20 기준이다. C++11 요청을 조용히 다른 표준으로 바꾸지 않는다.
if(DEFINED CMAKE_CXX_STANDARD AND NOT CMAKE_CXX_STANDARD STREQUAL "20")
    message(FATAL_ERROR "This preserved lab requires C++20. Use the separate strict C++11 compatibility probe.")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(ENABLE_SANITIZERS AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message(FATAL_ERROR "This sanitizer configuration requires Clang or GCC.")
endif()

enable_testing()
# ZIP의 원본 CMake와 정상 프로그램 9개를 수정 없이 구성한다.
add_subdirectory(original)

foreach(name IN ITEMS lifetime ownership transaction polymorphism templates interfaces containers allocation pimpl)
    add_executable(${name}_ko ko/${name}.cpp)
    # Pimpl은 구현과 공개 헤더를 사용하는 main을 별도 번역 단위로 컴파일·링크한다.
    if(name STREQUAL "pimpl")
        target_sources(${name}_ko PRIVATE ko/pimpl_main.cpp)
    endif()
    target_include_directories(${name}_ko PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ko")
    if(MSVC)
        target_compile_options(${name}_ko PRIVATE /W4 /permissive- /EHsc /utf-8)
    else()
        target_compile_options(${name}_ko PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
        if(ENABLE_SANITIZERS)
            target_compile_options(${name}_ko PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${name}_ko PRIVATE -fsanitize=address,undefined)
        endif()
    endif()
    add_test(NAME ${name}_ko COMMAND ${name}_ko)
endforeach()

# negative/의 여섯 컴파일 거부 예제는 정상 CTest에 넣지 않는다.
# 이번 팩의 실패 주입은 정상 프로그램 내부의 예외 경로이며 의도적 UB가 아니다.
