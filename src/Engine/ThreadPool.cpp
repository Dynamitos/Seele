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
    running.store(false);
    for(auto& thread : workers)
    {
        thread.join();
    }
}
void ThreadPool::addJob(Job&& job)
{
    std::unique_lock lock(jobQueueLock);
    //std::cout << "Adding job " << job << std::endl;
    jobQueue.add(std::move(job));
    jobQueueCV.notify_one();
}
void ThreadPool::addJob(MainJob&& job)
{
    std::unique_lock lock(mainJobLock);
    //std::cout << "Adding main job " << std::addressof(job.handle) << std::endl;
    mainJobs.add(std::move(job));
    mainJobCV.notify_one();
}
void ThreadPool::enqueueWaiting(Event* event, Job&& job)
{
    //std::cout << job << " waiting for event " << event->name << std::endl;
    std::unique_lock lock(waitingLock);
    waitingJobs[*event].add(std::move(job));
}
void ThreadPool::enqueueWaiting(Event* event, MainJob&& job)
{
    //std::cout << std::addressof(job.handle) << " main waiting for event " << event->name << std::endl;
    std::unique_lock lock(waitingMainLock);
    waitingMainJobs[*event].add(std::move(job));
}
void ThreadPool::notify(Event* event)
{
    {
        std::unique_lock lock(jobQueueLock);
        std::unique_lock lock2(waitingLock);
        while(!waitingJobs[*event].empty())
        {
            //std::cout << "Waking up job " << job << std::endl;
            jobQueue.add(std::move(waitingJobs[*event].retrieve()));
            jobQueueCV.notify_one();
        }
    }
    {
        std::unique_lock lock(mainJobLock);
        std::unique_lock lock2(waitingMainLock);
        while(!waitingMainJobs[*event].empty())
        {
            mainJobs.add(std::move(waitingMainJobs[*event].retrieve()));
            mainJobCV.notify_one();
        }
    }
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
                lock.unlock();
                job.resume();
            }
        }
        std::unique_lock lock(jobQueueLock);
        if(jobQueue.empty())
        {
            jobQueueCV.wait(lock);
        }
        if(!jobQueue.empty())
        {
            Job&& job = jobQueue.retrieve();
            lock.unlock();
            //std::cout << "Starting job " << job << std::endl;
            job.resume();
        }
    }
}

ThreadPool& Seele::getGlobalThreadPool()
{
    static ThreadPool threadPool;
    return threadPool;
}