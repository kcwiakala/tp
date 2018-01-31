#ifndef IZI_THREAD_POOL_HPP
#define IZI_THREAD_POOL_HPP

#include <functional>
#include <future>
#include <mutex>
#include <queue>

#include <iostream>

namespace izi {

class ThreadPool 
{
public:
  ThreadPool(size_t size): _stop(false)
  {
    while(size--)
    {
      _pool.emplace_back(&ThreadPool::run, this);
    }
  }

  template<class F, class...Args>
  auto enqueue(F&& f, Args&&...args) -> std::future<std::result_of_t<F(Args...)>>
  {
    using R = std::result_of_t<F(Args...)>;
    using pkg_task_t = std::packaged_task<R()>;

    auto task = std::make_shared<pkg_task_t>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto fut = task->get_future();
    {
      std::unique_lock<std::mutex> lock(_mtx);
      if(_stop)
      {
        throw std::runtime_error("ThreadPool already stopped");
      }
      _tasks.emplace([task = std::move(task)]{
        task->operator()();
      });
    }
    _cv.notify_one();
    return fut;
  }

  ~ThreadPool()
  {
    {
      std::unique_lock<std::mutex> lock(_mtx);
      _stop = true;
    }
    _cv.notify_all();
    for(std::thread& thread: _pool)
    {
      thread.join();
    }
  }

private:
  void run()
  {
    for(;;)
    {
      std::unique_lock<std::mutex> lock(_mtx);
      _cv.wait(lock, [this](){
        return _stop || !_tasks.empty();
      });
      if(_stop) 
      {
        break;
      }
      auto task = std::move(_tasks.front());
      _tasks.pop();
      lock.unlock();
      task();
    }
  }

private:
  using task_t = std::function<void()>;

  std::vector<std::thread> _pool;
  std::queue<task_t> _tasks;
  std::condition_variable _cv;
  std::mutex _mtx;
  bool _stop;
};

} // namespace izi

#endif // IZI_THREAD_POOL_HPP