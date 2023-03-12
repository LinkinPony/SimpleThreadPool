#include <iostream>
#include <bits/stdc++.h>
#include <benchmark/benchmark.h>
#include <thread_pool.h>
// 15_parallel_algorithm.cpp
using namespace std;
const int kTaskNum = 20;
void time_consume_func(){
    int siz = 2000;
    std::vector<int>vec(siz,0);
    for(int i = 1;i <= 10000;i++) {
        int x = rand() * i + 10;
        int idx = (i * x) % siz;
        idx = abs(idx) % siz;
        vec[idx] = x * rand()  + 2;
    }
}
void default_run(int n){
    for(int i = 1;i <= n;i++){
        time_consume_func();
    }
}
void naive_thread_run(int n){

    std::vector<std::thread>threads;
    for(int i = 1;i <= n;i++){
        threads.emplace_back(time_consume_func);
    }
    for(auto & it:threads){
        it.join();
    }
}
void f(){
  std::cout << "qwq" << std::endl;
}
ThreadPool::ThreadPool thread_pool(12);
void thread_pool_run(int n){

  std::vector<future<decltype(time_consume_func())>>futures;
  for(int i = 1;i <= n;i++){
    futures.emplace_back(thread_pool.submit(time_consume_func));
  }
  for(auto & it:futures){
    it.get();
  }
//  thread_pool.stop();
}

static void BM_default_run(benchmark::State & state){
    for(auto _:state){
        default_run(kTaskNum);
    }
}
BENCHMARK(BM_default_run);

static void BM_naive_thread_run(benchmark::State & state){
    for(auto _:state){
        naive_thread_run(kTaskNum);
    }
}
BENCHMARK(BM_naive_thread_run);


static void BM_thread_pool_run(benchmark::State & state){
  for(auto _:state){
    thread_pool_run(kTaskNum);
  }
}
BENCHMARK(BM_thread_pool_run);
BENCHMARK_MAIN();

