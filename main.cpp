#include <iostream>
#include <bits/stdc++.h>
#include <benchmark/benchmark.h>
#include <thread_pool.h>
// 15_parallel_algorithm.cpp
using namespace std;
const int kTaskNum = 20;
void time_consume_func(const string & method = ""){
  int id = rand() % 10000;
//  std::cout << id << " " << method << "Start sleep:" << std::endl;
    /*int siz = 2000;
    std::vector<int>vec(siz,0);
    for(int i = 1;i <= 1000;i++) {
        int x = rand() * i + 10;
        int idx = (i * x) % siz;
        idx = abs(idx) % siz;
        vec[idx] = x * rand()  + 2;
    }*/
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
//    std::cout << id << " " << method << " End." << std::endl;
}
void default_run(int n){
    for(int i = 1;i <= n;i++){
        time_consume_func("[default]");
    }
}
void naive_thread_run(int n){

    std::vector<std::thread>threads;
    for(int i = 1;i <= n;i++){
        threads.emplace_back(time_consume_func,"[naive]");
    }
    for(auto & it:threads){
        it.join();
    }
}
void f(){
  std::cout << "qwq" << std::endl;
}
ThreadPool::ThreadPool thread_pool(8);
void thread_pool_run(int n){

  std::vector<future<decltype(time_consume_func())>>futures;
  for(int i = 1;i <= n;i++){
    futures.emplace_back(thread_pool.submit(time_consume_func,"[my thread pool]"));
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


static void BM_naive_thread_run(benchmark::State & state){
    for(auto _:state){
        naive_thread_run(kTaskNum);
    }
}



static void BM_thread_pool_run(benchmark::State & state){
  for(auto _:state){
    thread_pool_run(kTaskNum);
  }
}


BENCHMARK(BM_default_run);
BENCHMARK(BM_naive_thread_run);
BENCHMARK(BM_thread_pool_run);
BENCHMARK_MAIN();

