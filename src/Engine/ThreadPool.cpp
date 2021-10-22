#include "ThreadPool.h"

using namespace Seele;


void Event::await_suspend(std::coroutine_handle<JobPromise> h)
{
    getGlobalThreadPool().addJob(Job(h));
}

Job JobPromise::get_return_object() noexcept {
    return Job { std::coroutine_handle<JobPromise>::from_promise(*this) };
}

ThreadPool::ThreadPool(uint32 threadCount)
    : workers(threadCount)
{

}

ThreadPool::~ThreadPool()
{
    for(auto& thread : workers)
    {
        thread.join();
    }
}

ThreadPool& Seele::getGlobalThreadPool()
{
    static ThreadPool threadPool;
    return threadPool;
}