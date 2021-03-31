#include "EngineTest.h"
#include "MinimalEngine.h"
#include "Fibers/Fibers.h"
#include <coroutine>
#include <boost/test/unit_test.hpp>

using namespace Seele;
using namespace Seele::Fibers;

BOOST_AUTO_TEST_SUITE(Fibers_Suite)

FiberTask basicFiberSync1(PCounter syncCounter)
{
    syncCounter->increment();
    std::cout << "basicFiberSync1 start" << std::endl;
    co_await AwaitCounter{syncCounter, 2};
    std::cout << "basicFiberSync1 has resumed, counter is " << syncCounter << std::endl;
    co_return;
}

FiberTask basicFiberSync2(PCounter syncCounter)
{
    syncCounter->increment();
    std::cout << "basicFiberSync2 start" << std::endl;
    co_await AwaitCounter(syncCounter, 3);
    std::cout << "basicFiberSync2 has resumed, counter is " << syncCounter << std::endl;
    co_return;
}
BOOST_AUTO_TEST_CASE(basicFiberSync)
{
    PCounter syncCounter = new Counter(1);
    PCounter counter1 = new Counter();
    FiberJob job1 = FiberJob(counter1, &basicFiberSync1, syncCounter);
    JobQueue::runJobs(&job1, 1);
    FiberJob job2 = FiberJob(counter1, &basicFiberSync2, syncCounter);
    JobQueue::runJobs(&job2, 1);
    while(counter1->lessThan(2))
        std::this_thread::yield();
    BOOST_CHECK_EQUAL(syncCounter->getValue(), 3);
}
FiberTask incrementJob(PCounter localCounter)
{
    localCounter->increment();
    co_return;
}
BOOST_AUTO_TEST_CASE(jobBatch)
{
    PCounter localCounter = new Counter();
    PCounter jobCounter = new Counter();
    FiberJob jobs[256];
    for(int i = 0; i < 256; ++i)
    {
        jobs[i] = FiberJob(jobCounter, JobPriority::HIGH, &incrementJob, localCounter);
    }
    JobQueue::runJobs(jobs, 256);
    while(jobCounter->lessThan(256))
        std::this_thread::yield();
    BOOST_CHECK_EQUAL(localCounter->getValue(), 256);
}

BOOST_AUTO_TEST_SUITE_END()