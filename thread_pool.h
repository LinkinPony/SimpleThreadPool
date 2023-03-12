//
// Created by linkinpony on 23-3-11.
//

#ifndef SIMPLETHREADPOOL_THREAD_POOL_H
#define SIMPLETHREADPOOL_THREAD_POOL_H

#include <future>
#include <mutex>
#include <thread>
#include <chrono>
#include <queue>
#include <vector>
#include <functional>

namespace ThreadPool {
template<typename T>
class ThreadSafeQueue {
 public:
  size_t size();
  T &front();
  void pop();
  void push(T &in);
  void clear();
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
};
template<typename T>
void ThreadSafeQueue<T>::clear() {
  std::unique_lock<std::mutex> lock{mutex_};
  while (!queue_.empty()) {
    queue_.pop();
  }
}
template<typename T>
void ThreadSafeQueue<T>::pop() {
  std::unique_lock<std::mutex> lock{mutex_};
  queue_.pop();
}
template<typename T>
T &ThreadSafeQueue<T>::front() {
  std::unique_lock<std::mutex> lock{mutex_};
  return queue_.front();
}
template<typename T>
void ThreadSafeQueue<T>::push(T &in) {
  std::unique_lock<std::mutex> lock{mutex_};
  queue_.emplace(in);
}
template<typename T>
size_t ThreadSafeQueue<T>::size() {
  std::unique_lock<std::mutex> lock{mutex_};
  return queue_.size();
}
//extract tasks in queue and handle them. each worker is a endless thread.


class ThreadPool {
 public:
  //submit a task, return the future of this task.
  template<typename F, typename... Args>
  auto submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))>;
 public:
  explicit ThreadPool(size_t thread_num = std::thread::hardware_concurrency());
  ~ThreadPool();
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool& operator = (const ThreadPool &) = delete;
  ThreadPool& operator = (ThreadPool&&) = delete;
 public:
  [[nodiscard]] bool isRun() const { return run_; }
  //Terminate and discard all queued works
  void stop();
 public:
  std::mutex worker_mutex_;
  std::condition_variable queue_cond_;
  ThreadSafeQueue<std::function<void()>> task_queue_;
 private:
  void init();
 private:
  bool run_;
  int thread_num_;
  std::vector<std::thread> threads_;


};
class ThreadWorker {
 public:
  ThreadWorker(ThreadPool *pool, int id);
  void work();
  void operator()();
 private:
  void run();
 private:
  int id_;
  //we don't care how to manage pool's resource in worker, so just use bare pointer.
  ThreadPool *pool_;
};
template<typename F, typename... Args>
auto ThreadPool::submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
  //bind origin function into func. now we can finish task by simply call func().
  auto func = std::bind(
      std::forward<F>(f),
      std::forward<Args>(args)...
  );
  using kTaskType = std::packaged_task<decltype(f(args...))()>;
  auto task_ptr = std::make_shared<kTaskType>(func);
  //use closure to capture task pointer and wrap it into void function.
  //so we can process these task in a queue easily.
  auto closure = std::function<void()>(
      [task_ptr] {
        (*task_ptr)();
      }
  );
  task_queue_.push(closure);
  queue_cond_.notify_one();
  return task_ptr->get_future();
}
};
#endif //SIMPLETHREADPOOL_THREAD_POOL_H
