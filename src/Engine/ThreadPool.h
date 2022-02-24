#pragma once
#include <thread>
#include <coroutine>
#include "MinimalEngine.h"
#include "Containers/List.h"
#include "Concepts.h"
#include <source_location>

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
    ~Event() = default;
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
        std::scoped_lock lock(flag->lock);
        return flag->data;
    }
    
    void raise();
    void reset();
    bool await_ready();
    template<bool MainJob>
    constexpr void await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h);
    constexpr void await_resume() {}
private:
    std::string name;
    struct StateStore
    {
        std::mutex lock;
        bool data;
    };
    std::shared_ptr<StateStore> flag;
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
    JobPromiseBase(const std::source_location& location = std::source_location::current())
    {
        handle = std::coroutine_handle<JobPromiseBase<MainJob>>::from_promise(*this);
        id = globalCounter++;
        finishedEvent = Event(std::string(location.file_name()).append(": ").append(location.function_name()));
    }
    ~JobPromiseBase()
    {}
    JobBase<MainJob> get_return_object() noexcept;

    inline auto initial_suspend() noexcept;
    inline auto final_suspend() noexcept;

    void return_void() noexcept {}
    void unhandled_exception() noexcept {
        std::cerr << "Unhandled exception! Exiting" << std::endl;
        exit(1);
    };

    void resume()
    {
        std::scoped_lock lock(promiseLock);
        if(!handle || handle.done() || executing())
        {
            return;
        }
        state = State::EXECUTING;
        handle.resume();
    }
    void markDone() 
    {
        state = State::DONE;
        finishedEvent.raise();
        if(continuation)
        {
            std::scoped_lock lock(continuation->promiseLock);
            getGlobalThreadPool().scheduleJob(continuation);
            continuation->removeRef();
        }
    }
    void setContinuation(JobPromiseBase* cont)
    {
        std::scoped_lock lock(promiseLock, cont->promiseLock);
        assert(cont->ready());
        continuation = cont;
        cont->state = State::SCHEDULED;
        cont->addRef();
    }
    bool done()
    {
        return state == State::DONE;
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
    void reset()
    {
        std::scoped_lock lock(promiseLock);
        finishedEvent.reset();
    }
    void enqueue(Event& event)
    {
        if(!handle || handle.done() || waiting() || scheduled())
        {
            return;
        }
        state = State::WAITING;
        getGlobalThreadPool().enqueueWaiting(event, this);
    }
    void schedule()
    {
        std::scoped_lock lock(promiseLock);
        if(!handle || done() || !ready())
        {
            return;
        }
        state = State::SCHEDULED;
        getGlobalThreadPool().scheduleJob(this);
    }
    void addRef()
    {
        numRefs++;
    }
    void removeRef()
    {
        if(--numRefs < 1 && done())
        {
            handle.destroy();
        }
    }

    std::mutex promiseLock;
    std::coroutine_handle<JobPromiseBase> handle;
    JobPromiseBase* continuation = nullptr;
    uint64 id;
    std::atomic_uint64_t numRefs = 0;
    Event finishedEvent;
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
    JobBase(const JobBase& other)
    {
        std::scoped_lock lock(other.promise->promiseLock);
        promise = other.promise;
        promise->addRef();
    }
    JobBase(JobBase&& other)
    {
        std::scoped_lock lock(other.promise->promiseLock);
        promise = other.promise;
        other.promise = nullptr;
    }
    ~JobBase()
    {
        if(promise)
        {
            promise->schedule();
            promise->removeRef();
        }
    }
    JobBase& operator=(const JobBase& other)
    {
        if(this != &other)
        {
            std::scoped_lock lock(other.promise->promiseLock);
            promise = other.promise;
            promise->addRef();
        }
        return *this;
    }
    JobBase& operator=(JobBase&& other)
    {
        if(this != &other)
        {
            std::scoped_lock lock(other.promise->promiseLock);
            promise = other.promise;
            other.promise = nullptr;
        }
        return *this;
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
        promise->schedule();
        return std::move(continuation);
    }
    bool done()
    {
        return promise->done();
    }
    void finalize()
    {
        promise->finalize();
    }
    Event operator co_await() const
    {
        // the co_await operator keeps a reference to this, we it won't 
        // be scheduled from the destructor
        promise->schedule();
        return promise->finishedEvent;
    }
    static JobBase all() = delete;
    template<iterable Iterable>
    static JobBase all(Iterable collection)
    {
        for(auto it : collection)
        {
            co_await it;
        }
    }
    template<typename... Awaitable>
    static JobBase all(Awaitable&&... jobs)
    {
        return JobBase::all(Array{jobs...});
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
    void cleanup();
    // Adds a job to the waiting queue for event
    void enqueueWaiting(Event& event, Promise* job);
    // Adds a job to the waiting queue for event
    void enqueueWaiting(Event& event, MainPromise* job);
    void scheduleJob(Promise* job);
    void scheduleJob(MainPromise* job);
    void notify(Event& event);
    void threadLoop(const bool isMainThread);
private:
    std::atomic_bool running;
    std::vector<std::thread> workers;

    std::list<MainPromise*> mainJobs;
    std::mutex mainJobLock;
    std::condition_variable mainJobCV;
    
    std::list<Promise*> jobQueue;
    std::mutex jobQueueLock;
    std::condition_variable jobQueueCV;
    
    std::map<Event, std::list<MainPromise*>> waitingMainJobs;
    std::mutex waitingMainLock;

    std::map<Event, std::list<Promise*>> waitingJobs;
    std::mutex waitingLock;
};

template<bool MainJob>
inline JobBase<MainJob> JobPromiseBase<MainJob>::get_return_object() noexcept 
{
    numRefs++;
    return JobBase<MainJob>(this);
}

template<bool MainJob>
inline auto JobPromiseBase<MainJob>::initial_suspend() noexcept
{
    return std::suspend_always{};
}

template<bool MainJob>
inline auto JobPromiseBase<MainJob>::final_suspend() noexcept
{
    /*struct JobAwaitable
    {
        constexpr bool await_ready() const noexcept { return false; }
        constexpr auto await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h) const noexcept
        {
            auto continuation = h.promise().continuation;
            h.destroy();

        }
        constexpr void await_resume() const noexcept {}
    };
    return JobAwaitable{};*/
    markDone();
    return std::suspend_always{};
}

template<bool MainJob>
inline constexpr void Event::await_suspend(std::coroutine_handle<JobPromiseBase<MainJob>> h)
{
    h.promise().enqueue(*this);
    flag->lock.unlock();
}

} // namespace Seele
