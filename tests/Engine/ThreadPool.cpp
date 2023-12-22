#include "EngineTest.h"
#include "ThreadPool.h"
#include <numeric>

TEST(ThreadPool, RunBatch)
{
    ThreadPool t(10);
    uint32 test = 20;
    std::mutex m;
    List<std::function<void()>> work;
    for (uint32 i = 0; i < 40000; ++i)
    {
        work.add([&]() {
            std::unique_lock l(m);
            test++;
        });
    }
    t.runAndWait(std::move(work));
    ASSERT_EQ(test, 40020);
}