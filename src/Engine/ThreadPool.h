#pragma once
#include "Containers/Array.h"
#include "Containers/List.h"
#include <functional>
#include <thread>

namespace Seele {
class ThreadPool {
  public:
    ThreadPool(uint32 numWorkers = std::thread::hardware_concurrency() - 2);
    ~ThreadPool();
    void runAndWait(List<std::function<void()>> functions);
    void runAsync(std::function<void()> func);
  private:
    struct Task {
        uint64 numRemaining = 0;
        List<std::function<void()>> functions;
    };
    void work();
    Array<std::thread> workers;
    std::mutex taskLock;
    std::condition_variable taskCV;
    std::condition_variable completedCV;
    Task currentTask;
    bool running = true;
};
ThreadPool& getThreadPool();
} // namespace Seele