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
    //Event(nullptr_t);
    Event(const std::string& name, const std::source_location& location = std::source_location::current());
    Event(const std::source_location& location = std::source_location::current());
    Event(const Event& other);
    Event(Event&& other);
    ~Event() = default;
    Event& operator=(const Event& other);
    Event& operator=(Event&& other);
    auto operator<=>(const Event& other) const
    {
        return state->name <=> other.state->name;
    }
    bool operator==(const Event& other) const
    {
        return state->name == other.state->name;
    }
    operator bool()
    {
        std::scoped_lock lock(eventLock);
        return state->data;
    }
    
    friend std::ostream& operator<<(std::ostream& stream, const Event& event)
    {
        stream 
            << event.state->location.file_name() 
            << "(" 
            << event.state->location.line() 
            << ":" 
            << event.state->location.column() 
            << "): " 
            << event.state->location.function_name();
        return stream;
    }

    void raise();
    void reset();
    bool await_ready();
    void await_suspend(std::coroutine_handle<JobPromiseBase<false>> h);
    void await_suspend(std::coroutine_handle<JobPromiseBase<true>> h);
    constexpr void await_resume() {}
private:
    mutable std::mutex eventLock;
    struct EventState
    {
        std::string name;
        std::source_location location;
        bool data = false;
        Array<JobBase<false>> waitingJobs;
        Array<JobBase<true>> waitingMainJobs;
    };
    std::shared_ptr<EventState> state;
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
        : pad0(12345)//0x7472617453)
        , handle(std::coroutine_handle<JobPromiseBase<MainJob>>::from_promise(*this))
        , waitingFor(nullptr)
        , continuation(nullptr)
        , numRefs(0)
        , finishedEvent(location)
        , state(State::READY)
        , pad1(12345)//0x646E45)
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
        std::scoped_lock lock(promiseLock);
        validate();
        if(!handle || handle.done() || executing())
        {
            return;
        }
        state = State::EXECUTING;
        handle.resume();
    }
    void setContinuation(JobPromiseBase* cont)
    {
        std::scoped_lock lock(promiseLock, cont->promiseLock);
        validate();
        assert(cont->ready());
        continuation = cont;
        cont->state = State::SCHEDULED;
        cont->waitingFor = &finishedEvent;
        cont->addRef();
    }
    bool done()
    {
        validate();
        return state == State::DONE;
    }
    bool scheduled()
    {
        validate();
        return state == State::SCHEDULED;
    }
    bool waiting()
    {
        validate();
        return state == State::WAITING;
    }
    bool executing()
    {
        validate();
        return state == State::EXECUTING;
    }
    bool ready()
    {
        validate();
        return state == State::READY;
    }
    void enqueue(Event* event);
    bool schedule();
    void addRef()
    {
        validate();
        numRefs++;
    }
    void removeRef()
    {
        {
            std::scoped_lock lock(promiseLock);
            validate();
            numRefs--;
            if(numRefs != 0)
            {
                return;
            }
            if(schedule())
            {
                return;
            }
        }
        handle.destroy();
    }
    void validate()
    {
        assert(pad0 == 12345);
        assert(pad1 == 12345);
    }
    uint64 pad0;
    std::mutex promiseLock;
    std::coroutine_handle<JobPromiseBase> handle;
    Event* waitingFor;
    JobPromiseBase* continuation;
    std::atomic_uint64_t numRefs;
    Event finishedEvent;
    State state;
    uint64 pad1;
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
        if(promise != nullptr)
        {
            promise->addRef();
        }
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
            if(promise != nullptr)
            {
                promise->addRef();
            }
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
    inline JobBase then(Callable callable)
    {
        return then(callable());
    }
    JobBase then(JobBase continuation)
    {
        promise->setContinuation(continuation.promise);
        continuation.promise->state = JobPromiseBase<MainJob>::State::SCHEDULED;
        return continuation;
    }
    bool done()
    {
        return promise->done();
    }
    Event& operator co_await()
    {
        // the co_await operator keeps a reference to this, it won't 
        // be scheduled from the destructor
        std::scoped_lock lock(promise->promiseLock);
        promise->schedule();
        return promise->finishedEvent;
    }
    static JobBase all() = delete;
    
    template<std::ranges::range Iterable>
    requires std::same_as<std::ranges::range_value_t<Iterable>, JobBase<MainJob>>
    static JobBase<MainJob> all(Iterable collection);
    
    template<std::ranges::range Iterable>
    static JobBase all(Iterable collection)
    {
        for(auto it : collection)
        {
            co_await it;
        }
    }
    
    template<typename... Awaitable>
    static JobBase all(Awaitable... jobs)
    {
        std::vector vec{jobs...};
        return std::move(JobBase::all(std::move(vec)));
    }
    template<typename JobFunc, std::ranges::input_range Iterable>
    requires std::invocable<JobFunc, std::ranges::range_reference_t<Iterable>>
    static JobBase launchJobs(JobFunc&& func, Iterable params);
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
    ThreadPool(uint32 threadCount = std::thread::hardware_concurrency());
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
            std::scoped_lock lock(job.promise->promiseLock);
            job.promise->validate();
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
            std::scoped_lock lock(job.promise->promiseLock);
            job.promise->validate();
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
    validate();
    return std::suspend_always{};
}

template<bool MainJob>
inline auto JobPromiseBase<MainJob>::final_suspend() noexcept
{
    validate();
    state = State::DONE;
    finishedEvent.raise();
    if(continuation)
    {
        getGlobalThreadPool().scheduleJob(JobBase<MainJob>(continuation));
        continuation->removeRef();
    }
    return std::suspend_always{};
}
template<bool MainJob>
inline bool JobPromiseBase<MainJob>::schedule()
{
    validate();
    if(!handle || done() || !ready())
    {
        return false;
    }
    state = State::SCHEDULED;
    getGlobalThreadPool().scheduleJob(std::move(JobBase<MainJob>(this)));
    return true;
}
template<bool MainJob>
template<std::ranges::range Iterable>
requires std::same_as<std::ranges::range_value_t<Iterable>, JobBase<MainJob>>
inline JobBase<MainJob> JobBase<MainJob>::all(Iterable collection)
{
    getGlobalThreadPool().scheduleBatch(collection);
    for(auto it : collection)
    {
        co_await it;
    }
    collection.clear();
}
template<bool MainJob>
template<typename JobFunc, std::ranges::input_range Iterable>
requires std::invocable<JobFunc, std::ranges::range_reference_t<Iterable>>
inline JobBase<MainJob> JobBase<MainJob>::launchJobs(JobFunc&& func, Iterable params)
{
    Array<JobBase<MainJob>> jobs;
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
} // namespace Seele
