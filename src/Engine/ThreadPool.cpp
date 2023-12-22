#include "ThreadPool.h"

using namespace Seele;

ThreadPool::ThreadPool(uint32 numWorkers)
{
    for (uint32 i = 0; i < numWorkers; ++i)
    {
        workers.add(std::thread(&ThreadPool::work, this));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock l(taskLock);
        running = false;
        taskCV.notify_all();
    }
    for (auto& worker : workers)
    {
        worker.join();
    }
}

void ThreadPool::runAndWait(List<std::function<void()>> functions)
{
    std::unique_lock l(taskLock);
    currentTask.numRemaining = functions.size();
    currentTask.functions = std::move(functions);
    taskCV.notify_all();
    
    while (currentTask.numRemaining > 0)
    {
        completedCV.wait(l);
    }
}

void ThreadPool::work()
{
    while (true)
    {
        std::unique_lock l(taskLock);
        while(currentTask.functions.empty())
        {
            taskCV.wait(l);
            if (!running)
            {
                return;
            }
        }
        auto func = std::move(currentTask.functions.front());
        currentTask.functions.popFront();
        l.unlock();
        func();
        l.lock();
        currentTask.numRemaining--;
        if (currentTask.numRemaining == 0)
        {
            completedCV.notify_one();
        }
    }
}
