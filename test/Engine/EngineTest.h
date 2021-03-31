#pragma once
#include "Fibers/JobQueue.h"

namespace Seele
{
    struct GlobalFixture
    {
        GlobalFixture()
        {
        }
        ~GlobalFixture()
        {
        }
        void setup()
        {
            Fibers::JobQueue::initJobQueues();
        }
        void teardown()
        {
            Fibers::JobQueue::cleanupJobQueues();
        }
    };
};