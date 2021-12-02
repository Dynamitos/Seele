#pragma once
#include <thread>
#include <coroutine>
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
    inline auto final_suspend() const noexcept;

    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        std::cerr << "Unhandled exception" << std::endl;
        exit(1);
    };
    std::coroutine_handle<> continuation;
};

struct Event
{
public:
    Event();
    Event(nullptr_t);
    Event(const std::string& name);
    auto operator<=>(const Event& other) const
    {
        return flag <=> other.flag;
    }
    Event operator co_await()
    {
        return *this;
    }
    operator bool()
    {
        return flag->load();
    }
    
    void raise();
    void reset();
    bool await_ready();
    template<bool MainJob>
    constexpr void await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h);
    constexpr void await_resume() {}
private:
    std::string name;
    RefPtr<std::atomic_bool> flag;
    friend class ThreadPool;
};


static std::atomic_uint64_t globalCounter;
template<bool MainJob>
struct JobBase
{
public:
    using promise_type = JobPromiseBase<MainJob>;
    
    explicit JobBase()
        : id(-1)
    {}
    explicit JobBase(std::coroutine_handle<promise_type> handle)
        : handle(handle)
        , id(globalCounter++)
        , event(std::format("Job {}", id))
    {
    }
    JobBase(const JobBase & rhs) = delete;
    JobBase(JobBase&& rhs)
        : handle(std::move(rhs.handle))
        , id(std::move(rhs.id))
    {
        rhs.id = -1;
        rhs.handle = nullptr;
    }
    ~JobBase()
    {
        if(handle && handle.done()) 
        {
            handle.destroy();
        }
    }
    JobBase& operator=(const JobBase& rhs) = delete;
    JobBase& operator=(JobBase&& rhs)
    {
        if(this != &rhs)
        {
            handle = std::move(rhs.handle);
            id = std::move(rhs.id);
            rhs.id = -1;
            rhs.handle = nullptr;
        }
        return *this;
    }
    void resume()
    {
        handle.resume();
    }
    void then(JobBase continuation)
    {
        handle.promise().continuation = continuation.handle;
    }
    bool done()
    {
        return handle.done();
    }
    void raise()
    {
        event.raise();
    }
    void reset()
    {
        event.reset();
    }
    Event operator co_await()
    {
        return event;
    }
    template<typename... Awaitable>
    static JobBase all(Awaitable... jobs)
    {
        co_await jobs;
    }
    template<typename Iterable>
    static JobBase all(Iterable&& collection)
    {
        for(auto&& it : collection)
        {
            co_await it;
        }
    }
    template<typename JobFunc, typename IterableParams>
    static JobBase launchJobs(JobFunc&& func, IterableParams params)
    {
        List<JobBase> jobs;
        for(auto&& param : params)
        {
            jobs.add(func(param));
        }
        for(auto job : jobs)
        {
            co_await job;
        }
    }
private:
    std::coroutine_handle<promise_type> handle;
    Event event;
    uint64 id;
};

using MainJob = JobBase<true>;
using Job = JobBase<false>;

class ThreadPool
{
public:
    ThreadPool(uint32 threadCount = std::thread::hardware_concurrency());
    virtual ~ThreadPool();
    void addJob(Job&& job);
    void addJob(MainJob&& job);
    void enqueueWaiting(Event& event, Job&& job);
    void enqueueWaiting(Event& event, MainJob&& job);
    void notify(Event& event);
    void threadLoop(const bool isMainThread);
private:
    std::atomic_bool running;
    std::vector<std::thread> workers;

    List<MainJob> mainJobs;
    std::mutex mainJobLock;
    std::condition_variable mainJobCV;
    
    List<Job> jobQueue;
    std::mutex jobQueueLock;
    std::condition_variable jobQueueCV;
    
    Map<Event, List<Job>> waitingJobs;
    std::mutex waitingLock;
    
    Map<Event, List<MainJob>> waitingMainJobs;
    std::mutex waitingMainLock;
    
    void tryMainJob();
};


template<bool MainJob>
inline JobBase<MainJob> JobPromiseBase<MainJob>::get_return_object() noexcept {
    return JobBase<MainJob>();
}

template<bool MainJob>
inline auto JobPromiseBase<MainJob>::initial_suspend() const noexcept
{
    struct JobAwaitable
    {
        constexpr bool await_ready() { return false; }
        constexpr void await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h)
        {
            getGlobalThreadPool().addJob(std::move(JobBase<MainJob>(h)));
        }
        constexpr void await_resume() {}
    };
    return JobAwaitable{};
}
template<bool MainJob>
inline auto JobPromiseBase<MainJob>::final_suspend() const noexcept
{
    struct JobAwaitable
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr auto await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h) const noexcept
        {
            return h.promise().continuation;
        }
        constexpr void await_resume() const noexcept {}
    };
    return JobAwaitable{};
}

template<bool MainJob>
inline constexpr void Event::await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h)
{
    getGlobalThreadPool().enqueueWaiting(*this, std::move(JobBase<MainJob>(h)));
}

} // namespace Seele
