#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <condition_variable>

using namespace std;

class ThreadPool
{
public:
  ThreadPool(int num_threads);
  void Enqueue(std::function<void()> task);

private:
  std::vector<std::thread> threads_;
  std::mutex queue_mutex_;
  std::queue<std::function<void()>> queue_;
  std::condition_variable queue_condition_;
};

#endif