cmake_minimum_required(VERSION 3.10.2)
project(avl)

Include(FetchContent)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(benchmark)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE benchmark::benchmark)
set_property(TARGET bench PROPERTY CXX_STANDARD 20)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
set_property(TARGET tests PROPERTY CXX_STANDARD 20)
target_compile_definitions(tests PRIVATE AVL_CHECKS_ENABLED=1)
