cmake_minimum_required(VERSION 3.16)
project(mavlink_daemon CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)

add_executable(daemon src/main.cpp)

target_include_directories(daemon SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mavlink/v2.0
)