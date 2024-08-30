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
        std::unique_lock l(queueLock);
        running = false;
        queueCV.notify_all();
    }
    for (auto& worker : workers) {
        worker.join();
    }
}

void ThreadPool::runAndWait(List<std::function<void()>> functions) {
    std::unique_lock l(taskLock);
    auto newTask = runningTasks.add();
    newTask->numRemaining = functions.size();
    {
        std::unique_lock q(queueLock);
        while (!functions.empty()) {
            queue.add(QueueEntry{
                .func = std::move(functions.back()),
                .task = &*newTask,
            });
            functions.popBack();
        }
        queueCV.notify_all();
    }
    while (newTask->numRemaining > 0) {
        completedCV.wait(l);
    }
    runningTasks.remove(newTask);
}

void ThreadPool::runAsync(std::function<void()> func) {
    std::unique_lock l(queueLock);
    queue.add(QueueEntry{
        .func = std::move(func),
        .task = nullptr,
    });
    queueCV.notify_one();
}

void ThreadPool::waitIdle() {
    std::unique_lock l(queueLock);
    while (numWaiting < workers.size())
    {
        idleCV.wait(l);
    }
}

void ThreadPool::work() {
    while (running) {
        std::unique_lock l(queueLock);
        while (queue.empty()) {
            numWaiting++;
            if (numWaiting == workers.size())
            {
                idleCV.notify_all();
            }
            queueCV.wait(l);
            numWaiting--;
            if (!running) {
                return;
            }
        }
        auto entry = std::move(queue.front());
        queue.popFront();
        l.unlock();
        entry.func();
        l.lock();
        if (entry.task != nullptr) {
            std::unique_lock t(taskLock);
            entry.task->numRemaining--;
            if (entry.task->numRemaining == 0) {
                completedCV.notify_all();
            }
        }
    }
}

static ThreadPool threadPool;

ThreadPool& Seele::getThreadPool() { return threadPool; }
