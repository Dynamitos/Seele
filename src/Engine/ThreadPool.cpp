#include "ThreadPool.h"
#include <memory_resource>

using namespace Seele;

std::mutex Seele::promisesLock;
List<Promise*> Seele::promises;

//Event::Event(nullptr_t)
//{
//}

Event::Event(const std::string &name, const std::source_location &location)
    : state(std::make_shared<EventState>())
{
    state->name = name;
    state->location = location;
}

Event::Event(const std::source_location &location)
    : state(std::make_shared<EventState>())
{
    state->name = location.function_name();
    state->location = location;
}

Event::Event(const Event& other)
{
    std::scoped_lock lock(other.eventLock);
    state = other.state;
}

Event::Event(Event&& other)
{
    std::scoped_lock lock(other.eventLock);
    state = std::move(other.state);
}

Event& Event::operator=(const Event& other)
{
    if(this != &other)
    {
        std::scoped_lock lock(eventLock, other.eventLock);
        state = other.state;
    }
    return *this;
}
Event& Event::operator=(Event&& other)
{
    if(this != &other)
    {
        std::scoped_lock lock(eventLock, other.eventLock);
        state = std::move(other.state);
    }
    return *this;
}

void Event::raise()
{
    std::scoped_lock lock(eventLock);
    state->data = true;
    if(state->waitingJobs.size() > 0)
    {
        getGlobalThreadPool().scheduleBatch(state->waitingJobs);
    }
    if(state->waitingMainJobs.size() > 0)
    {
        getGlobalThreadPool().scheduleBatch(state->waitingMainJobs);
    }
}
void Event::reset()
{
    std::scoped_lock lock(eventLock);
    state->data = false;
}

bool Event::await_ready()
{
    eventLock.lock();
    bool result = state->data;
    if(result)
    {
        eventLock.unlock();
    }
    return result;
}

void Event::await_suspend(std::coroutine_handle<JobPromiseBase<false>> h)
{
    //h.promise().enqueue(this);
    state->waitingJobs.add(JobBase<false>(&h.promise()));
    eventLock.unlock();
}

void Event::await_suspend(std::coroutine_handle<JobPromiseBase<true>> h)
{
    //h.promise().enqueue(this);
    state->waitingMainJobs.add(JobBase<true>(&h.promise()));
    eventLock.unlock();
}

ThreadPool::ThreadPool(uint32 threadCount)
    : workers(threadCount)
{
    running.store(true);
    for (uint32 i = 0; i < threadCount; ++i)
    {
        workers[i] = std::thread(&ThreadPool::threadLoop, this);
    }
}

ThreadPool::~ThreadPool()
{
    running.store(false);
    {
        std::unique_lock lock(mainJobLock);
        mainJobCV.notify_all();
    }
    {
        std::unique_lock lock(jobQueueLock);
        jobQueueCV.notify_all();
    }
    for(auto& worker : workers)
    {
        worker.join();
    }
}

void ThreadPool::waitIdle()
{
    while(true)
    {
        std::unique_lock lock(numIdlingLock);
        if(numIdling == workers.size())
        {
            assert(promises.size() == 0);
            return;
        }
        numIdlingIncr.wait(lock);
    }
}
void ThreadPool::scheduleJob(Job job)
{
    assert(!job.done());
    std::scoped_lock lock(jobQueueLock);
    jobQueue.add(std::move(job));
    jobQueueCV.notify_one();
}
void ThreadPool::scheduleJob(MainJob job)
{
    assert(!job.done());
    std::scoped_lock lock(mainJobLock);
    mainJobs.add(std::move(job));
    mainJobCV.notify_one();
}
void ThreadPool::mainLoop()
{
    while(running.load())
    {
        MainJob job;
        {
            std::unique_lock lock(mainJobLock);
            if(mainJobs.empty())
            {
                mainJobCV.wait(lock);
            }
            [[likely]]
            if(!mainJobs.empty())
            {
                job = mainJobs.front();
                mainJobs.popFront();
            }
            else
            {
                continue;
            }
        }
        job.resume();
    }
}

void ThreadPool::threadLoop()
{
    List<Job> localQueue;
    while (running.load())
    {
        [[likely]]
        if(!localQueue.empty())
        {
            Job job = localQueue.retrieve();
            job.resume();
        }
        else
        {
            std::unique_lock lock(jobQueueLock);
            if (jobQueue.empty())
            {
                {
                    std::unique_lock lock2(numIdlingLock);
                    numIdling++;
                    numIdlingIncr.notify_one();
                }
                jobQueueCV.wait(lock);
                {
                    std::unique_lock lock2(numIdlingLock);
                    numIdling--;
                }
            }
            // take 1/numThreads jobs, maybe make this a parameter that
            // adjusts based on past workload
            uint32 partitionedWorkload = (uint32)(jobQueue.size() / workers.size());
            uint32 numTaken = std::clamp(partitionedWorkload, 1u, localQueueSize);
            while (!jobQueue.empty() && localQueue.size() < numTaken)
            {
                localQueue.add(jobQueue.retrieve());
            }
        }
    }
}
ThreadPool &Seele::getGlobalThreadPool()
{
    static ThreadPool threadPool;
    return threadPool;
}