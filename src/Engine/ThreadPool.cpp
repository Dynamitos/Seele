#include "ThreadPool.h"
#include <memory_resource>

using namespace Seele;


Event::Event()
    : flag(std::make_shared<StateStore>())
{
}

Event::Event(nullptr_t)
    : flag(nullptr)
{
}

Event::Event(const std::string &name)
    : flag(std::make_shared<StateStore>())
{
    flag->name = name;
}


Event::Event(const std::source_location &location)
    : flag(std::make_shared<StateStore>())
{
    flag->name = location.function_name();
    flag->location = location;
}


void Event::raise()
{
    std::scoped_lock lock(flag->lock);
    flag->data = 1;
    getGlobalThreadPool().notify(*this);
}
void Event::reset()
{
    std::scoped_lock lock(flag->lock);
    flag->data = 0;
}

bool Event::await_ready()
{
    flag->lock.lock();
    bool result = flag->data;
    if(result)
    {
        flag->lock.unlock();
    }
    return result;
}

ThreadPool::ThreadPool(uint32 threadCount)
    : workers(threadCount)
{
    running.store(true);
    for (uint32 i = 0; i < threadCount; ++i)
    {
        workers[i] = std::thread(&ThreadPool::threadLoop, this);
        workers[i].detach();
    }
}

ThreadPool::~ThreadPool()
{
    workers.clear();
}

void ThreadPool::waitIdle()
{
    while(true)
    {
        std::unique_lock lock(numIdlingLock);
        numIdlingIncr.wait(lock);
        if(numIdling == workers.size())
        {
            return;
        }
    }
}
void ThreadPool::enqueueWaiting(Event &event, Promise* job)
{
    assert(!job->done());
    std::scoped_lock lock(waitingLock);
    //std::cout << "Job " << job->finishedEvent.name << " waiting on event " << event.name << std::endl;
    waitingJobs[event].add(job);
    job->addRef();
}
void ThreadPool::enqueueWaiting(Event &event, MainPromise* job)
{
    assert(!job->done());
    std::scoped_lock lock(waitingMainLock);
    //std::cout << job->finishedEvent.name << " waiting on event " << event.name << std::endl;
    waitingMainJobs[event].add(job);
    job->addRef();
}
void ThreadPool::scheduleJob(Promise* job)
{
    assert(!job->done());
    std::scoped_lock lock(jobQueueLock);
    //std::cout << "Queueing job " << job->finishedEvent << std::endl;
    jobQueue.add(job);
    jobQueueCV.notify_one();
    job->addRef();
}
void ThreadPool::scheduleJob(MainPromise* job)
{
    assert(!job->done());
    std::scoped_lock lock(mainJobLock);
    //std::cout << "Queueing job " << job->finishedEvent << std::endl;
    mainJobs.add(job);
    mainJobCV.notify_one();
    job->addRef();
}
void ThreadPool::notify(Event &event)
{
    //std::cout << "Event " << event.name << " raised" << std::endl;
    {
        std::scoped_lock lock(jobQueueLock, waitingLock);
        List<Promise*> jobs = std::move(waitingJobs[event]);
        waitingJobs.erase(event);
        for (auto &job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up " << job->finishedEvent.name << std::endl;
            job->state = Promise::State::SCHEDULED;
            jobQueue.add(job);
            jobQueueCV.notify_one();
        }
    }
    {
        std::scoped_lock lock(mainJobLock, waitingMainLock);
        List<MainPromise*> jobs = std::move(waitingMainJobs[event]);
        waitingMainJobs.erase(event);
        for (auto &job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up main " << job->finishedEvent.name << std::endl;
            job->state = MainPromise::State::SCHEDULED;
            mainJobs.add(job);
            mainJobCV.notify_one();
        }
    }
}

void ThreadPool::mainLoop()
{
    while(true)
    {
        MainPromise* job;
        {
            std::unique_lock lock(mainJobLock);
            if(mainJobs.empty())
            {
                mainJobCV.wait(lock);
            }
            job = mainJobs.front();
            mainJobs.popFront();
        }
        job->resume();
        job->removeRef();
    }
}

void ThreadPool::threadLoop()
{
    List<Promise*> localQueue;
    while (true)
    {
        [[likely]]
        if(!localQueue.empty())
        {
            Promise* job = localQueue.retrieve();
            job->resume();
            job->removeRef();
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
            uint32 numTaken = std::max(jobQueue.size() / workers.size(), 1ull);
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