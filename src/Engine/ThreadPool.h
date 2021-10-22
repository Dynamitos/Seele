#pragma once
#include <thread>
#include <coroutine>
#include "Containers/List.h"

namespace Seele
{
struct JobPromise;
struct Event
{
public:
    void raise()
    {
        flag.test_and_set();
        flag.notify_all();
    }
    void reset()
    {
        flag.clear();
    }
    bool await_ready() { return flag.test(); }
    void await_suspend(std::coroutine_handle<JobPromise> h);
    void await_resume() {}
private:
    std::atomic_flag flag;
};
struct [[nodiscard]] Job;
struct JobPromise
{
    Job get_return_object() noexcept;

    std::suspend_always initial_suspend() const noexcept { return {}; }
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
    
    explicit Job(std::coroutine_handle<JobPromise> handle)
        : handle(handle)
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
};

class ThreadPool
{
public:
    ThreadPool(uint32 threadCount = std::thread::hardware_concurrency());
    virtual ~ThreadPool();
    void addJob(Job&& job)
    {
        jobs.add(std::move(job));
    }
private:
    std::vector<std::thread> workers;
    List<Job> jobs;
    void threadLoop();
};
extern ThreadPool& getGlobalThreadPool();
} // namespace Seele
