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
#include <iostream>

namespace ThreadPool {

class QueueInterface {
 public:
  virtual bool empty() = 0;
  virtual bool dequeue(std::function<void()> &ret_var) = 0;
  virtual void enqueue(const std::function<void()> &in_var) = 0;
  virtual ~QueueInterface() = default;
};
template<typename T>
class ThreadSafeQueue : public QueueInterface {
 public:
  bool empty() override;
  bool dequeue(T &ret_var) override;
  void enqueue(const T &in_var) override;
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
};

template<typename T>
void ThreadSafeQueue<T>::enqueue(const T &in_var) {
  std::unique_lock<std::mutex> lock(mutex_);
  queue_.push(in_var);
}
template<typename T>
bool ThreadSafeQueue<T>::dequeue(T &ret_var) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!queue_.empty()) {
    ret_var = queue_.front();
    queue_.pop();
    return true;
  }
  return false;
}
template<typename T>
bool ThreadSafeQueue<T>::empty() {
  std::unique_lock<std::mutex> lock(mutex_);
  return queue_.empty();
}

class ThreadPool {
 public:
  //submit a task, return the future of this task.
  template<typename F, typename... Args>
  auto submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))>;
 public:
  explicit ThreadPool(std::unique_ptr<QueueInterface> task_queue,
                      size_t thread_num = std::thread::hardware_concurrency());
  ~ThreadPool();
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;
 public:
  [[nodiscard]] bool isRun() const { return run_; }
  //Terminate and discard all queued works
  void stop();
 public:
  std::mutex worker_mutex_;
  std::condition_variable queue_cond_;
  std::unique_ptr<QueueInterface> task_queue_;
 private:
  void init();
 private:
  bool run_;
  int thread_num_;
  std::vector<std::thread> threads_;
};
//extract tasks in queue and handle them. each worker is a endless thread.
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
  auto res = task_ptr->get_future();
//  std::cout << threads_.size() << std::endl;
  task_queue_->enqueue(closure);
  queue_cond_.notify_one();
  return res;
}
};
#endif //SIMPLETHREADPOOL_THREAD_POOL_H
