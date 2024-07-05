#include "ThreadPool.h"
#include "MinimalEngine.h"

using namespace Seele;

Globals globals;

Globals& Seele::getGlobals() { return globals; }

ThreadPool::ThreadPool(uint32 numWorkers) {
    for (uint32 i = 0; i < numWorkers; ++i) {
        workers.add(std::thread(&ThreadPool::work, this));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock l(taskLock);
        running = false;
        taskCV.notify_all();
    }
    for (auto& worker : workers) {
        worker.join();
    }
}

void ThreadPool::runAndWait(List<std::function<void()>> functions) {
    std::unique_lock l(taskLock);
    currentTask.numRemaining = functions.size();
    currentTask.functions = std::move(functions);
    taskCV.notify_all();

    while (currentTask.numRemaining > 0) {
        completedCV.wait(l);
    }
}

void ThreadPool::work() {
    while (running) {
        std::unique_lock l(taskLock);
        while (currentTask.functions.empty()) {
            taskCV.wait(l);
            if (!running) {
                return;
            }
        }
        auto func = std::move(currentTask.functions.front());
        currentTask.functions.popFront();
        l.unlock();
        func();
        l.lock();
        currentTask.numRemaining--;
        if (currentTask.numRemaining == 0) {
            currentTask.functions.clear();
            completedCV.notify_one();
        }
    }
}

static ThreadPool threadPool;

ThreadPool& Seele::getThreadPool() { return threadPool; }
