#include "ThreadPool.h"
#include <memory_resource>

using namespace Seele;

Event::Event()
    : flag(new std::atomic_bool())
{
}

Event::Event(nullptr_t)
    : flag(nullptr)
{
}

Event::Event(const std::string &name)
    : name(name), flag(new std::atomic_bool())
{
}

void Event::raise()
{
    flag->store(1);
    getGlobalThreadPool().notify(*this);
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
    for (uint32 i = 0; i < threadCount; ++i)
    {
        workers[i] = std::thread(&ThreadPool::threadLoop, this, false);
    }
}

ThreadPool::~ThreadPool()
{
    running.store(false);
    for (auto &thread : workers)
    {
        thread.join();
    }
}
void ThreadPool::enqueueWaiting(Event &event, Promise* job)
{
    assert(!job->done());
    if(event == nullptr)
    {
        std::unique_lock lock(jobQueueLock);
        jobQueue.add(job);
        jobQueueCV.notify_one();
    }
    else
    {
        std::unique_lock lock(waitingLock);
        waitingJobs[event].add(job);
    }
}
void ThreadPool::enqueueWaiting(Event &event, MainPromise* job)
{
    assert(!job->done());
    if(event == nullptr)
    {
        std::unique_lock lock(mainJobLock);
        mainJobs.add(job);
        mainJobCV.notify_one();
    }
    else
    {
        std::unique_lock lock(waitingMainLock);
        waitingMainJobs[event].add(job);
    }
}
void ThreadPool::notify(Event &event)
{
    {
        std::unique_lock lock(jobQueueLock);
        std::unique_lock lock2(waitingLock);
        List<Promise*> &jobs = waitingJobs[event];
        for (auto &job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up job " << job.id << std::endl;
            jobQueue.add(job);
            jobQueueCV.notify_one();
        }
        jobs.clear();
    }
    {
        std::unique_lock lock(mainJobLock);
        std::unique_lock lock2(waitingMainLock);
        List<MainPromise*> &jobs = waitingMainJobs[event];
        for (auto &job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up main job " << job.id << std::endl;
            mainJobs.add(job);
            mainJobCV.notify_one();
        }
        jobs.clear();
    }
}
void ThreadPool::threadLoop(const bool mainThread)
{
    while (running.load())
    {
        if (mainThread)
        {
            MainPromise* job;
            {
                std::unique_lock lock(mainJobLock);
                if(mainJobs.empty())
                {
                    mainJobCV.wait(lock);
                }
                if (!mainJobs.empty())
                {
                    job = mainJobs.retrieve();
                }
                else
                {
                    continue;
                }
            }
            job->resume();
            if (job->done())
            {
                job->raise();
            }
        }
        else
        {
            Promise* job;
            {
                std::unique_lock lock(jobQueueLock);
                if (jobQueue.empty())
                {
                    jobQueueCV.wait(lock);
                }
                if (!jobQueue.empty())
                {
                    job = jobQueue.retrieve();
                }
                else
                {
                    continue;
                }
            }
            //std::cout << "Starting job " << job.id << std::endl;
            job->resume();
            if (job->done())
            {
                job->raise();
            }
        }
    }
}

ThreadPool &Seele::getGlobalThreadPool()
{
    static ThreadPool threadPool;
    return threadPool;
}