//
// Created by linkinpony on 23-3-12.
//

#ifndef SIMPLETHREADPOOL_LOCK_FREE_QUEUE_H_
#define SIMPLETHREADPOOL_LOCK_FREE_QUEUE_H_
#include <atomic>
#include <memory>

namespace ThreadPool{

template <typename T,size_t SIZE>
class LockFreeQueue{
 public:
  bool enqueue(const T & in_var);
  bool dequeue(T & ret_var);
  LockFreeQueue();
 private:
  //head of queue
  std::atomic_size_t head_;
  //reserved tail for data waiting to be stored
  std::atomic_size_t tail_res_;
  //the tail of **stored** data
  std::atomic_size_t tail_;
  T data_[SIZE];
};
template<typename T, size_t SIZE>
LockFreeQueue<T, SIZE>::LockFreeQueue() {
  head_ = 0;
  tail_ = 0;
  tail_res_ = 0;
  memset(data_,0,sizeof(data_));
}
template<typename T, size_t SIZE>
bool LockFreeQueue<T, SIZE>::dequeue(T &ret_var) {
  size_t cur_head = head_;
  size_t cur_tail;
  do{
    cur_tail = tail_;
    if(cur_tail == cur_head){
      return false;
    }
    //this statement must in loop!
    ret_var = data_[cur_head];
  }while(head_.compare_exchange_strong(cur_head,(cur_head + 1) % SIZE));
  return true;
}
template<typename T, size_t SIZE>
bool LockFreeQueue<T, SIZE>::enqueue(const T &in_var) {

  size_t cur_head = head_;
  size_t cur_tail = tail_res_;
  do{
    cur_head = head_;
    //queue is full
    if((cur_tail + 1) % SIZE == cur_head){
      return false;
    }
  }while(!tail_res_.compare_exchange_strong(cur_tail,(cur_tail+1) % SIZE));
  data_[cur_tail] = in_var;
  while(!tail_.compare_exchange_strong(cur_tail,(cur_tail+1)%SIZE));
  return true;
}

};
#endif //SIMPLETHREADPOOL_LOCK_FREE_QUEUE_H_
