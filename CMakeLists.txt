cmake_minimum_required(VERSION 3.10.2)
project(avl)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(benchmark)

add_executable(avl avl.cpp)
set_property(TARGET avl PROPERTY CXX_STANDARD 20)

add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE benchmark::benchmark)
set_property(TARGET bench PROPERTY CXX_STANDARD 20)
