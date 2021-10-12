#pragma once
#include <thread>
#include <coroutine>
#include "MinimalEngine.h"

namespace Seele
{
enum class JobStage
{
    INPUT,
    GAMELOGIC,
    RENDER,
    DONT_CARE
};
struct [[nodiscard]] Job;
struct JobPromise
{
    Job get_return_object() noexcept;

    std::suspend_never initial_suspend() const noexcept { return {}; }
    std::suspend_never final_suspend() const noexcept { return {}; }

    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        std::cerr << "Unhandled exception caught" << std::endl;
        exit(1);
    };
};
struct [[nodiscard]] Job
{
public:
    using promise_type = JobPromise;
    
    explicit Job(std::coroutine_handle<task_promise> handle, JobStage stage = JobStage::DONT_CARE)
        : handle(handle)
        , stage(stage)
    {}
    ~Job()
    {
        if(handle)
        {
            handle.destroy();
        }
    }
private:
    std::coroutine_handle<JobPromise> handle;
    JobStage stage;
};

class ThreadPool
{
public:
    ThreadPool(uint32 threadCount = std::thread::hardware_concurrency);
    virtual ~ThreadPool();
    void schedule(std::function<void()> function)
    {

    }
private:
    Array<std::thread> workers;
    void threadLoop();
};
static ThreadPool& getGlobalThreadPool();
} // namespace Seele
