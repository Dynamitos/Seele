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
struct JobBase;
template<bool MainJob>
struct JobPromiseBase;
struct Event
{
public:
    Event(nullptr_t);
    Event(const std::string& name, const std::source_location& location = std::source_location::current());
    Event(const std::source_location& location = std::source_location::current());
    Event(const Event& other) = delete;
    Event(Event&& other) = default;
    ~Event() = default;
    Event& operator=(const Event& other) = delete;
    Event& operator=(Event&& other) = default;
    auto operator<=>(const Event& other) const
    {
        return name <=> other.name;
    }
    bool operator==(const Event& other) const
    {
        return name == other.name;
    }
    operator bool()
    {
        std::scoped_lock lock(eventLock);
        return data;
    }
    
    friend std::ostream& operator<<(std::ostream& stream, const Event& event)
    {
        stream 
            << event.location.file_name() 
            << "(" 
            << event.location.line() 
            << ":" 
            << event.location.column() 
            << "): " 
            << event.location.function_name();
        return stream;
    }

    void raise();
    void reset();
    bool await_ready();
    void await_suspend(std::coroutine_handle<JobPromiseBase<false>> h);
    void await_suspend(std::coroutine_handle<JobPromiseBase<true>> h);
    constexpr void await_resume() {}
private:
    std::mutex eventLock;
    std::string name;
    std::source_location location;
    bool data = false;
    Array<JobBase<false>> waitingJobs;
    Array<JobBase<true>> waitingMainJobs;
    friend class ThreadPool;
};

extern std::mutex promisesLock;
extern List<JobPromiseBase<false>*> promises;
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
        : handle(std::coroutine_handle<JobPromiseBase<MainJob>>::from_promise(*this))
        , finishedEvent(Event(location))
    {
        if constexpr(!MainJob)
        {
            std::scoped_lock lock(promisesLock);
            promises.add(this);
        }
    }
    ~JobPromiseBase()
    {
        if constexpr (!MainJob)
        {
            std::scoped_lock lock(promisesLock);
            promises.remove(promises.find(this));
        }
    }

    JobBase<MainJob> get_return_object() noexcept;

    inline auto initial_suspend() noexcept;
    inline auto final_suspend() noexcept;

    void return_void() noexcept {}
    void unhandled_exception() {
        std::exception_ptr exception = std::current_exception();
        std::cerr << "Unhandled exception!" << std::endl;
        throw exception;
    };

    void resume()
    {
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
            getGlobalThreadPool().scheduleJob(JobBase<MainJob>(continuation));
            continuation->removeRef();
        }
    }
    void setContinuation(JobPromiseBase* cont)
    {
        assert(cont->ready());
        continuation = cont;
        cont->state = State::SCHEDULED;
        cont->waitingFor = &finishedEvent;
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
    void enqueue(Event* event)
    {
        if(!handle || handle.done() || waiting() || scheduled())
        {
            return;
        }
        state = State::WAITING;
        waitingFor = event;
        getGlobalThreadPool().enqueueWaiting(event, std::move(JobBase<MainJob>(this)));
    }
    bool schedule()
    {
        if(!handle || done() || !ready())
        {
            return false;
        }
        state = State::SCHEDULED;
        getGlobalThreadPool().scheduleJob(std::move(JobBase<MainJob>(this)));
        return true;
    }
    void addRef()
    {
        numRefs++;
    }
    void removeRef()
    {
        numRefs--;
        if(numRefs == 0)
        {
            if(!schedule())
            {
                handle.destroy();
            }
        } 
    }
    uint64 pad0 = 0x7472617453;
    std::coroutine_handle<JobPromiseBase> handle;
    Event* waitingFor = nullptr;
    JobPromiseBase* continuation = nullptr;
    uint64 numRefs = 0;
    Event finishedEvent;
    State state = State::READY;
    uint64 pad1 = 0x646E45;
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
        promise->addRef();
    }
    JobBase(const JobBase& other)
    {
        promise = other.promise;
        promise->addRef();
    }
    JobBase(JobBase&& other)
    {
        promise = other.promise;
        other.promise = nullptr;
    }
    ~JobBase()
    {
        if(promise)
        {
            //promise->schedule();
            promise->removeRef();
        }
    }
    JobBase& operator=(const JobBase& other)
    {
        if(this != &other)
        {
            if(promise != nullptr)
            {
                promise->removeRef();
            }
            promise = other.promise;
            promise->addRef();
        }
        return *this;
    }
    JobBase& operator=(JobBase&& other)
    {
        if(this != &other)
        {
            if(promise != nullptr)
            {
                promise->removeRef();
            }
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
    Event& operator co_await()
    {
        // the co_await operator keeps a reference to this, it won't 
        // be scheduled from the destructor
        promise->schedule();
        return promise->finishedEvent;
    }
    static JobBase all() = delete;
    template<std::ranges::range Iterable>
    requires std::same_as<std::ranges::range_value_t<Iterable>, JobBase>
    static JobBase all(Iterable collection)
    {
        getGlobalThreadPool().scheduleBatch(collection);
        for(auto it : collection)
        {
            co_await it;
        }
    }
    template<std::ranges::range Iterable>
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
    template<typename JobFunc, std::ranges::input_range Iterable>
    requires std::invocable<JobFunc, std::ranges::range_reference_t<Iterable>>
    static JobBase launchJobs(JobFunc&& func, Iterable params)
    {
        Array<JobBase> jobs;
        for(auto&& param : params)
        {
            jobs.add(func(param));
        }
        getGlobalThreadPool().scheduleBatch(jobs);
        for(auto job : jobs)
        {
            co_await job;
        }
    }
private:
    JobPromiseBase<MainJob>* promise;
    friend class ThreadPool;
};

using MainJob = JobBase<true>;
using Job = JobBase<false>;
using MainPromise = JobPromiseBase<true>;
using Promise = JobPromiseBase<false>;

class ThreadPool
{
public:
    ThreadPool(uint32 threadCount = 1);//std::thread::hardware_concurrency());
    virtual ~ThreadPool();
    void waitIdle();
    void scheduleJob(Job job);
    void scheduleJob(MainJob job);
    template<std::ranges::range Iterable>
    requires std::same_as<std::ranges::range_value_t<Iterable>, MainJob>
    void scheduleBatch(Iterable jobs)
    {
        std::scoped_lock lock(mainJobLock);
        for(auto job : jobs)
        {
            //job.promise->addRef();
            job.promise->state = JobPromiseBase<true>::State::SCHEDULED;
            mainJobs.add(job);
        }
        mainJobCV.notify_one();
    }
    template<std::ranges::range Iterable>
    requires std::same_as<std::ranges::range_value_t<Iterable>, Job>
    void scheduleBatch(Iterable jobs)
    {
        std::scoped_lock lock(jobQueueLock);
        for(auto job : jobs)
        {
            //job.promise->addRef();
            job.promise->state = JobPromiseBase<false>::State::SCHEDULED;
            jobQueue.add(job);
        }
        jobQueueCV.notify_all();
    }
    void notify(Event* event);
    void mainLoop();
    void threadLoop();
private:
    std::atomic_bool running;
    std::mutex numIdlingLock;
    std::condition_variable numIdlingIncr;
    uint32 numIdling;
    Array<std::thread> workers;

    List<MainJob> mainJobs;
    std::mutex mainJobLock;
    std::condition_variable mainJobCV;
    
    List<Job> jobQueue;
    std::mutex jobQueueLock;
    std::condition_variable jobQueueCV;

    uint32 localQueueSize = 50;
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
inline auto JobPromiseBase<MainJob>::final_suspend() noexcept
{
    markDone();
    return std::suspend_always{};
}

} // namespace Seele
