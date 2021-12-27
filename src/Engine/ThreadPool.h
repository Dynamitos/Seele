#pragma once
#include <thread>
#include <coroutine>
#include "MinimalEngine.h"
#include "Containers/List.h"
#include "Concepts.h"

namespace Seele
{
extern class ThreadPool& getGlobalThreadPool();
template<bool MainJob>
struct JobPromiseBase;
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
    bool operator==(const Event& other) const
    {
        return flag == other.flag;
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

extern std::atomic_uint64_t globalCounter;
template<bool MainJob>
struct JobBase;
template<bool MainJob>
struct JobPromiseBase
{
    enum class State
    {
        READY,
        WAITING,
        SCHEDULED,
        EXECUTING,
        DONE
    };
    JobPromiseBase()
    {
        handle = std::coroutine_handle<JobPromiseBase<MainJob>>::from_promise(*this);
        id = globalCounter++;
        finishedEvent = Event(std::format("Job {}", id));
    }
    JobBase<MainJob> get_return_object() noexcept;

    inline auto initial_suspend() noexcept;
    inline auto final_suspend() const noexcept;

    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        std::cerr << "Unhandled exception" << std::endl;
        exit(1);
    };

    void resume()
    {
        std::unique_lock lock(promiseLock);
        if(!handle || handle.done() || executing())
        {
            return;
        }
        state = State::EXECUTING;
        handle.resume();
    }
    void setContinuation(JobPromiseBase* cont)
    {
        std::unique_lock lock(promiseLock);
        std::unique_lock lock2(cont->promiseLock);
        continuation = cont->handle;
        cont->precondition = finishedEvent;
    }
    bool done()
    {
        return handle.done();
    }
    bool scheduled()
    {
        return state == State::SCHEDULED;
    }
    bool waiting()
    {
        return state == State::WAITING;
    }
    bool executing()
    {
        return state == State::EXECUTING;
    }
    bool ready()
    {
        return state == State::READY;
    }
    void raise()
    {
        std::unique_lock lock(promiseLock);
        state = State::DONE;
        finishedEvent.raise();
    }
    void reset()
    {
        std::unique_lock lock(promiseLock);
        finishedEvent.reset();
    }
    void enqueue(Event& event)
    {
        // no need to lock here, as it is only called while executing, where the lock is already held
        if(!handle || handle.done() || waiting() || scheduled())
        {
            return;
        }
        state = State::WAITING;
        getGlobalThreadPool().enqueueWaiting(event, this);
    }
    void schedule()
    {
        std::unique_lock lock(promiseLock);
        if(!handle || done() || !ready())
        {
            return;
        }
        state = (precondition == nullptr) ? State::SCHEDULED : State::WAITING;
        getGlobalThreadPool().enqueueWaiting(precondition, this);
    }

    std::mutex promiseLock;
    std::coroutine_handle<JobPromiseBase> handle;
    std::coroutine_handle<> continuation = std::noop_coroutine();
    uint64 id;
    Event finishedEvent;
    Event precondition = nullptr;
    State state = State::READY;
};

template<bool MainJob = false>
struct JobBase
{
public:
    using promise_type = JobPromiseBase<MainJob>;
    
    explicit JobBase()
        : promise(nullptr)
    {
    }
    explicit JobBase(JobPromiseBase<MainJob>* promise)
        : promise(promise)
    {
    }
    ~JobBase()
    {
        if(promise)
        {
            promise->schedule();
        }
    }
    void resume()
    {
        promise->resume();
    }
    template<std::invocable Callable>
    inline JobBase&& then(Callable callable)
    {
        return then(callable());
    }
    JobBase&& then(JobBase&& continuation)
    {
        promise->setContinuation(continuation.promise);
        return std::move(continuation);
    }
    bool done()
    {
        return promise->done();
    }
    void raise()
    {
        promise->raise();
    }
    void reset()
    {
        promise->reset();
    }
    Event operator co_await()
    {
        promise->schedule();
        return promise->finishedEvent;
    }
    static JobBase all() = delete;
    template<typename... Awaitable>
    static JobBase all(Awaitable... jobs)
    {
        (co_await jobs, ...);
    }
    template<iterable Iterable>
    static JobBase all(Iterable&& collection)
    {
        for(auto&& it : collection)
        {
            co_await it;
        }
    }
    template<std::invocable JobFunc, iterable IterableParams>
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
    JobPromiseBase<MainJob>* promise;
};

using MainJob = JobBase<true>;
using Job = JobBase<false>;
using MainPromise = JobPromiseBase<true>;
using Promise = JobPromiseBase<false>;

class ThreadPool
{
public:
    ThreadPool(uint32 threadCount = std::thread::hardware_concurrency());
    virtual ~ThreadPool();
    // Adds a job to the waiting queue for event, or directly to the job queue if event is nullptr
    void enqueueWaiting(Event& event, Promise* job);
    // Adds a job to the waiting queue for event, or directly to the job queue if event is nullptr
    void enqueueWaiting(Event& event, MainPromise* job);
    void notify(Event& event);
    void threadLoop(const bool isMainThread);
private:
    std::atomic_bool running;
    std::vector<std::thread> workers;

    List<MainPromise*> mainJobs;
    std::mutex mainJobLock;
    std::condition_variable mainJobCV;
    
    List<Promise*> jobQueue;
    std::mutex jobQueueLock;
    std::condition_variable jobQueueCV;
    
    std::map<Event, List<MainPromise*>> waitingMainJobs;
    std::mutex waitingMainLock;

    std::map<Event, List<Promise*>> waitingJobs;
    std::mutex waitingLock;
};

template<bool MainJob>
inline JobBase<MainJob> JobPromiseBase<MainJob>::get_return_object() noexcept 
{
    return JobBase<MainJob>(this);
}

template<bool MainJob>
inline auto JobPromiseBase<MainJob>::initial_suspend() noexcept
{
    return std::suspend_always{};
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
    h.promise().enqueue(*this);
}

} // namespace Seele
