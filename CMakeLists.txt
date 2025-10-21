cmake_minimum_required(VERSION 3.21)
project(Toad8 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Toad8 
    src/main.cpp
    src/chip8.cpp
    )

find_package(raylib CONFIG REQUIRED)
target_link_libraries(Toad8 PRIVATE raylib)