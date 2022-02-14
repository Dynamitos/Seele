#pragma once
//#include <vld.h>
#include "ThreadPool.h"

namespace Seele
{
    struct GlobalFixture
    {
        GlobalFixture()
        {
            //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        }
        ~GlobalFixture()
        {
            getGlobalThreadPool().cleanup();
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