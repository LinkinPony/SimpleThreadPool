#include <iostream>
#include <bits/stdc++.h>
#include <benchmark/benchmark.h>
#include <thread_pool.h>
#include <lock_free_queue.h>
#include <cassert>
#include "ThridPartyLibs/concurrentqueue.h"
// 15_parallel_algorithm.cpp
using namespace std;
const int kTaskNum = 16;
class ThridQueueWrapper : public ThreadPool::QueueInterface{
 public:
  bool empty() override{
    return queue_.size_approx() == 0;
  }
  void enqueue(const std::function<void()> & in_var) override{
    queue_.enqueue(in_var);
  }
  bool dequeue(std::function<void()> & out_var) override{
    return queue_.try_dequeue(out_var);
  }
  ThridQueueWrapper()= default;
 private:
  moodycamel::ConcurrentQueue<std::function<void()> >queue_;
};

class ArrayLockFreeQueueWrapper : public ThreadPool::QueueInterface{
 public:
  bool empty() override{
    return false;
  }
  void enqueue(const std::function<void()> & in_var) override{
    while(!queue_.enqueue(in_var));
  }
  bool dequeue(std::function<void()> & out_var) override{
    return queue_.dequeue(out_var);
  }
  ArrayLockFreeQueueWrapper()= default;
 private:
  ThreadPool::LockFreeQueue<std::function<void()>,kTaskNum + 100> queue_;
};
void time_consume_func(const string & method = ""){
  int id = rand() % 10000;
  std::this_thread::sleep_for(1ms);
//  std::cout << id << " " << method << "Start sleep:" << std::endl;
//    int siz = 2000;
//    std::vector<int>vec(siz,0);
//    int y = 233,z = 114514;
//    for(int i = 1;i <= 100000;i++) {
//        int x = (y * i + 10) ^ z;
//        z = x ^ y | z;
//        int idx = (i * x) % siz;
//        idx = abs(idx) % siz;
//        vec[idx] = x * z | y  + 2;
//    }
//    std::cout << "Taks finish" << " " << vec[0] << std::endl;
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
ThreadPool::ThreadPool thread_pool(std::unique_ptr<ThreadPool::QueueInterface>(
    new ThreadPool::ThreadSafeQueue<std::function<void()>>()
    ));
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
ThreadPool::ThreadPool cameron_thread_pool(std::unique_ptr<ThreadPool::QueueInterface>(
    new ThridQueueWrapper()
));
void cameron_queue_run(int n){
  std::vector<future<decltype(time_consume_func())>>futures;
  for(int i = 1;i <= n;i++){
    futures.emplace_back(cameron_thread_pool.submit(time_consume_func,"[my thread pool]"));
  }
  for(auto & it:futures){
    it.get();
  }
}


ThreadPool::ThreadPool array_queue_pool(
    std::unique_ptr<ThreadPool::QueueInterface>(
        new ArrayLockFreeQueueWrapper()
        )
    );
void array_base_lock_free_queue_run(int n){
  std::vector<future<decltype(time_consume_func())>>futures;
  for(int i = 1;i <= n;i++){
    futures.emplace_back(array_queue_pool.submit(time_consume_func,"[array based]"));
  }
  for(auto & it:futures){
    it.wait();
  }
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
static void BM_cameron_thread_pool_run(benchmark::State & state){
  for(auto _:state){
    cameron_queue_run(kTaskNum);
  }
}

static void BM_array_queue_pool_run(benchmark::State & state){
  for(auto _ : state){
    array_base_lock_free_queue_run(kTaskNum);
  }
}

void queue_test(){
  ThreadPool::LockFreeQueue<unsigned long long,500> queue;
  std::vector<std::thread>threads;
  for(int i = 1;i <= 10;i++){
    std:thread task{
      [&queue](){
        for(int tsk = 1;tsk <= 1000;tsk++){
          unsigned long long r = rand();
//          std::cout << r << std::endl;
          if(r%6 == 0){
            queue.enqueue(r);
          }
          else{
            unsigned long long x = 233;
            bool ok = queue.dequeue(x);
            if(ok){
              std::cout << x << std::endl;
            }
            else{
              std::cout << "dequeue failed" << std::endl;
            }
          }
        }
      }
    };
    threads.push_back(std::move(task));
  }
  for(auto & it:threads){
    it.join();
  }
}


BENCHMARK(BM_default_run);
BENCHMARK(BM_naive_thread_run);
BENCHMARK(BM_thread_pool_run);
BENCHMARK(BM_cameron_thread_pool_run);
BENCHMARK(BM_array_queue_pool_run);
BENCHMARK_MAIN();

//int main(){
////  return 0;
//  queue_test();
//}

