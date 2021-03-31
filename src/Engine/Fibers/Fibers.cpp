#include "Fibers.h"

using namespace Seele;
using namespace Seele::Fibers;

std::atomic_uint64_t FiberJob::jobIDGenerator;

Counter::Counter() 
{
    count.store(0);
}

Counter::Counter(uint64 initialValue) 
{
    count.store(initialValue);
}

Counter::~Counter() 
{   
}

AwaitCounter&& FiberTask::promise_type::await_transform(AwaitCounter&& awaitCounter) 
{
    counter = awaitCounter.counter;
    target = awaitCounter.target;
    priority = awaitCounter.priority;
    return std::move(awaitCounter);
}

FiberJob::FiberJob() 
{
}

FiberJob::~FiberJob() 
{
}
