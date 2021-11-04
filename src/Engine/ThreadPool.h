#pragma once
#include <thread>
#include "MinimalEngine.h"
#include "Containers/List.h"

namespace Seele
{
extern class ThreadPool& getGlobalThreadPool();

template<bool MainJob>
struct JobBase;

template<bool MainJob>
struct JobPromiseBase
{
    JobBase<MainJob> get_return_object() noexcept;

    inline auto initial_suspend() const noexcept;
    std::suspend_never final_suspend() const noexcept { return {}; }

    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        std::cerr << "Unhandled exception caught" << std::endl;
        exit(1);
    };
};
template<bool MainJob>
struct JobBase
{
public:
    using promise_type = JobPromiseBase<MainJob>;
    
    explicit JobBase(std::coroutine_handle<promise_type> handle)
        : handle(handle)
    {}
    ~JobBase()
    {
        if(handle)
        {
            handle.destroy();
        }
    }
    void resume()
    {
        handle.resume();
    }
private:
    std::coroutine_handle<promise_type> handle;
};

using MainJob = JobBase<true>;
using Job = JobBase<false>;

struct Event
{
public:
    Event();
    Event(const std::string& name);
    void raise();
    void reset();
    bool await_ready();
    template<bool MainJob>
    constexpr void await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h);
    constexpr void await_resume() {}
private:
    std::string name;
    RefPtr<std::atomic_bool> flag;
};

class ThreadPool
{
public:
    ThreadPool(uint32 threadCount = std::thread::hardware_concurrency());
    virtual ~ThreadPool();
    void addJob(Job&& job);
    void addJob(MainJob&& job);
    void enqueueWaiting(Event* event, Job&& job);
    void enqueueWaiting(Event* event, MainJob&& job);
    void notify(Event* event);
private:
    std::atomic_bool running;
    std::thread* mainThread;
    std::vector<std::thread> workers;

    List<MainJob> mainJobs;
    std::mutex mainJobLock;
    
    List<Job> jobQueue;
    std::mutex jobQueueLock;
    
    Map<Event*, List<Job>> waitingJobs;
    std::mutex waitingLock;
    
    Map<Event*, List<MainJob>> waitingMainJobs;
    std::mutex waitingMainLock;
    
    void threadLoop(const bool isMainThread);
};


template<bool MainJob>
inline JobBase<MainJob> JobPromiseBase<MainJob>::get_return_object() noexcept {
    return JobBase<MainJob> { std::coroutine_handle<JobPromiseBase<MainJob>>::from_promise(*this) };
}

template<bool MainJob>
inline auto JobPromiseBase<MainJob>::initial_suspend() const noexcept
{
    struct JobAwaitable
    {
        constexpr bool await_ready() { return false; }
        constexpr void await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h)
        {
            getGlobalThreadPool().addJob(JobBase<MainJob>(h));
        }
        constexpr void await_resume() {}
    };
    return JobAwaitable{};
}

template<bool MainJob>
inline constexpr void Event::await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h)
{
    getGlobalThreadPool().enqueueWaiting(this, JobBase<MainJob>(h));
}

} // namespace Seele
