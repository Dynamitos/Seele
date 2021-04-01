#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
namespace Seele
{
namespace Fibers
{
class FiberJob;
DECLARE_REF(Counter)
enum class JobPriority : size_t
{
    HIGH = 0, 
    MEDIUM, 
    LOW,
    IO,
    NUM_PRIORITIES
};
struct AtomicJobQueue
{
};
class FiberTask;
struct JobPriorityInfo
{
    List<FiberJob> jobQueue;
    Map<PCounter, Array<FiberTask>> waiterQueue;
    List<FiberTask> resumableTasks;
    std::mutex jobQueueLock;
};
class JobQueue
{
public:
    JobQueue(JobPriority priority);
    JobQueue(const JobQueue& other) = delete;
    JobQueue(JobQueue&& other) = delete;
    JobQueue& operator=(const JobQueue& other) = delete;
    JobQueue& operator=(JobQueue&& other) = delete;
    ~JobQueue();
    static void initJobQueues();
    static void cleanupJobQueues();
    static void runJobs(FiberJob* jobs, uint32 numJobs);
    //This does in fact not do any coroutine stuff, but is a simple busy wait
    //For coroutine waits, you have to be in a coroutine, and 
    static void waitForCounter(PCounter counter, uint32 waitFor);
    static void suspendJob(FiberTask&& task);
    void refreshWaiterQueue();
private:
    void work();
    static Array<JobQueue*> workers;
    static StaticArray<JobPriorityInfo, (size_t)JobPriority::NUM_PRIORITIES> priorityQueues;
    std::thread threadHandle;
    JobPriority priority;
    bool running = true;
};
} // namespace Fibers
} // namespace Seele
