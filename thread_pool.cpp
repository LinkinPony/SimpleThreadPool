
//
// Created by linkinpony on 23-3-11.
//

#include <cassert>
#include "thread_pool.h"
void ThreadPool::ThreadWorker::work() {
  std::unique_lock<std::mutex> lock(pool_->worker_mutex_);
  pool_->queue_cond_.wait(lock,
                          [this]() ->
                              bool {
                            return pool_->task_queue_.size() > 0 || !pool_->isRun();
                          }
  );
  if(!pool_->isRun()){
    return;
  }
  //TODO: delete this
  assert(pool_->task_queue_.size() > 0);
  auto work = pool_->task_queue_.front();
  pool_->task_queue_.pop();
  work();
}
void ThreadPool::ThreadWorker::run() {
  while (pool_->isRun()) {
    work();
  }
}
ThreadPool::ThreadWorker::ThreadWorker(ThreadPool *pool, int id) : pool_(pool), id_(id) {

}
void ThreadPool::ThreadWorker::operator()() {
  run();
}
ThreadPool::ThreadPool::ThreadPool(size_t thread_num) : thread_num_(thread_num) {
  run_ = true;
  init();
}
void ThreadPool::ThreadPool::init() {
  for (int i = 0; i < thread_num_; i++) {
    threads_.emplace_back(std::thread{ThreadWorker{this, i}});
  }
}
ThreadPool::ThreadPool::~ThreadPool() {
  stop();
}
void ThreadPool::ThreadPool::stop() {
  run_ = false;
  queue_cond_.notify_all();
  for(auto & it:threads_){
    if(it.joinable()){
      it.join();
    }
  }
}
