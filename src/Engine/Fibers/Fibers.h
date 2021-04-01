#pragma once
#include "MinimalEngine.h"
#include "JobQueue.h"

namespace Seele
{
namespace Fibers
{
class Counter
{
public:
    Counter();
    Counter(uint64 initialValue);
    ~Counter();
    inline void increment()
    {
        count++;
    }
    inline void decrement()
    {
        count--;
    }
    inline void setValue(uint64 value)
    {
        count.store(value);
    }
    inline uint64 getValue()
    {
        return count.load();
    }
    inline bool greaterEqual(uint64 target)
    {
        return count.load() >= target;
    }
    inline bool greaterThan(uint64 target)
    {
        return count.load() > target;
    }
    inline bool lessEqual(uint64 target)
    {
        return count.load() <= target;
    }
    inline bool lessThan(uint64 target)
    {
        return count.load() < target;
    }
    inline bool equals(uint64 target)
    {
        return count.load() == target;
    }
    friend std::strong_ordering operator<=>(const Counter& counter, uint64 other)
    {
        return counter.count.load() <=> other;
    }
    friend std::ostream& operator<<(std::ostream& stream, const Counter* counter)
    {
        stream << counter->count;
        return stream;
    }
    friend std::ostream& operator<<(std::ostream& stream, PCounter counter)
    {
        stream << counter->count;
        return stream;
    }
private:
    std::atomic_uint64_t count;
};
DEFINE_REF(Counter)
struct AwaitCounter
{
    PCounter counter;
    uint64 target;
    JobPriority priority = JobPriority::MEDIUM;
    bool await_ready() { return counter->greaterEqual(target); }
    void await_suspend(std::coroutine_handle<>) {
    }
    void await_resume() {}
};
struct AwaitCounter;
class FiberTask
{
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
private:
    promise_type* promise;
public:
    FiberTask() = default;
    explicit FiberTask(promise_type* promise) 
        : promise(promise) {}
    FiberTask(const FiberTask& other)
        : promise(other.promise) {}
    FiberTask(FiberTask&& other) 
        : promise(std::move(other.promise)) {}
    FiberTask& operator=(const FiberTask& other)
    {
        promise = other.promise;
        return *this;
    }
    FiberTask& operator=(FiberTask&& other) 
    {
        promise = std::move(other.promise);
        return *this;
    }
    inline void resume() { promise->resume(); }
    inline void destroy() { promise->destroy(); }
    inline bool done() { return promise->done(); }
    inline PCounter getCounter() const { return promise->counter; }
    inline uint64 getTarget() const { return promise->target; }
    inline JobPriority getPriority() const { return promise->priority; }
    struct promise_type
    {
        promise_type()
        {
            handle = handle_type::from_promise(*this);
        }
        ~promise_type() = default;
        promise_type(const promise_type&) = delete;
        promise_type(promise_type&&) = delete;
        promise_type &operator=(const promise_type&) = delete;
        promise_type &operator=(promise_type&&) = delete;
        FiberTask get_return_object()
        {
            return FiberTask(this);
        }
        auto initial_suspend() { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        auto return_void() {}
        void unhandled_exception() { std::exit(1); }
        AwaitCounter&& await_transform(AwaitCounter&& awaitCounter);
        inline void resume() { handle.resume(); }
        inline void destroy() { handle.destroy(); }
        inline bool done() { return handle.done(); }
    private:
        handle_type handle;
        PCounter counter;
        uint64 target;
        JobPriority priority;
        friend class FiberTask;
    };
};
class FiberJob
{
public:
    FiberJob();
    template<typename func, typename... args>
    FiberJob(PCounter counter, func function, args... arg)
        : counter(counter), function(std::bind(function, arg...))
    {
        jobID = jobIDGenerator.fetch_add(1);
    }
    template<typename func, typename...args>
    FiberJob(PCounter counter, JobPriority priority, func function, args... arg)
        : counter(counter), priority(priority), function(std::bind(function, arg...))
    {
    }
    FiberJob(const FiberJob& other) = delete;
    FiberJob(FiberJob&& other)
        : counter(std::move(other.counter))
        , jobID(std::move(other.jobID))
        , priority(std::move(other.priority))
        , function(std::move(other.function))
    {
    }
    ~FiberJob();
    FiberJob& operator=(const FiberJob& other) = delete;
    FiberJob& operator=(FiberJob&& other)
    {
        if(this != &other)
        {
            counter = std::move(other.counter);
            jobID = std::move(other.jobID);
            priority = std::move(other.priority);
            function = std::move(other.function);
        }
        return *this;
    }
    void operator()()
    {
        FiberTask task = function();
        if(!task.done())
        {
            JobQueue::suspendJob(std::move(task));
        }
        else
        {
            counter->increment();
            task.destroy();
        }
    }
    inline JobPriority getPriority()
    {
        return priority;
    }
private:
    PCounter counter;
    uint64 jobID;
    static std::atomic_uint64_t jobIDGenerator;
    JobPriority priority = JobPriority::MEDIUM;
    std::function<FiberTask()> function;
};
} // namespace Fibers
} // namespace Seele