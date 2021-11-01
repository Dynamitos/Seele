#include "ThreadPool.h"

using namespace Seele;

Event::Event()
    : flag(new std::atomic_bool()) 
{}

Event::Event(const std::string& name)
    : name(name)
    , flag(new std::atomic_bool()) 
{}

void Event::raise()
{
    flag->store(1);
    getGlobalThreadPool().notify(this);
}
void Event::reset()
{ 
    flag->store(0);
}

bool Event::await_ready()
{
    return flag->load();
}

ThreadPool::ThreadPool(uint32 threadCount)
    : workers(threadCount)
{
    running.store(true);
    for(uint32 i = 0; i < threadCount; ++i)
    {
        workers[i] = std::thread(&ThreadPool::threadLoop, this, i == 0);
    }
}

ThreadPool::~ThreadPool()
{
    for(auto& thread : workers)
    {
        thread.join();
    }
}
void ThreadPool::addJob(Job&& job)
{
    std::unique_lock lock(jobQueueLock);
    jobQueue.add(job);
}
void ThreadPool::addJob(MainJob&& job)
{
    std::unique_lock lock(mainJobLock);
    mainJobs.add(job);
}
void ThreadPool::enqueueWaiting(Event* event, Job&& job)
{
    std::unique_lock lock(waitingLock);
    waitingJobs[event].add(std::move(job));
}
void ThreadPool::enqueueWaiting(Event* event, MainJob&& job)
{
    std::unique_lock lock(waitingMainLock);
    waitingMainJobs[event].add(std::move(job));
}
void ThreadPool::notify(Event* event)
{
    {
        std::unique_lock lock(jobQueueLock);
        std::unique_lock lock2(waitingLock);
        for(auto&& job : waitingJobs[event])
        {
            jobQueue.add(job);
        }
        waitingJobs[event].clear();
    }
    {
        std::unique_lock lock(mainJobLock);
        std::unique_lock lock2(waitingMainLock);
        for(auto&& job : waitingMainJobs[event])
        {
            mainJobs.add(job);
        }
        waitingMainJobs[event].clear();
    }
    event->reset();
}
void ThreadPool::threadLoop(const bool mainThread)
{
    while(running.load())
    {
        if(mainThread)
        {
            std::unique_lock lock(mainJobLock);
            if(!mainJobs.empty())
            {
                MainJob&& job = mainJobs.retrieve();
                job.resume();
            }
        }
    }
}

ThreadPool& Seele::getGlobalThreadPool()
{
    static ThreadPool threadPool;
    return threadPool;
}