cmake_minimum_required(VERSION 3.16)
project(snake CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)


add_executable(snake 
    random.h
    src/main.cpp)

target_compile_options(snake PRIVATE -Wall -Wextra -Wshadow)
target_include_directories(snake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snake PRIVATE 
    SDL3_image::SDL3_image
    SDL3::SDL3
)
 
