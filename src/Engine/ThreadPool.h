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
    std::suspend_never final_suspend() const noexcept { return {}; }

    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        std::cerr << "Unhandled exception caught" << std::endl;
        exit(1);
    };
};

struct Event
{
public:
    Event();
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
    {
        //std::cout << "Creating job " << id << std::endl;
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
            //std::cout << "Destroying job " << id << std::endl;
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
private:
    std::coroutine_handle<promise_type> handle;
    uint64 id;
};

using MainJob = JobBase<true>;
using Job = JobBase<false>;

class ThreadPool
{
public:
    ThreadPool(uint32 threadCount = std::thread::hardware_concurrency() + 1);
    virtual ~ThreadPool();
    void addJob(Job&& job);
    void addJob(MainJob&& job);
    void enqueueWaiting(Event& event, Job job);
    void enqueueWaiting(Event& event, MainJob job);
    void notify(Event& event);
private:
    std::atomic_bool running;
    std::thread* mainThread;
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
    void threadLoop(const bool isMainThread);
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
inline constexpr void Event::await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h)
{
    getGlobalThreadPool().enqueueWaiting(*this, std::move(JobBase<MainJob>(h)));
}

} // namespace Seele
