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
    //std::cout << "Adding job " << job.id << std::endl;
    jobQueue.add(std::move(job));
    jobQueueCV.notify_one();
}
void ThreadPool::addJob(MainJob&& job)
{
    std::unique_lock lock(mainJobLock);
    //std::cout << "Adding main job " << job.id << std::endl;
    mainJobs.add(std::move(job));
    mainJobCV.notify_one();
}
void ThreadPool::enqueueWaiting(Event& event, Job job)
{
    //std::cout << job.id << " waiting for event " << event.name << std::endl;
    std::unique_lock lock(waitingLock);
    waitingJobs[event].add(std::move(job));
}
void ThreadPool::enqueueWaiting(Event& event, MainJob job)
{
    //std::cout << job.id << " main waiting for event " << event.name << std::endl;
    std::unique_lock lock(waitingMainLock);
    waitingMainJobs[event].add(std::move(job));
}
void ThreadPool::notify(Event& event)
{
    {
        std::unique_lock lock(jobQueueLock);
        std::unique_lock lock2(waitingLock);
        List<Job>& jobs = waitingJobs[event];
        for(auto& job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up job " << job.id << std::endl;
            jobQueue.add(std::move(job));
            jobQueueCV.notify_one();
        }
        jobs.clear();
    }
    {
        std::unique_lock lock(mainJobLock);
        std::unique_lock lock2(waitingMainLock);
        List<MainJob>& jobs = waitingMainJobs[event];
        for(auto& job : jobs)
        {
            //assert(job.id != -1ull);
            //std::cout << "Waking up main job " << job.id << std::endl;
            mainJobs.add(std::move(job));
            mainJobCV.notify_one();
        }
        jobs.clear();
    }
}
void ThreadPool::tryMainJob()
{
    MainJob job;
    {
        std::unique_lock lock(mainJobLock);
        if(!mainJobs.empty())
        {
            job = std::move(mainJobs.retrieve());
        }
        else
        {
            return;
        }
    }
    job.resume();
}
void ThreadPool::threadLoop(const bool mainThread)
{
    while(running.load())
    {
        if(mainThread)
        {
            tryMainJob();
        }
        Job job;
        {
            std::unique_lock lock(jobQueueLock);
            if(jobQueue.empty())
            {
                jobQueueCV.wait(lock);
            }
            if(!jobQueue.empty())
            {
                job = std::move(jobQueue.retrieve());
            }
            else
            {
                continue;
            }
        }
        //std::cout << "Starting job " << job.id << std::endl;
        job.resume();
        if(job.done())
        {
            job.signal();
        }
    }
}

ThreadPool& Seele::getGlobalThreadPool()
{
    static ThreadPool threadPool;
    return threadPool;
}