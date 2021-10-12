#include "ThreadPool.h"

using namespace Seele;

static ThreadPool gThreadPool;

Job JobPromise::get_return_object() noexcept {
    return Job { std::coroutine_handle<JobPromise>::from_promise(*this) };
}

ThreadPool::ThreadPool(uint32 threadCount)
    : workers(threadCount)
{

}

static ThreadPool& getGlobalThreadPool()
{
    return gThreadPool;
}