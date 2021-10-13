#pragma once

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
            //Fibers::JobQueue::initJobQueues();
        }
        void teardown()
        {
            //Fibers::JobQueue::cleanupJobQueues();
        }
    };
};