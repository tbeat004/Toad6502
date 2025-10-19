cmake_minimum_required(VERSION 3.21)
project(MyRaylibGame LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Toad6502 
    src/main.cpp
    src/chip8.cpp
    )

find_package(raylib CONFIG REQUIRED)
target_link_libraries(Toad6502 PRIVATE raylib)