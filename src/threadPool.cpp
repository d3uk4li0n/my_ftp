#include "../include/threadPool.hpp"

ThreadPool::ThreadPool(int num_threads){
  threads_.reserve(num_threads);
  for (int i = 0; i < num_threads; i++){
    threads_.emplace_back([this] {
      while (true){
        std::function<void()> task;{
          std::unique_lock<std::mutex> lock(queue_mutex_);
          while (queue_.empty()){
            queue_condition_.wait(lock);
          }
          task = std::move(queue_.front());
          queue_.pop();
        }

        task();
      }
    });
  }
}

void ThreadPool::Enqueue(std::function<void()> task){
  std::lock_guard<std::mutex> lock(queue_mutex_);
  queue_.push(std::move(task));
  queue_condition_.notify_one();
}