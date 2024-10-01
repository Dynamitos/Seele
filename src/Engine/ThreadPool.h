#pragma once
#include "Containers/Array.h"
#include "Containers/List.h"
#include <functional>
#include <thread>

namespace Seele {
class ThreadPool {
  public:
    ThreadPool(uint32 numWorkers = 1);
    ~ThreadPool();
    void runAndWait(List<std::function<void()>> functions);
    void runAsync(std::function<void()> func);
    void waitIdle();
  private:
    struct TaskGroup {
        uint64 numRemaining = 0;
    };
    struct QueueEntry {
        std::function<void()> func;
        TaskGroup* task = nullptr;
    };
    
    std::mutex queueLock;
    std::condition_variable queueCV;
    std::atomic_uint32_t numWaiting = 0;
    std::condition_variable idleCV;
    List<QueueEntry> queue;
    
    void work();
    Array<std::thread> workers;

    std::mutex taskLock;
    std::condition_variable completedCV;
    List<TaskGroup> runningTasks;
    bool running = true;
};
ThreadPool& getThreadPool();
} // namespace Seele