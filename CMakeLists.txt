cmake_minimum_required(VERSION 3.24)
project(SimpleThreadPool)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
find_package(benchmark REQUIRED CONFIG)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
include_directories(
        .
)

add_executable(SimpleThreadPool main.cpp thread_pool.h thread_pool.cpp lock_free_queue.h ThridPartyLibs/concurrentqueue.h)
target_link_libraries(SimpleThreadPool benchmark::benchmark unofficial::concurrentqueue::concurrentqueue)