cmake_minimum_required(VERSION 3.24)
project(SimpleThreadPool)

set(CMAKE_CXX_STANDARD 20)

find_package(benchmark REQUIRED CONFIG)

include_directories(
        .
)

add_executable(SimpleThreadPool main.cpp thread_pool.h thread_pool.cpp)
target_link_libraries(SimpleThreadPool benchmark::benchmark)