#include "ThreadPool.h"
#include <memory_resource>

using namespace Seele;

std::atomic_uint64_t Seele::globalCounter;

Event::Event()
    : flag(std::make_shared<std::atomic_bool>())
{
}

Event::Event(nullptr_t)
    : flag(nullptr)
{
}

Event::Event(const std::string &name)
    : name(name)
    , flag(std::make_shared<std::atomic_bool>())
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
    {
        std::scoped_lock lock(jobQueueLock);
        jobQueueCV.notify_all();
    }
    {
        std::scoped_lock lock(mainJobLock);
        mainJobCV.notify_all();
    }
    for (auto &thread : workers)
    {
        thread.join();
    }
    workers.clear();
    waitingJobs.clear();
    waitingMainJobs.clear();
}
void ThreadPool::enqueueWaiting(Event &event, Promise* job)
{
    assert(!job->done());
    std::scoped_lock lock(waitingLock);
    //std::cout << "Job " << job->finishedEvent.name << " waiting on event " << event.name << std::endl;
    waitingJobs[event].push_back(job);
}
void ThreadPool::enqueueWaiting(Event &event, MainPromise* job)
{
    assert(!job->done());
    std::scoped_lock lock(waitingMainLock);
    //std::cout << job->finishedEvent.name << " waiting on event " << event.name << std::endl;
    waitingMainJobs[event].push_back(job);
}
void ThreadPool::scheduleJob(Promise* job)
{
    assert(!job->done());
    std::scoped_lock lock(jobQueueLock);
    //std::cout << "Queueing job " << job->finishedEvent.name << std::endl;
    jobQueue.push_back(job);
    jobQueueCV.notify_one();
}
void ThreadPool::scheduleJob(MainPromise* job)
{
    assert(!job->done());
    std::scoped_lock lock(mainJobLock);
    //std::cout << "Queueing job " << job->finishedEvent.name << std::endl;
    mainJobs.push_back(job);
    mainJobCV.notify_one();
}
void ThreadPool::notify(Event &event)
{
    //std::cout << "Event " << event.name << " raised" << std::endl;
    {
        std::scoped_lock lock(jobQueueLock, waitingLock);
        std::list<Promise*> jobs = std::move(waitingJobs[event]);
        waitingJobs.erase(event);
        for (auto &job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up " << job->finishedEvent.name << std::endl;
            job->state = Promise::State::SCHEDULED;
            jobQueue.push_back(job);
            jobQueueCV.notify_one();
        }
    }
    {
        std::scoped_lock lock(mainJobLock, waitingMainLock);
        std::list<MainPromise*> jobs = std::move(waitingMainJobs[event]);
        waitingMainJobs.erase(event);
        for (auto &job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up main " << job->finishedEvent.name << std::endl;
            job->state = MainPromise::State::SCHEDULED;
            mainJobs.push_back(job);
            mainJobCV.notify_one();
        }
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
                    job = mainJobs.front();
                    mainJobs.pop_front();
                }
                else
                {
                    continue;
                }
            }
            job->resume();
            if (job->done())
            {
                job->finalize();
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
                    job = jobQueue.front();
                    jobQueue.pop_front();
                }
                else
                {
                    continue;
                }
            }
            //std::cout << "Starting job " << job.id << std::endl;
            job->resume();
            if(job->done())
            {
                job->finalize();
            }
        }
    }
}

ThreadPool &Seele::getGlobalThreadPool()
{
    static ThreadPool threadPool;
    return threadPool;
}