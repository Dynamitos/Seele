#include "JobQueue.h"
#include "Fibers.h"
#include <Windows.h>
#include <processthreadsapi.h>

using namespace Seele;
using namespace Seele::Fibers;

std::wstring toString(JobPriority priority)
{
    switch (priority)
    {
    case JobPriority::HIGH:
        return L"HIGH";
    case JobPriority::MEDIUM:
        return L"MEDIUM";
    case JobPriority::LOW:
        return L"LOW";
    case JobPriority::IO:
        return L"IO";
    default:
        return L"Illegal Priority";
    }
}

Array<JobQueue*> JobQueue::workers;
StaticArray<JobPriorityInfo, (size_t)JobPriority::NUM_PRIORITIES> JobQueue::priorityQueues;

JobQueue::JobQueue(JobPriority priority)
    : priority(priority)
{
    threadHandle = std::thread(&JobQueue::work, this);
    SetThreadDescription(threadHandle.native_handle(), toString(priority).c_str());
}

JobQueue::~JobQueue() 
{
    running = false;
    threadHandle.join();
}

void JobQueue::initJobQueues() 
{
    // TODO: make this dependent on actual cores
    JobPriority priorities[16] = {
        JobPriority::IO, JobPriority::IO,
        JobPriority::LOW, JobPriority::LOW, JobPriority::LOW, JobPriority::LOW, 
        JobPriority::MEDIUM, JobPriority::MEDIUM, JobPriority::MEDIUM, JobPriority::MEDIUM, JobPriority::MEDIUM,
        JobPriority::HIGH, JobPriority::HIGH, JobPriority::HIGH, JobPriority::HIGH, JobPriority::HIGH};

    for(uint32 i = 0; i < 16; ++i)
    {
        workers.add(new JobQueue(priorities[i]));
    }
}
void JobQueue::cleanupJobQueues()
{
    for(auto worker : workers)
    {
        delete worker;
    }
}

void JobQueue::runJobs(FiberJob* jobs, uint32 numJobs) 
{
    for(uint32 i = 0; i < numJobs; ++i)
    {
        JobPriorityInfo& info = priorityQueues[(size_t)jobs[i].getPriority()];
        std::unique_lock lock(info.jobQueueLock);
        info.jobQueue.add(std::move(jobs[i]));
        //info.jobQueueCV.notify_one();
    }
}

void JobQueue::waitForCounter(PCounter counter, uint32 waitFor) 
{
    while(counter->lessThan(waitFor))
        std::this_thread::yield();
}

void JobQueue::suspendJob(FiberTask&& task) 
{
    JobPriorityInfo& info = priorityQueues[(size_t)task.getPriority()];
    std::unique_lock lock(info.jobQueueLock);
    info.waiterQueue[task.getCounter()].add(task);
}
void JobQueue::refreshWaiterQueue()
{
    JobPriorityInfo& info = priorityQueues[(size_t)priority];
    //shortcut to avoid iterator creation
    if(info.waiterQueue.empty())
        return;
    for(auto it : info.waiterQueue)
    {
        auto& entries = info.waiterQueue[it.key];
        for(uint32 i = 0; i < entries.size(); ++i)
        {
            if(it.key->greaterEqual(entries[i].getTarget()))
            {
                info.resumableTasks.add(std::move(entries[i]));
                entries.remove(i);
                i--;
            }
        }
    }
}

void JobQueue::work()
{
    JobPriorityInfo& targetInfo = priorityQueues[(size_t)priority];
    auto& jobQueue = targetInfo.jobQueue;
    auto& resumableTasks = targetInfo.resumableTasks;
    while(running)
    {
        std::unique_lock lock(targetInfo.jobQueueLock);
        refreshWaiterQueue();
        if(resumableTasks.size() > 0)
        {
            FiberTask task = std::move(resumableTasks.front());
            resumableTasks.popFront();
            lock.unlock();
            task.resume();
            lock.lock();
        }
        if(jobQueue.size() > 0)
        {
            FiberJob job = std::move(jobQueue.front());
            jobQueue.popFront();
            lock.unlock();
            job();
            lock.lock();
        }
    }
}